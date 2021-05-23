#include "map.h"

#include "app/city/view.h"
#include "app/editor/editor.h"
#include "app/editor/tool.h"
#include "app/game/game.h"
#include "app/graphics/generic_button.h"
#include "app/graphics/graphics.h"
#include "app/graphics/image.h"
#include "app/graphics/lang_text.h"
#include "app/graphics/panel.h"
#include "app/graphics/text.h"
#include "app/graphics/window.h"
#include "app/widget/map_editor.h"
#include "app/widget/top_menu_editor.h"
#include "app/widget/sidebar/editor.h"
#include "app/window/file_dialog.h"
#include "app/window/popup_dialog.h"
#include "app/window/editor/attributes.h"

static void draw_background(void)
{
    graphics_clear_screen();
    widget_sidebar_editor_draw_background();
    widget_top_menu_editor_draw();
}

static void draw_cancel_construction(void)
{
    if (!mouse_get()->is_touch || !editor_tool_is_active()) {
        return;
    }
    int x, y, width, height;
    city_view_get_viewport(&x, &y, &width, &height);
    width -= 4 * 16;
    inner_panel_draw(width - 4, 40, 3, 2);
    image_draw(image_group(GROUP_OK_CANCEL_SCROLL_BUTTONS) + 4, width, 44);
}

static void draw_foreground(void)
{
    widget_sidebar_editor_draw_foreground();
    widget_map_editor_draw();
    if (window_is(WINDOW_EDITOR_MAP)) {
        draw_cancel_construction();
    }
}

static void handle_hotkeys(const hotkeys *h)
{
    if (h->load_file) {
        window_file_dialog_show(FILE_TYPE_SCENARIO, FILE_DIALOG_LOAD);
    }
    if (h->save_file) {
        window_file_dialog_show(FILE_TYPE_SCENARIO, FILE_DIALOG_SAVE);
    }
}

static void handle_input(const mouse *m, const hotkeys *h)
{
    handle_hotkeys(h);
    if (widget_top_menu_editor_handle_input(m, h)) {
        return;
    }
    if (widget_sidebar_editor_handle_mouse(m)) {
        return;
    }
    widget_map_editor_handle_input(m, h);
}

void window_editor_map_draw_all(void)
{
    draw_background();
    draw_foreground();
}

void window_editor_map_draw_panels(void)
{
    draw_background();
}

void window_editor_map_draw(void)
{
    widget_map_editor_draw();
}

void window_editor_map_show(void)
{
    window_type window = {
        WINDOW_EDITOR_MAP,
        draw_background,
        draw_foreground,
        handle_input
    };
    window_show(&window);
}