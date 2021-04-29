#pragma once
#include <stdint.h>

//bool show_demo_window;
//bool show_another_window;

void GUI_DrawAllWindows();
void GUI_DrawMainWindow();
void GUI_DrawMainWindowMenu();
void GUI_DrawMemoryEditorWindow();
void GUI_DrawRenderWindow();
void GUI_DrawRegisterWindow();

void GUI_Closing();
int GUI_Init();

bool GUI_Render();