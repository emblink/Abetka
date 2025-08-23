#include <assert.h>
#include "appMode.h"
#include "appModeIdle.h"
#include "appModeRead.h"
#include "appModeWrite.h"
#include "appModeDischarged.h"
#include "appModeMenu.h"
#include "appModeMusic.h"
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
        case APP_MODE_MUSIC:
            appModeMusicProcess();
            break;
        case APP_MODE_MENU:
            appModeMenuProcess();
            break;
        case APP_MODE_DISCHARGED:
            appModeDischargedProcess();
            break;
        case APP_MODE_SLEEP:
            // TODO: O.T. process to sleep mode
            break;
        default:
            assert(0);
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
        case APP_MODE_MUSIC:
            appModeMusicExit();
            break;
        case APP_MODE_MENU:
            appModeMenuExit();
            break;
        case APP_MODE_DISCHARGED:
            appModeDischargedExit();
            break;
        case APP_MODE_SLEEP:
            // TODO: O.T. exit to sleep mode
            break;
        default:
            assert(0);
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
        case APP_MODE_MUSIC:
            appModeMusicEnter();
            break;
        case APP_MODE_MENU:
            appModeMenuEnter();
            break;
        case APP_MODE_DISCHARGED:
            appModeDischargedEnter();
            break;
        case APP_MODE_SLEEP:
            // TODO: O.T. enter sleep mode
            break;
        default:
            assert(0);
            break;
    }
}