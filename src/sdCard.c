#include "sdCard.h"
#include "hw_config.h"
#include "f_util.h"
#include "ff.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

static FATFS fs;

bool sdCardInit()
{
    FRESULT fr = f_mount(&fs, "", 1);
    if (FR_OK != fr) {
        panic("f_mount error: %s (%d)\n", FRESULT_str(fr), fr);
        return false;
    }
    return true;
}

void freeList(Node* head) {
    while (head) {
        Node* next = head->next;
        free(head);
        head = next;
    }
}

Node* sdCardGetDirsInFolder(const char* folderPath) {
    DIR dir;
    FILINFO fno;
    FRESULT fr;

    fr = f_opendir(&dir, folderPath);
    if (fr != FR_OK) {
        printf("f_opendir error: %s (%d)\n", FRESULT_str(fr), fr);
        return NULL;
    }

    Node* head = NULL;
    Node* tail = NULL;

    while (true) {
        fr = f_readdir(&dir, &fno);
        if (fr != FR_OK || fno.fname[0] == 0) {
            break;
        }

        if (fno.fattrib & (AM_SYS | AM_HID)) {
            continue;
        }            

        if (fno.fattrib & AM_DIR) {
            Node* node = malloc(sizeof(Node));
            if (!node) {
                printf("Memory allocation failed\n");
                freeList(head);
                head = NULL;
                break;
            }

            strncpy(node->name, fno.fname, sizeof(node->name) - 1);
            node->name[sizeof(node->name) - 1] = '\0';
            node->next = NULL;
            node->prev = NULL;

            if (!head) {
                head = tail = node;
                head->prev = NULL;
            } else {
                tail->next = node;
                tail->next->prev = tail;
                tail = node;
            }
        }
    }

    f_closedir(&dir);
    return head;
}

Node* sdCardGetLanguageList(void) {
    Node* langList = sdCardGetDirsInFolder("/");
    return langList;
}

Node* sdCardGetLetters(const char* languageFolder)
{
    char path[256] = {0};
    snprintf(path, sizeof(path), "/%s/letters", languageFolder);
    Node* letterList = sdCardGetFilesInFolder(path, "wav");
    return letterList;
}

Node* sdCardGetFilesInFolder(const char* path, const char *fileExtension)
{
    DIR dir;
    FILINFO fno;
    FRESULT fr;

    fr = f_opendir(&dir, path);
    if (fr != FR_OK) {
        printf("Cannot open folder '%s': %s (%d)\n", path, FRESULT_str(fr), fr);
        return NULL;
    }

    Node* head = NULL;
    Node* tail = NULL;

    while (true) {
        fr = f_readdir(&dir, &fno);
        if (fr != FR_OK || fno.fname[0] == 0) {
            break;
        }

        // Skip directories and hidden/system files
        if (fno.fattrib & (AM_DIR | AM_SYS | AM_HID)) {
            continue;
        }

        // Check for .wav extension (case-insensitive)
        const char* fname = fno.fname;
        const char* ext = strrchr(fname, '.');
        if (!ext || strcasecmp(ext + 1, fileExtension) != 0) {
            continue;
        }

        Node* node = malloc(sizeof(Node));
        if (!node) {
            printf("Memory allocation failed\n");
            freeList(head);
            head = NULL;
            break;
        }

        int i = 0;
        while (fname[i] != '.' && fname[i] != '\0' && i < sizeof(node->name) - 1) {
            node->name[i] = fname[i];
            i++;
        }
        node->name[i] = '\0';
        node->next = NULL;
        node->prev = NULL;

        if (!head) {
            head = tail = node;
            head->prev = NULL;
        } else {
            tail->next = node;
            tail->next->prev = tail;
            tail = node;
        }
    }

    f_closedir(&dir);

    return head;
}
