#include "lua_kernel.h"
#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <wchar.h>
#include <windows.h>
#include <direct.h>

static lua_State *L = NULL;
static wash_state_t *g_state = NULL;
static int g_unrestricted = 0;  /* 0=受限模式, 1=不受限模式 */

/* UTF-8 转宽字符 */
static int utf8_to_wide(const char *utf8, wchar_t *wide, int wide_len) {
    return MultiByteToWideChar(CP_UTF8, 0, utf8, -1, wide, wide_len);
}

/* 宽字符转 UTF-8 */
static int wide_to_utf8(const wchar_t *wide, char *utf8, int utf8_len) {
    return WideCharToMultiByte(CP_UTF8, 0, wide, -1, utf8, utf8_len, NULL, NULL);
}

/* 把相对路径（UTF-8）拼接 cwd 后转宽字符 */
static void resolve_path_wide(const char *path, wchar_t *out, int out_len) {
    char full[MAX_PATH_LEN];
    if (path[1] == ':' || path[0] == '\\' || path[0] == '/') {
        strncpy(full, path, MAX_PATH_LEN - 1);
        full[MAX_PATH_LEN - 1] = '\0';
    } else {
        snprintf(full, MAX_PATH_LEN, "%s\\%s", g_state->cwd, path);
    }
    utf8_to_wide(full, out, out_len);
}

/* ============================================================
 * wash.* API 实现（C 函数暴露给 Lua）
 * ============================================================ */

/* wash.get_cwd() */
static int api_get_cwd(lua_State *L) {
    lua_pushstring(L, g_state->cwd);
    return 1;
}

/* wash.print(text) */
static int api_print(lua_State *L) {
    const char *s = luaL_checkstring(L, 1);
    printf("%s\n", s);
    fflush(stdout);
    return 0;
}

/* wash.read_dir(path) -> table */
static int api_read_dir(lua_State *L) {
    const char *path = luaL_optstring(L, 1, g_state->cwd);

    wchar_t wpath[MAX_PATH_LEN];
    resolve_path_wide(path, wpath, MAX_PATH_LEN);

    wchar_t pattern[MAX_PATH_LEN];
    wcscpy(pattern, wpath);
    wcscat(pattern, L"\\*");

    WIN32_FIND_DATAW fd;
    HANDLE hFind = FindFirstFileW(pattern, &fd);

    lua_newtable(L);
    int idx = 1;

    if (hFind != INVALID_HANDLE_VALUE) {
        do {
            if (wcscmp(fd.cFileName, L".") == 0 || wcscmp(fd.cFileName, L"..") == 0)
                continue;
            char name[MAX_PATH_LEN];
            wide_to_utf8(fd.cFileName, name, MAX_PATH_LEN);
            lua_pushstring(L, name);
            lua_rawseti(L, -2, idx++);
        } while (FindNextFileW(hFind, &fd));
        FindClose(hFind);
    }
    return 1;
}

/* wash.make_dir(path) -> boolean */
static int api_make_dir(lua_State *L) {
    const char *path = luaL_checkstring(L, 1);
    wchar_t wpath[MAX_PATH_LEN];
    resolve_path_wide(path, wpath, MAX_PATH_LEN);
    BOOL ok = CreateDirectoryW(wpath, NULL);
    lua_pushboolean(L, ok);
    return 1;
}

/* wash.remove_file(path) -> boolean */
static int api_remove_file(lua_State *L) {
    const char *path = luaL_checkstring(L, 1);
    wchar_t wpath[MAX_PATH_LEN];
    resolve_path_wide(path, wpath, MAX_PATH_LEN);
    BOOL ok = DeleteFileW(wpath);
    lua_pushboolean(L, ok);
    return 1;
}

/* wash.read_file(path) -> string or nil, err */
static int api_read_file(lua_State *L) {
    const char *path = luaL_checkstring(L, 1);
    wchar_t wpath[MAX_PATH_LEN];
    resolve_path_wide(path, wpath, MAX_PATH_LEN);

    FILE *f = _wfopen(wpath, L"rb");
    if (!f) {
        lua_pushnil(L);
        lua_pushstring(L, "cannot open file");
        return 2;
    }
    fseek(f, 0, SEEK_END);
    long size = ftell(f);
    fseek(f, 0, SEEK_SET);

    char *buf = malloc(size + 1);
    if (!buf) {
        fclose(f);
        lua_pushnil(L);
        lua_pushstring(L, "out of memory");
        return 2;
    }
    fread(buf, 1, size, f);
    buf[size] = '\0';
    fclose(f);

    lua_pushlstring(L, buf, size);
    free(buf);
    return 1;
}

