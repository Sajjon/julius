#include "input.h"

#include "app/input/hotkey.h"
#include "app/input/mouse.h"

int input_go_back_requested(const mouse *m, const hotkeys *h)
{
    return m->right.went_up || (m->is_touch && m->left.double_click) || h->escape_pressed;
}
