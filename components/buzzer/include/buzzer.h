#ifndef BUZZER_H
#define BUZZER_H

#include "driver/ledc.h"

#define BUZZER_GPIO 18

void buzzer_init(void);
void play_tone(int frequency, int duration_ms);
void play_game_over(void);
void play_menu_select(void);
void play_menu_navigate(void);
void play_point_scored(void);
void play_level_up(void);

#endif