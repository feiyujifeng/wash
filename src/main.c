#include "shell.h"
#include "lua_kernel.h"
#include <stdio.h>
#include <string.h>
#include <windows.h>

/*
 * 编译宏控制默认模式：
 *   -DWASH_DEFAULT_UNRESTRICTED  -> 激进版（默认不受限）
 *   不定义                       -> 稳定版（默认受限）
 */
#ifdef WASH_DEFAULT_UNRESTRICTED
#define WASH_FLAVOR "激进版 (Unrestricted)"
#else
#define WASH_FLAVOR "稳定版 (Restricted)"
#endif

int main(int argc, char *argv[]) {
    /* 设置控制台为 UTF-8 代码页（输入+输出） */
    SetConsoleCP(CP_UTF8);
    SetConsoleOutputCP(CP_UTF8);

    int start_unrestricted = 0;

#ifdef WASH_DEFAULT_UNRESTRICTED
    start_unrestricted = 1;  /* 激进版默认不受限 */
#endif

    /* 解析命令行参数 */
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--unrestricted") == 0 || strcmp(argv[i], "-u") == 0) {
            start_unrestricted = 1;
        } else if (strcmp(argv[i], "--restricted") == 0 || strcmp(argv[i], "-r") == 0) {
            start_unrestricted = 0;  /* 激进版也可强制受限启动 */
        } else if (strcmp(argv[i], "--help") == 0 || strcmp(argv[i], "-h") == 0) {
            printf("WASH (Wash Aren't Shell) v%s - %s\n", WASH_VERSION, WASH_FLAVOR);
            printf("Usage: wash [options]\n");
            printf("\n");
            printf("Options:\n");
            printf("  -u, --unrestricted   Start in unrestricted mode\n");
            printf("  -r, --restricted     Start in restricted mode\n");
            printf("  -h, --help           Show this help\n");
            return 0;
        }
    }

    wash_state_t state;
    shell_init(&state);

    /* 根据配置切换初始模式 */
    if (start_unrestricted) {
        lua_kernel_unrestrict();
    }

    printf("Build flavor: %s\n", WASH_FLAVOR);
    shell_main_loop(&state);

    return 0;
}
