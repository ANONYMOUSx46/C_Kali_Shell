#ifndef CONFIG_H
#define CONFIG_H

#define PROMPT_MAX_LEN 256

typedef enum {
    THEME_LIGHT,
    THEME_DARK,
    THEME_DEFAULT = THEME_LIGHT
} theme_t;

typedef struct {
    char prompt_format[PROMPT_MAX_LEN];
    theme_t theme;
} shell_config_t;

void config_init(shell_config_t *config);
int config_load(shell_config_t *config);

#endif // CONFIG_H
