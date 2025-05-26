#include "sdCard.h"
#include "hw_config.h"
#include "f_util.h"
#include "ff.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

static FATFS fs;
static Node* langList = NULL;
static Node* letterList = NULL;

bool sdCardInit()
{
    FRESULT fr = f_mount(&fs, "", 1);
    if (FR_OK != fr) {
        panic("f_mount error: %s (%d)\n", FRESULT_str(fr), fr);
        return false;
    }
    return true;
}

static void freeList(Node* head) {
    while (head) {
        Node* next = head->next;
        free(head);
        head = next;
    }
}

Node* sdCardGetLanguageList(void) {
    DIR dir;
    FILINFO fno;
    FRESULT fr;

    printf("Reading folders from root...\n");

    fr = f_opendir(&dir, "/");
    if (fr != FR_OK) {
        printf("f_opendir error: %s (%d)\n", FRESULT_str(fr), fr);
        return NULL;
    }

    if (langList) {
        freeList(langList);
        langList = NULL;
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

        if (0 == strcmp("misc", fno.fname)) {
            continue; // ignore app files dir
        }

        if (fno.fattrib & AM_DIR) {
            Node* node = malloc(sizeof(Node));
            if (!node) {
                printf("Memory allocation failed\n");
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

            printf("Directory found: %s\n", node->name);
        }
    }

    f_closedir(&dir);

    langList = head;
    return head;
}

Node* sdCardGetLetters(const char* languageFolder)
{
    char path[256] = {0};
    snprintf(path, sizeof(path), "/%s/letters", languageFolder);

    DIR dir;
    FILINFO fno;
    FRESULT fr;

    fr = f_opendir(&dir, path);
    if (fr != FR_OK) {
        printf("Cannot open folder '%s': %s (%d)\n", path, FRESULT_str(fr), fr);
        return NULL;
    }

    // clear previous list
    if (letterList) {
        freeList(letterList);
        letterList = NULL;
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
        size_t len = strlen(fname);
        if (len < 5) {
            continue;
        }
        const char* ext = fname + len - 4; // Pointer to the start of the potential extension (e.g., ".wav")
        if (strcasecmp(ext, ".wav") != 0) { // Check if it's a .wav file (case-insensitive)
             continue; // Skip if it's not a .wav file
        }

        Node* node = malloc(sizeof(Node));
        if (!node) {
            printf("Memory allocation failed\n");
            break;
        }

        int i = 0;
        while (fname[i] != '.') {
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

        printf("Letter found: %s\n", node->name);
    }

    f_closedir(&dir);
    letterList = head;

    return head;
}
