#include "shell.h"
#include "builtin.h"
#include "lua_kernel.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>

/* 宽字符转 UTF-8 */
static int wide_to_utf8(const wchar_t *wide, char *utf8, int utf8_len) {
    return WideCharToMultiByte(CP_UTF8, 0, wide, -1, utf8, utf8_len, NULL, NULL);
}

void shell_init(wash_state_t *state) {
    /* 使用宽字符 API 获取当前目录，再转 UTF-8 */
    wchar_t wdir[MAX_PATH_LEN];
    DWORD len = GetCurrentDirectoryW(MAX_PATH_LEN, wdir);
    if (len == 0 || len > MAX_PATH_LEN) {
        strcpy(state->cwd, "C:\\");
    } else {
        wide_to_utf8(wdir, state->cwd, MAX_PATH_LEN);
    }
    strcpy(state->prompt, "wash> ");

    /* 初始化 Lua 内核 */
    if (lua_kernel_init(state) != 0) {
        fprintf(stderr, "wash: failed to initialize lua kernel\n");
        exit(1);
    }
}

int shell_read_line(char *buf, size_t bufsize, wash_state_t *state) {
    printf("%s %s", state->cwd, state->prompt);
    fflush(stdout);

    HANDLE hStdin = GetStdHandle(STD_INPUT_HANDLE);
    DWORD mode;
    BOOL isConsole = GetConsoleMode(hStdin, &mode);

    if (isConsole) {
        /* 控制台输入：用 ReadConsoleW 读宽字符，再转 UTF-8 */
        wchar_t wbuf[MAX_INPUT];
        DWORD read = 0;
        if (!ReadConsoleW(hStdin, wbuf, MAX_INPUT - 1, &read, NULL)) {
            return 0;
        }
        wbuf[read] = L'\0';
        /* 去掉末尾换行符（宽字符） */
        while (read > 0 && (wbuf[read-1] == L'\n' || wbuf[read-1] == L'\r' || wbuf[read-1] == L'\0')) {
            wbuf[--read] = L'\0';
        }
        /* 宽字符转 UTF-8 */
        WideCharToMultiByte(CP_UTF8, 0, wbuf, -1, buf, (int)bufsize, NULL, NULL);
        return 1;
    } else {
        /* 管道/重定向输入：用 fgets */
        if (fgets(buf, (int)bufsize, stdin) == NULL) {
            return 0;
        }
        size_t len = strlen(buf);
        while (len > 0 && (buf[len-1] == '\n' || buf[len-1] == '\r')) {
            buf[--len] = '\0';
        }
        return 1;
    }
}

int shell_parse(const char *input, char *argv[], int max_args) {
    int argc = 0;
    const char *p = input;
    static char buf[MAX_INPUT];
    char *out = buf;

    while (*p && argc < max_args - 1) {
        /* 跳过空格 */
        while (*p == ' ' || *p == '\t') p++;
        if (!*p) break;

        argv[argc] = out;
        int in_quote = 0;

        while (*p) {
            if (*p == '"') {
                in_quote = !in_quote;
                p++;
                continue;
            }
            if ((*p == ' ' || *p == '\t') && !in_quote) {
                break;
            }
            *out++ = *p++;
        }
        *out++ = '\0';
        argc++;
    }
    argv[argc] = NULL;
    return argc;
}

void shell_main_loop(wash_state_t *state) {
    char input[MAX_INPUT];
    char *argv[MAX_ARGS];

    printf("WASH (Wash Aren't Shell) v%s\n", WASH_VERSION);
    printf("C kernel + Lua VM - type 'help' for commands.\n\n");

    while (1) {
        if (!shell_read_line(input, MAX_INPUT, state)) {
            printf("\n");
            break;
        }

        /* 跳过空行 */
        if (input[0] == '\0') continue;

        int argc = shell_parse(input, argv, MAX_ARGS);
        if (argc == 0) continue;

        /* 1. 内置命令 */
        builtin_func_t fn = builtin_lookup(argv[0]);
        if (fn) {
            if (fn(argc, argv, state) == 1) {
                break; /* exit */
            }
            continue;
        }

        /* 2. Lua 插件命令 */
        int rc = lua_kernel_run_module(argv[0], argc - 1, argv + 1);
        if (rc == 0) continue;

        /* 3. 未找到 */
        printf("wash: command not found: %s\n", argv[0]);
        printf("  type 'help' to see available commands\n");
    }

    lua_kernel_close();
}