/* wash.write_file(path, content) -> boolean */
static int api_write_file(lua_State *L) {
    const char *path = luaL_checkstring(L, 1);
    size_t len;
    const char *content = luaL_checklstring(L, 2, &len);

    wchar_t wpath[MAX_PATH_LEN];
    resolve_path_wide(path, wpath, MAX_PATH_LEN);

    FILE *f = _wfopen(wpath, L"wb");
    if (!f) {
        lua_pushboolean(L, 0);
        return 1;
    }
    fwrite(content, 1, len, f);
    fclose(f);
    lua_pushboolean(L, 1);
    return 1;
}

/* wash.spawn_exe(exe, args_table) -> boolean */
static int api_spawn_exe(lua_State *L) {
    const char *exe = luaL_checkstring(L, 1);

    wchar_t cmdline[8192];
    wchar_t wexe[MAX_PATH_LEN];
    utf8_to_wide(exe, wexe, MAX_PATH_LEN);
    wcscpy(cmdline, wexe);

    if (lua_istable(L, 2)) {
        lua_pushnil(L);
        while (lua_next(L, 2) != 0) {
            const char *arg = lua_tostring(L, -1);
            if (arg) {
                wchar_t warg[MAX_PATH_LEN];
                utf8_to_wide(arg, warg, MAX_PATH_LEN);
                wcscat(cmdline, L" \"");
                wcscat(cmdline, warg);
                wcscat(cmdline, L"\"");
            }
            lua_pop(L, 1);
        }
    }

    int rc = _wsystem(cmdline);
    lua_pushboolean(L, rc == 0);
    return 1;
}

/* ============================================================
 * 注册 wash.* API 到 Lua
 * ============================================================ */
static void register_wash_api(lua_State *L) {
    lua_newtable(L);

    lua_pushcfunction(L, api_get_cwd);
    lua_setfield(L, -2, "get_cwd");

    lua_pushcfunction(L, api_print);
    lua_setfield(L, -2, "print");

    lua_pushcfunction(L, api_read_dir);
    lua_setfield(L, -2, "read_dir");

    lua_pushcfunction(L, api_make_dir);
    lua_setfield(L, -2, "make_dir");

    lua_pushcfunction(L, api_remove_file);
    lua_setfield(L, -2, "remove_file");

    lua_pushcfunction(L, api_read_file);
    lua_setfield(L, -2, "read_file");

    lua_pushcfunction(L, api_write_file);
    lua_setfield(L, -2, "write_file");

    lua_pushcfunction(L, api_spawn_exe);
    lua_setfield(L, -2, "spawn_exe");

    lua_setglobal(L, "wash");
}

/* ============================================================
 * 屏蔽高危接口（架构分层，非安全沙箱）
 * ============================================================ */
static void lockdown_env(lua_State *L) {
    /* 移除 os 库中的危险函数 */
    lua_getglobal(L, "os");
    if (lua_istable(L, -1)) {
        lua_pushnil(L);
        lua_setfield(L, -2, "execute");
        lua_pushnil(L);
        lua_setfield(L, -2, "remove");
        lua_pushnil(L);
        lua_setfield(L, -2, "rename");
        lua_pushnil(L);
        lua_setfield(L, -2, "exit");
        lua_pushnil(L);
        lua_setfield(L, -2, "tmpname");
    }
    lua_pop(L, 1);

    /* 移除 io 库（插件不允许直接操作文件，必须走 wash.* API） */
    lua_pushnil(L);
    lua_setglobal(L, "io");

    /* 移除 loadfile / dofile / require（插件不能加载任意脚本） */
    lua_pushnil(L);
    lua_setglobal(L, "loadfile");
    lua_pushnil(L);
    lua_setglobal(L, "dofile");
    lua_pushnil(L);
    lua_setglobal(L, "require");
}

/* ============================================================
 * 恢复被屏蔽的接口（不受限模式）
 * ============================================================ */
