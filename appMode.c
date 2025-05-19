#include "appMode.h"
#include "appModeIdle.h"
#include "appModeRead.h"
#include "appModeWrite.h"
#include "lvgl.h"

static AppMode appMode = APP_MODE_IDLE;

AppMode appModeGetCurrent()
{
    return appMode;
}

void appModeProcess()
{
    switch (appMode) {
        case APP_MODE_IDLE:
            appModeIdleProcess();
            break;
        case APP_MODE_READ_CARD:
            appModeReadProcess();
            break;
        case APP_MODE_WRITE_CARD:
            appModeWriteProcess();
            break;
        case APP_MODE_UI_MENU:
            // Future use
            break;
    }
}

void appModeSwitch(AppMode newMode)
{
    switch (appMode) {
        case APP_MODE_IDLE:
            // Maybe display idle screen or animation
            appModeIdleExit();
            break;
        case APP_MODE_READ_CARD:
            appModeReadExit();
            break;
        case APP_MODE_WRITE_CARD:
            appModeWriteExit();
            break;
        case APP_MODE_UI_MENU:
            // Future use
            break;
    }

    lv_obj_clean(lv_scr_act());
    appMode = newMode;

    switch (newMode) {
        case APP_MODE_IDLE:
            // Maybe display idle screen or animation
            appModeIdleEnter();
            break;
        case APP_MODE_READ_CARD:
            appModeReadEnter();
            break;
        case APP_MODE_WRITE_CARD:
            appModeWriteEnter();
            break;
        case APP_MODE_UI_MENU:
            // Future use
            break;
    }
}