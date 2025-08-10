#ifndef GAMES_H
#define GAMES_H

typedef enum {
    MENU_OPTION_DODGE,
    MENU_OPTION_TILT_MAZE,
    MENU_OPTION_SNAKE_TILT,
    MENU_OPTION_PADDLE_PONG,
    MENU_OPTION_COUNT
} MenuOption;

void games_init(void);
void games_run(void);

#endif