#ifndef WIDGET_TOP_MENU_EDITOR_H
#define WIDGET_TOP_MENU_EDITOR_H

#include "app/input/hotkey.h"
#include "app/input/mouse.h"

void widget_top_menu_editor_draw(void);
int widget_top_menu_editor_handle_input(const mouse *m, const hotkeys *h);

#endif // WIDGET_TOP_MENU_EDITOR_H
