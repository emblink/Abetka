#pragma once
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct Node {
    char name[64];
    struct Node* next;
    struct Node* prev;
} Node;

bool sdCardInit();
Node* sdCardGetLanguageList(void);
Node* sdCardGetLetters(const char* languageFolder);

#ifdef __cplusplus
}
#endif