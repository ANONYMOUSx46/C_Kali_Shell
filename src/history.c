// src/history.c
#define _GNU_SOURCE
#include "history.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_HISTORY 1000

static char *history[MAX_HISTORY];
static int history_count = 0;
static char history_filename[512] = {0};

void history_init(const char *filename) {
    if (!filename) return;
    strncpy(history_filename, filename, sizeof(history_filename)-1);
    FILE *fp = fopen(history_filename, "r");
    if (!fp) return;

    char *line = NULL;
    size_t len = 0;
    ssize_t read;
    while ((read=getline(&line, &len, fp)) != -1) {
        if (read>0 && (line[read-1] == '\n' || line[read-1] == '\r')) line[read-1] = 0;
        if (history_count < MAX_HISTORY) {
            history[history_count++] = strdup(line);
        }
    }
    free(line);
    fclose(fp);
}

void history_add(const char *line) {
    if (!line || line[0]=='\0') return;
    // ignore duplicates of last command
    if (history_count > 0 && strcmp(history[history_count-1], line) == 0)
        return;

    if (history_count == MAX_HISTORY) {
        free(history[0]);
        for (int i=1; i < MAX_HISTORY; i++) {
            history[i-1] = history[i];
        }
        history_count--;
    }
    history[history_count++] = strdup(line);
}

void history_save(void) {
    if (history_count == 0 || history_filename[0] == 0) return;
    FILE *fp = fopen(history_filename, "w");
    if (!fp) return;
    for (int i=0; i < history_count; i++) {
        fprintf(fp, "%s\n", history[i]);
    }
    fclose(fp);
}

void history_free(void) {
    for (int i=0; i < history_count; i++) {
        free(history[i]);
    }
    history_count = 0;
}
