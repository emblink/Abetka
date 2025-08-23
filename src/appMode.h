#pragma once

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    APP_MODE_IDLE,
    APP_MODE_READ_CARD,
    APP_MODE_MUSIC,
    APP_MODE_WRITE_CARD,
    APP_MODE_MENU,
    APP_MODE_SLEEP,
    APP_MODE_DISCHARGED,
} AppMode;

AppMode appModeGetCurrent(void);
void appModeProcess(void);
void appModeSwitch(AppMode newMode);

#ifdef __cplusplus
}
#endif