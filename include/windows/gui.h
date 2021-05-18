#pragma once
#include <stdint.h>
#include "imgui.h"

//bool show_demo_window;
//bool show_another_window;

bool GUI_DrawAllWindows();
void GUI_DrawFileDialog();
bool GUI_DrawMainWindowMenu();
void GUI_DrawMemoryEditorWindow();
void GUI_DrawRenderWindow();
void GUI_DrawRegisterWindow();

void GUI_Closing();
int GUI_Init();

bool GUI_Render();

int8_t OpenROM(const char* filename);

struct CustomConstraints
{
    // Helper functions to demonstrate programmatic constraints
    static void Square(ImGuiSizeCallbackData* data)
    {
        data->DesiredSize.x = data->DesiredSize.y =
            (
                (data->DesiredSize.x >= data->DesiredSize.y) ? data->DesiredSize.x : data->DesiredSize.y
            );
    }

    static void TwoToOneRatio(ImGuiSizeCallbackData* data)
    {
        if (data->DesiredSize.x < data->DesiredSize.y * 2)
            data->DesiredSize.x = data->DesiredSize.y * 2;
        else if (data->DesiredSize.y < data->DesiredSize.x / 2)
            data->DesiredSize.y = data->DesiredSize.x / 2;

    }
};