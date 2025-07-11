#include <ctype.h>
#include <string.h>

// Trim whitespace from both ends of a string
char *trim_whitespace(char *str) {
    if (!str) return NULL;

    // Skip leading whitespace
    while (isspace((unsigned char)*str)) str++;

    if (*str == '\0') return str;

    // Remove trailing whitespace
    char *end = str + strlen(str) - 1;
    while (end > str && isspace((unsigned char)*end)) end--;

    *(end + 1) = '\0';
    return str;
}