static void restore_env(lua_State *L) {
    /* 通过 lua_call 正确打开各库（不能直接调用 luaopen_*） */
    lua_pushcfunction(L, luaopen_base);
    lua_pushstring(L, "");
    lua_call(L, 1, 0);  /* 恢复 loadfile / dofile / print 等 */

    /* 通过 lua_pcall 打开 io 库，捕获错误 */
    lua_pushcfunction(L, luaopen_io);
    lua_pushstring(L, LUA_IOLIBNAME);
    if (lua_pcall(L, 1, 1, 0) != 0) {
        printf("[debug] luaopen_io error: %s\n", lua_tostring(L, -1));
        lua_pop(L, 1);
    } else {
        lua_setglobal(L, "io");
    }

    lua_pushcfunction(L, luaopen_os);
    lua_pushstring(L, LUA_OSLIBNAME);
    lua_call(L, 1, 0);  /* 恢复 os.execute / os.remove 等 */

    lua_pushcfunction(L, luaopen_package);
    lua_pushstring(L, LUA_LOADLIBNAME);
    lua_call(L, 1, 0);  /* 恢复 require */

    lua_pushcfunction(L, luaopen_table);
    lua_pushstring(L, LUA_TABLIBNAME);
    lua_call(L, 1, 0);

    lua_pushcfunction(L, luaopen_string);
    lua_pushstring(L, LUA_STRLIBNAME);
    lua_call(L, 1, 0);

    lua_pushcfunction(L, luaopen_math);
    lua_pushstring(L, LUA_MATHLIBNAME);
    lua_call(L, 1, 0);

    lua_pushcfunction(L, luaopen_debug);
    lua_pushstring(L, LUA_DBLIBNAME);
    lua_call(L, 1, 0);

    /* 重新注册 wash.* API，确保不被覆盖 */
    register_wash_api(L);
}

/* ============================================================
 * 公开接口
 * ============================================================ */

int lua_kernel_init(wash_state_t *state) {
    g_state = state;

    L = luaL_newstate();
    if (!L) return -1;

    /* 打开基础库（table, string, math, os(部分), coroutine, package, debug） */
    luaL_openlibs(L);

    /* 注册 wash.* API */
    register_wash_api(L);

    /* 屏蔽高危接口 */
    lockdown_env(L);

    return 0;
}

void lua_kernel_close(void) {
    if (L) {
        lua_close(L);
        L = NULL;
    }
}

void lua_kernel_unrestrict(void) {
    if (!L) return;
    restore_env(L);
    g_unrestricted = 1;
    printf("[wash] switched to UNRESTRICTED mode\n");
}

void lua_kernel_restrict(void) {
    if (!L) return;
    lockdown_env(L);
    g_unrestricted = 0;
    printf("[wash] switched to RESTRICTED mode\n");
}

int lua_kernel_is_unrestricted(void) {
    return g_unrestricted;
}

int lua_kernel_run_module(const char *name, int argc, char *argv[]) {
    if (!L) return -1;

    char utf8path[MAX_PATH_LEN];
    snprintf(utf8path, MAX_PATH_LEN, "wash-modules\\%s.wash", name);

    /* 用宽字符检查文件是否存在 */
    wchar_t wpath[MAX_PATH_LEN];
    utf8_to_wide(utf8path, wpath, MAX_PATH_LEN);
    DWORD attr = GetFileAttributesW(wpath);
    if (attr == INVALID_FILE_ATTRIBUTES) {
        return -1; /* 插件不存在 */
    }

    /* luaL_loadfile 内部用 fopen，需要 ANSI(GBK) 路径 */
    char ansipath[MAX_PATH_LEN];
    wchar_t wfull[MAX_PATH_LEN];
    /* 先转宽字符，再转 ANSI */
    utf8_to_wide(utf8path, wfull, MAX_PATH_LEN);
    WideCharToMultiByte(CP_ACP, 0, wfull, -1, ansipath, MAX_PATH_LEN, NULL, NULL);

    /* 加载脚本 */
    int rc = luaL_loadfile(L, ansipath);
    if (rc != 0) {
        fprintf(stderr, "wash: module load error: %s\n", lua_tostring(L, -1));
        lua_pop(L, 1);
        return -2;
    }

    /* 传入参数表 */
    lua_newtable(L);
    for (int i = 0; i < argc; i++) {
        lua_pushstring(L, argv[i]);
        lua_rawseti(L, -2, i + 1);
    }

    /* 执行：1个参数（args表），0个返回值 */
    rc = lua_pcall(L, 1, 0, 0);
    if (rc != 0) {
        fprintf(stderr, "wash: module runtime error: %s\n", lua_tostring(L, -1));
        lua_pop(L, 1);
        return -2;
    }

    return 0;
}

void lua_kernel_list_modules(void) {
    WIN32_FIND_DATAW fd;
    HANDLE hFind = FindFirstFileW(L"wash-modules\\*.wash", &fd);

    if (hFind == INVALID_HANDLE_VALUE) {
        printf("  (no modules found)\n");
        return;
    }

    do {
        char name[256];
        wide_to_utf8(fd.cFileName, name, 256);
        char *dot = strrchr(name, '.');
        if (dot) *dot = '\0';
        printf("  %s\n", name);
    } while (FindNextFileW(hFind, &fd));

    FindClose(hFind);
}
