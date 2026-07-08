#include "OLED_Display.h"

#include "global.h"
#include "oled.h"
#include <stdio.h>

#pragma SET_DATA_SECTION("logVariables")
static volatile uint8_t oledRefreshFlag = 0U;
static char oledLine1[20];
static char oledLine2[20];
static char oledLine3[20];
static char oledLine4[20];
static char oledLine5[20];
static char oledLine6[20];
#pragma SET_DATA_SECTION()

static void OLED_Display_FormatFixed2(char *buffer, const char *label, float32_t value)
{
    int32_t integral;
    int32_t fractional;
    float32_t absValue = value;

    if (absValue < 0.0f)
    {
        absValue = -absValue;
    }

    integral = (int32_t)absValue;
    fractional = (int32_t)((absValue - (float32_t)integral) * 100.0f + 0.5f);

    if (fractional >= 100)
    {
        integral++;
        fractional -= 100;
    }

    if (value < 0.0f)
    {
        sprintf(buffer, "%s:-%ld.%02ld    ", label, integral, fractional);
    }
    else
    {
        sprintf(buffer, "%s:%ld.%02ld     ", label, integral, fractional);
    }
}

static void OLED_Display_ShowLine(uint8_t page, char *buffer)
{
    OLED_ShowString(0U, page, (uint8_t *)buffer, 8U);
}

static void OLED_Display_UpdateLine(void)
{
    static uint8_t line = 0U;

    switch (line)
    {
        case 0U:
            sprintf(oledLine1, "MODE:%-10u", MODE);
            OLED_Display_ShowLine(0U, oledLine1);
            break;

        case 1U:
            OLED_Display_FormatFixed2(oledLine2, "Uout", Uout);
            OLED_Display_ShowLine(1U, oledLine2);
            break;

        case 2U:
            OLED_Display_FormatFixed2(oledLine3, "Duty", Duty);
            OLED_Display_ShowLine(2U, oledLine3);
            break;

        case 3U:
            OLED_Display_FormatFixed2(oledLine4, "Ua", Ua_pu);
            OLED_Display_ShowLine(3U, oledLine4);
            break;

        case 4U:
            OLED_Display_FormatFixed2(oledLine5, "Ub", Ub_pu);
            OLED_Display_ShowLine(4U, oledLine5);
            break;

        default:
            OLED_Display_FormatFixed2(oledLine6, "Uc", Uc_pu);
            OLED_Display_ShowLine(5U, oledLine6);
            line = 0U;
            return;
    }

    line++;
}

void OLED_Display_Init(void)
{
    OLED_Init();
}

void OLED_Display_RequestRefresh(void)
{
    oledRefreshFlag = 1U;
}

void OLED_Display_Task(void)
{
    if (oledRefreshFlag)
    {
        oledRefreshFlag = 0U;
        OLED_Display_UpdateLine();
    }
}
