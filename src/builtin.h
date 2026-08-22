#ifndef WASH_BUILTIN_H
#define WASH_BUILTIN_H

#include "shell.h"

/* 内置命令返回值：0 正常执行，1 退出shell */
typedef int (*builtin_func_t)(int argc, char *argv[], wash_state_t *state);

/* 查找内置命令，返回函数指针或 NULL */
builtin_func_t builtin_lookup(const char *name);

/* 列出所有内置命令名（用于 help） */
void builtin_list_names(void);

/* 具体内置命令 */
int cmd_exit(int argc, char *argv[], wash_state_t *state);
int cmd_cd(int argc, char *argv[], wash_state_t *state);
int cmd_pwd(int argc, char *argv[], wash_state_t *state);
int cmd_clear(int argc, char *argv[], wash_state_t *state);
int cmd_help(int argc, char *argv[], wash_state_t *state);
int cmd_modules(int argc, char *argv[], wash_state_t *state);
int cmd_unrestrict(int argc, char *argv[], wash_state_t *state);
int cmd_restrict(int argc, char *argv[], wash_state_t *state);

#endif /* WASH_BUILTIN_H */
