#include "attributes.h"

#include "app/core/image.h"
#include "app/core/image_group_editor.h"
#include "app/core/string.h"
#include "app/game/resource.h"
#include "app/graphics/arrow_button.h"
#include "app/graphics/button.h"
#include "app/graphics/generic_button.h"
#include "app/graphics/graphics.h"
#include "app/graphics/image.h"
#include "app/graphics/lang_text.h"
#include "app/graphics/panel.h"
#include "app/graphics/screen.h"
#include "app/graphics/text.h"
#include "app/graphics/window.h"
#include "app/input/input.h"
#include "app/scenario/editor.h"
#include "app/scenario/property.h"
#include "app/widget/input_box.h"
#include "app/widget/minimap.h"
#include "app/widget/sidebar/editor.h"
#include "app/window/editor/allowed_buildings.h"
#include "app/window/editor/demand_changes.h"
#include "app/window/editor/invasions.h"
#include "app/window/editor/map.h"
#include "app/window/editor/price_changes.h"
#include "app/window/editor/requests.h"
#include "app/window/editor/special_events.h"
#include "app/window/editor/starting_conditions.h"
#include "app/window/editor/win_criteria.h"
#include "app/window/select_list.h"

#define BRIEF_DESC_LENGTH 64

static void button_starting_conditions(int param1, int param2);
static void button_requests(int param1, int param2);
static void button_enemy(int param1, int param2);
static void button_invasions(int param1, int param2);
static void button_allowed_buildings(int param1, int param2);
static void button_win_criteria(int param1, int param2);
static void button_special_events(int param1, int param2);
static void button_price_changes(int param1, int param2);
static void button_demand_changes(int param1, int param2);
static void change_climate(int param1, int param2);
static void change_image(int forward, int param2);

static generic_button buttons[] = {
    {212, 76, 250, 30, button_starting_conditions, button_none, 1, 0},
    {212, 116, 250, 30, change_climate, button_none, 2, 0},
    {212, 156, 250, 30, button_requests, button_none, 3, 0},
    {212, 196, 250, 30, button_enemy, button_none, 4, 0},
    {212, 236, 250, 30, button_invasions, button_none, 5, 0},
    {212, 276, 250, 30, button_allowed_buildings, button_none, 6, 0},
    {212, 316, 250, 30, button_win_criteria, button_none, 7, 0},
    {212, 356, 250, 30, button_special_events, button_none, 8, 0},
    {212, 396, 250, 30, button_price_changes, button_none, 9, 0},
    {212, 436, 250, 30, button_demand_changes, button_none, 10, 0},
};

static arrow_button image_arrows[] = {
    {20, 424, 19, 24, change_image, 0, 0},
    {44, 424, 21, 24, change_image, 1, 0},
};

static struct {
    int is_paused;
    uint8_t brief_description[BRIEF_DESC_LENGTH];
    int focus_button_id;
} data;

static input_box scenario_description_input = {
    92, 40, 19, 2, FONT_NORMAL_WHITE, 1,
    data.brief_description, BRIEF_DESC_LENGTH
};

static void start(void)
{
    if (data.is_paused) {
        input_box_resume(&scenario_description_input);
    } else {
        string_copy(scenario_brief_description(), data.brief_description, BRIEF_DESC_LENGTH);
        input_box_start(&scenario_description_input);
    }
}

static void stop(int paused)
{
    if (paused) {
        input_box_pause(&scenario_description_input);
    } else {
        input_box_stop(&scenario_description_input);
    }
    data.is_paused = paused;
    scenario_editor_update_brief_description(data.brief_description);
}

static void draw_background(void)
{
    window_editor_map_draw_all();

    graphics_in_dialog();

    outer_panel_draw(0, 28, 30, 28);

    button_border_draw(18, 278, 184, 144, 0);
    image_draw(image_group(GROUP_EDITOR_SCENARIO_IMAGE) + scenario_image_id(), 20, 280);

    graphics_reset_dialog();
}

