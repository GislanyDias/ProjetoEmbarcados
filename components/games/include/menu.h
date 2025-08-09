#ifndef MENU_H
#define MENU_H

#include <stdbool.h>

// Enumeração das opções do menu
typedef enum {
    MENU_OPTION_DODGE,
    MENU_OPTION_TILT_MAZE,
    MENU_OPTION_SNAKE_TILT,
    MENU_OPTION_PADDLE_PONG,
    MENU_OPTION_COUNT
} MenuOption;

/**
 * @brief Inicializa o sistema de menu
 */
void menu_init(void);

/**
 * @brief Atualiza e renderiza o menu
 * @param current_option A opção atualmente selecionada
 */
void menu_update(MenuOption current_option);

/**
 * @brief Obtém a opção do menu selecionada pelo usuário
 * @return MenuOption selecionada
 */
MenuOption menu_get_selected_option(void);

/**
 * @brief Verifica se uma nova opção foi selecionada
 * @return true se uma nova opção foi selecionada, false caso contrário
 */
bool menu_option_selected(void);

/**
 * @brief Toca o som de navegação no menu
 */
void menu_play_nav_sound(void);

/**
 * @brief Toca o som de seleção no menu
 */
void menu_play_select_sound(void);

#endif // MENU_H