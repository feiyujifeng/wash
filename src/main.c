#include "shell.h"
#include "lua_kernel.h"
#include <stdio.h>
#include <string.h>
#include <windows.h>

/*
 * WASH 默认激进版（不受限模式），全部 Lua 原生接口开放。
 * 可用 -r / --restricted 启动参数强制受限模式。
 */
#define WASH_FLAVOR "激进版 (Unrestricted)"

int main(int argc, char *argv[]) {
    /* 设置控制台为 UTF-8 代码页（输入+输出） */
    SetConsoleCP(CP_UTF8);
    SetConsoleOutputCP(CP_UTF8);

    int start_restricted = 0;  /* 默认不受限 */

    /* 解析命令行参数 */
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--restricted") == 0 || strcmp(argv[i], "-r") == 0) {
            start_restricted = 1;
        } else if (strcmp(argv[i], "--unrestricted") == 0 || strcmp(argv[i], "-u") == 0) {
            start_restricted = 0;  /* 显式指定不受限（默认已是） */
        } else if (strcmp(argv[i], "--help") == 0 || strcmp(argv[i], "-h") == 0) {
            printf("WASH (Wash Aren't Shell) v%s - %s\n", WASH_VERSION, WASH_FLAVOR);
            printf("Usage: wash [options]\n");
            printf("\n");
            printf("Options:\n");
            printf("  -r, --restricted     Start in restricted mode (block os.execute/io/etc)\n");
            printf("  -u, --unrestricted   Start in unrestricted mode (default)\n");
            printf("  -h, --help           Show this help\n");
            return 0;
        }
    }

    wash_state_t state;
    shell_init(&state);

    /* 默认不受限；-r 启动时切换为受限模式 */
    if (start_restricted) {
        lua_kernel_restrict();
    }

    printf("Build flavor: %s\n", WASH_FLAVOR);
    shell_main_loop(&state);

    return 0;
}
