#include "screen.h"

#include "app/city/view.h"
#include "app/city/warning.h"
#include "app/graphics/graphics.h"
#include "app/graphics/window.h"

static struct {
    int width;
    int height;
    struct {
        int x;
        int y;
    } dialog_offset;
} data;

void screen_set_resolution(int width, int height)
{
    data.width = width;
    data.height = height;
    data.dialog_offset.x = (width - 640) / 2;
    data.dialog_offset.y = (height - 480) / 2;

    graphics_init_canvas(width, height);

    city_view_set_viewport(width, height);
    city_warning_clear_all();
    window_invalidate();
}

int screen_width(void)
{
    return data.width;
}

int screen_height(void)
{
    return data.height;
}

int screen_dialog_offset_x(void)
{
    return data.dialog_offset.x;
}

int screen_dialog_offset_y(void)
{
    return data.dialog_offset.y;
}
