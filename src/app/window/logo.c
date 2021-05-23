#include "logo.h"

#include "app/core/config.h"
#include "app/graphics/graphics.h"
#include "app/graphics/image.h"
#include "app/graphics/lang_text.h"
#include "app/graphics/window.h"
#include "app/sound/music.h"
#include "app/window/intro_video.h"
#include "app/window/main_menu.h"
#include "app/window/plain_message_dialog.h"

static void init(void)
{
    sound_music_play_intro();
}

static void draw_background(void)
{
    graphics_clear_screen();

    graphics_in_dialog();
    image_draw(image_group(GROUP_LOGO), 0, 0);
    lang_text_draw_centered_colored(13, 7, 160, 462, 320, FONT_NORMAL_PLAIN, COLOR_WHITE);
    graphics_reset_dialog();
}

static void handle_input(const mouse *m, const hotkeys *h)
{
    if (m->left.went_up || m->right.went_up) {
        window_main_menu_show(0);
        return;
    }
    if (h->escape_pressed) {
        hotkey_handle_escape();
    }
}

void window_logo_show(int show_patch_message)
{
    window_type window = {
        WINDOW_LOGO,
        draw_background,
        0,
        handle_input
    };
    init();
    window_show(&window);
    if (show_patch_message == MESSAGE_MISSING_PATCH) {
        window_plain_message_dialog_show(TR_NO_PATCH_TITLE, TR_NO_PATCH_MESSAGE);
    } else if (show_patch_message == MESSAGE_MISSING_FONTS) {
        window_plain_message_dialog_show(TR_MISSING_FONTS_TITLE, TR_MISSING_FONTS_MESSAGE);
    }
    if (config_get(CONFIG_UI_SHOW_INTRO_VIDEO)) {
        window_intro_video_show();
    }
}
