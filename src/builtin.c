#include "builtin.h"
#include "lua_kernel.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>

/* UTF-8 转宽字符 */
static int utf8_to_wide(const char *utf8, wchar_t *wide, int wide_len) {
    return MultiByteToWideChar(CP_UTF8, 0, utf8, -1, wide, wide_len);
}

/* 宽字符转 UTF-8 */
static int wide_to_utf8(const wchar_t *wide, char *utf8, int utf8_len) {
    return WideCharToMultiByte(CP_UTF8, 0, wide, -1, utf8, utf8_len, NULL, NULL);
}

/* 内置命令表 */
static struct {
    const char *name;
    builtin_func_t func;
} builtins[] = {
    {"exit",       cmd_exit},
    {"cd",         cmd_cd},
    {"pwd",        cmd_pwd},
    {"clear",      cmd_clear},
    {"help",       cmd_help},
    {"modules",    cmd_modules},
    {"unrestrict", cmd_unrestrict},
    {"restrict",   cmd_restrict},
    {NULL, NULL}
};

builtin_func_t builtin_lookup(const char *name) {
    for (int i = 0; builtins[i].name; i++) {
        if (strcmp(builtins[i].name, name) == 0) {
            return builtins[i].func;
        }
    }
    return NULL;
}

void builtin_list_names(void) {
    for (int i = 0; builtins[i].name; i++) {
        printf("  %s\n", builtins[i].name);
    }
}

int cmd_exit(int argc, char *argv[], wash_state_t *state) {
    printf("Wash Aren't Shell - goodbye.\n");
    return 1; /* 通知主循环退出 */
}

int cmd_cd(int argc, char *argv[], wash_state_t *state) {
    if (argc < 2) {
        printf("%s\n", state->cwd);
        return 0;
    }

    const char *target = argv[1];
    char newpath[MAX_PATH_LEN];

    /* 特殊目录 */
    if (strcmp(target, "..") == 0) {
        strcpy(newpath, state->cwd);
        char *last_slash = strrchr(newpath, '\\');
        if (last_slash && last_slash != newpath) {
            *last_slash = '\0';
        } else if (last_slash == newpath) {
            newpath[1] = '\0';
        }
        strcpy(state->cwd, newpath);
        return 0;
    }

    if (strcmp(target, "~") == 0) {
        /* USERPROFILE 环境变量是 ANSI 编码，需要转换为 UTF-8 */
        const char *up = getenv("USERPROFILE");
        if (up) {
            wchar_t wpath[MAX_PATH_LEN];
            MultiByteToWideChar(CP_ACP, 0, up, -1, wpath, MAX_PATH_LEN);
            wide_to_utf8(wpath, state->cwd, MAX_PATH_LEN);
        }
        return 0;
    }

    /* 拼接路径（UTF-8 字符串操作） */
    if (target[1] == ':' || target[0] == '\\' || target[0] == '/') {
        strncpy(newpath, target, MAX_PATH_LEN - 1);
    } else {
        snprintf(newpath, MAX_PATH_LEN, "%s\\%s", state->cwd, target);
    }
    newpath[MAX_PATH_LEN - 1] = '\0';

    /* 转宽字符后用 W 版本 API 检查和规范化 */
    wchar_t wpath[MAX_PATH_LEN];
    utf8_to_wide(newpath, wpath, MAX_PATH_LEN);

    DWORD attr = GetFileAttributesW(wpath);
    if (attr != INVALID_FILE_ATTRIBUTES && (attr & FILE_ATTRIBUTE_DIRECTORY)) {
        wchar_t wfull[MAX_PATH_LEN];
        if (GetFullPathNameW(wpath, MAX_PATH_LEN, wfull, NULL)) {
            wide_to_utf8(wfull, state->cwd, MAX_PATH_LEN);
        } else {
            strcpy(state->cwd, newpath);
        }
    } else {
        printf("cd: no such directory: %s\n", newpath);
    }
    return 0;
}

int cmd_pwd(int argc, char *argv[], wash_state_t *state) {
    printf("%s\n", state->cwd);
    return 0;
}

int cmd_clear(int argc, char *argv[], wash_state_t *state) {
    system("cls");
    return 0;
}

int cmd_help(int argc, char *argv[], wash_state_t *state) {
    printf("WASH (Wash Aren't Shell) v%s\n", WASH_VERSION);
    printf("\n");
    printf("Built-in commands:\n");
    printf("  exit       - exit wash\n");
    printf("  cd         - change directory\n");
    printf("  pwd        - print working directory\n");
    printf("  clear      - clear screen\n");
    printf("  help       - show this help\n");
    printf("  modules    - list available modules\n");
    printf("  unrestrict - switch to unrestricted mode (enable os.execute, io.*, etc.)\n");
    printf("  restrict   - switch back to restricted mode\n");
    printf("\n");
    printf("Available modules (in wash-modules\\):\n");
    lua_kernel_list_modules();
    printf("\n");
    printf("Note: C kernel with embedded Lua 5.1 VM.\n");
    return 0;
}

int cmd_modules(int argc, char *argv[], wash_state_t *state) {
    printf("Available modules:\n");
    lua_kernel_list_modules();
    return 0;
}

int cmd_unrestrict(int argc, char *argv[], wash_state_t *state) {
    if (lua_kernel_is_unrestricted()) {
        printf("already in unrestricted mode.\n");
        return 0;
    }
    printf("WARNING: unrestricted mode disables all API restrictions.\n");
    printf("Plugins will have access to os.execute, io.*, loadfile, etc.\n");
    printf("Only use this mode with trusted plugins.\n");
    lua_kernel_unrestrict();
    return 0;
}

int cmd_restrict(int argc, char *argv[], wash_state_t *state) {
    if (!lua_kernel_is_unrestricted()) {
        printf("already in restricted mode.\n");
        return 0;
    }
    lua_kernel_restrict();
    return 0;
}
