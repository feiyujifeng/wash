#ifndef WASH_SHELL_H
#define WASH_SHELL_H

#include <stddef.h>

#define WASH_VERSION "0.3.0"
#define MAX_INPUT 4096
#define MAX_ARGS 256
#define MAX_PATH_LEN 32768

/* 会话状态（内核维护） */
typedef struct {
    char cwd[MAX_PATH_LEN];
    char prompt[64];
} wash_state_t;

/* 初始化会话状态 */
void shell_init(wash_state_t *state);

/* 读取一行输入，返回 0 表示 EOF */
int shell_read_line(char *buf, size_t bufsize, wash_state_t *state);

/* 命令行分词，处理双引号，返回参数个数 */
int shell_parse(const char *input, char *argv[], int max_args);

/* 主循环 */
void shell_main_loop(wash_state_t *state);

#endif /* WASH_SHELL_H */