static void draw_foreground(void)
{
    graphics_in_dialog();

    input_box_draw(&scenario_description_input);

    button_border_draw(212, 76, 250, 30, data.focus_button_id == 1);
    lang_text_draw_centered(44, 88, 212, 85, 250, FONT_NORMAL_BLACK);

    lang_text_draw(44, 76, 32, 125, FONT_NORMAL_BLACK);
    button_border_draw(212, 116, 250, 30, data.focus_button_id == 2);
    lang_text_draw_centered(44, 77 + scenario_property_climate(), 212, 125, 250, FONT_NORMAL_BLACK);

    lang_text_draw(44, 40, 32, 165, FONT_NORMAL_BLACK);
    button_border_draw(212, 156, 250, 30, data.focus_button_id == 3);

    editor_request request;
    scenario_editor_request_get(0, &request);
    if (request.resource) {
        lang_text_draw_year(scenario_property_start_year() + request.year, 222, 165, FONT_NORMAL_BLACK);
        int width = text_draw_number(request.amount, '@', " ", 312, 165, FONT_NORMAL_BLACK);
        int offset = request.resource + resource_image_offset(request.resource, RESOURCE_IMAGE_ICON);
        image_draw(image_group(GROUP_EDITOR_RESOURCE_ICONS) + offset, 322 + width, 160);
    } else {
        lang_text_draw_centered(44, 19, 212, 165, 250, FONT_NORMAL_BLACK);
    }

    lang_text_draw(44, 41, 32, 205, FONT_NORMAL_BLACK);
    button_border_draw(212, 196, 250, 30, data.focus_button_id == 4);
    lang_text_draw_centered(37, scenario_property_enemy(), 212, 205, 250, FONT_NORMAL_BLACK);

    lang_text_draw(44, 42, 32, 245, FONT_NORMAL_BLACK);
    button_border_draw(212, 236, 250, 30, data.focus_button_id == 5);

    editor_invasion invasion;
    scenario_editor_invasion_get(0, &invasion);
    if (invasion.type) {
        lang_text_draw_year(scenario_property_start_year() + invasion.year, 222, 245, FONT_NORMAL_BLACK);
        int width = text_draw_number(invasion.amount, '@', " ", 302, 245, FONT_NORMAL_BLACK);
        lang_text_draw(34, invasion.type, 302 + width, 245, FONT_NORMAL_BLACK);
    } else {
        lang_text_draw_centered(44, 20, 212, 245, 250, FONT_NORMAL_BLACK);
    }

    button_border_draw(212, 276, 250, 30, data.focus_button_id == 6);
    lang_text_draw_centered(44, 44, 212, 285, 250, FONT_NORMAL_BLACK);

    button_border_draw(212, 316, 250, 30, data.focus_button_id == 7);
    lang_text_draw_centered(44, 45, 212, 325, 250, FONT_NORMAL_BLACK);

    button_border_draw(212, 356, 250, 30, data.focus_button_id == 8);
    lang_text_draw_centered(44, 49, 212, 365, 250, FONT_NORMAL_BLACK);

    button_border_draw(212, 396, 250, 30, data.focus_button_id == 9);
    lang_text_draw_centered(44, 95, 212, 405, 250, FONT_NORMAL_BLACK);

    button_border_draw(212, 436, 250, 30, data.focus_button_id == 10);
    lang_text_draw_centered(44, 94, 212, 445, 250, FONT_NORMAL_BLACK);

    arrow_buttons_draw(0, 0, image_arrows, 2);

    graphics_reset_dialog();
}

static void handle_input(const mouse *m, const hotkeys *h)
{
    const mouse *m_dialog = mouse_in_dialog(m);
    if (input_box_handle_mouse(m_dialog, &scenario_description_input) ||
        generic_buttons_handle_mouse(m_dialog, 0, 0, buttons, 10, &data.focus_button_id) ||
        arrow_buttons_handle_mouse(m_dialog, 0, 0, image_arrows, 2, 0) ||
        widget_sidebar_editor_handle_mouse_attributes(m)) {
        return;
    }
    if (input_go_back_requested(m, h)) {
        stop(0);
        window_editor_map_show();
    }
}

static void button_starting_conditions(int param1, int param2)
{
    stop(1);
    window_editor_starting_conditions_show();
}

static void button_requests(int param1, int param2)
{
    stop(1);
    window_editor_requests_show();
}

static void set_enemy(int enemy)
{
    scenario_editor_set_enemy(enemy);
    start();
}

static void button_enemy(int param1, int param2)
{
    stop(1);
    window_select_list_show(screen_dialog_offset_x() + 12, screen_dialog_offset_y() + 40, 37, 20, set_enemy);
}

static void button_invasions(int param1, int param2)
{
    stop(1);
    window_editor_invasions_show();
}

static void button_allowed_buildings(int param1, int param2)
{
    stop(1);
    window_editor_allowed_buildings_show();
}

static void button_win_criteria(int param1, int param2)
{
    stop(1);
    window_editor_win_criteria_show();
}

static void button_special_events(int param1, int param2)
{
    stop(1);
    window_editor_special_events_show();
}

static void button_price_changes(int param1, int param2)
{
    stop(1);
    window_editor_price_changes_show();
}

static void button_demand_changes(int param1, int param2)
{
    stop(1);
    window_editor_demand_changes_show();
}

static void change_climate(int param1, int param2)
{
    scenario_editor_cycle_climate();
    image_load_climate(scenario_property_climate(), 1, 0);
    widget_minimap_invalidate();
    window_request_refresh();
}

static void change_image(int forward, int param2)
{
    scenario_editor_cycle_image(forward);
    window_request_refresh();
}

void window_editor_attributes_show(void)
{
    window_type window = {
        WINDOW_EDITOR_ATTRIBUTES,
        draw_background,
        draw_foreground,
        handle_input
    };
    start();
    window_show(&window);
}