#include "warning.h"

#include "app/city/warning.h"
#include "app/game/state.h"
#include "app/graphics/image.h"
#include "app/graphics/panel.h"
#include "app/graphics/screen.h"
#include "app/graphics/text.h"
#include "app/graphics/window.h"

static const int TOP_OFFSETS[] = {30, 55, 80, 105, 130};

static int determine_width(const uint8_t *text)
{
    int width = text_get_width(text, FONT_NORMAL_BLACK);
    if (width <= 100) {
        return 200;
    } else if (width <= 200) {
        return 300;
    } else if (width <= 300) {
        return 400;
    } else {
        return 460;
    }
}

void warning_draw(void)
{
    if (!window_is(WINDOW_CITY) && !window_is(WINDOW_EDITOR_MAP)) {
        city_warning_clear_all();
        return;
    }

    int center = (screen_width() - 180) / 2;
    for (int i = 0; i < 5; i++) {
        const uint8_t *text = city_warning_get(i);
        if (!text) {
            continue;
        }
        int top_offset = TOP_OFFSETS[i];
        if (game_state_is_paused()) {
            top_offset += 70;
        }
        int box_width = determine_width(text);
        label_draw(center - box_width / 2 + 1, top_offset, box_width / 16 + 1, 1);
        if (box_width < 460) {
            // ornaments at the side
            image_draw(image_group(GROUP_CONTEXT_ICONS) + 15, center - box_width / 2 + 2, top_offset + 2);
            image_draw(image_group(GROUP_CONTEXT_ICONS) + 15, center + box_width / 2 - 30, top_offset + 2);
        }
        text_draw_centered(text, center - box_width / 2 + 1, top_offset + 4, box_width, FONT_NORMAL_WHITE, 0);
    }
    city_warning_clear_outdated();
}