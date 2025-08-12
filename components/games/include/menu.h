#ifndef MENU_H
#define MENU_H

#include <stdbool.h>

typedef enum {
    MENU_OPTION_DODGE,
    MENU_OPTION_TILT_MAZE,
    MENU_OPTION_SNAKE_TILT,
    MENU_OPTION_PADDLE_PONG,
    MENU_OPTION_COUNT
} MenuOption;

void menu_init(void);
void menu_update(MenuOption current_option);
MenuOption menu_get_selected_option(void);
bool menu_option_selected(void);
void menu_play_nav_sound(void);
void menu_play_select_sound(void);

#endif