#ifndef MENU_H
#define MENU_H

#include <stdbool.h>

typedef enum {
    MENU_NONE,
    MENU_START,
    MENU_OPTIONS
} MenuAction;

bool Menu_Init(void);
MenuAction Menu_Update(void);
void Menu_Draw(void);
void Menu_Unload(void);

#endif
