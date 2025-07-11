// src/history.h
#ifndef HISTORY_H
#define HISTORY_H

// Initialize history module and load from file
void history_init(const char *filename);

// Add command line to history in-memory
void history_add(const char *line);

// Save history to disk
void history_save(void);

// Free all resources used by history
void history_free(void);

#endif
