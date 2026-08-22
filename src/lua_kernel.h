#ifndef WASH_LUA_KERNEL_H
#define WASH_LUA_KERNEL_H

#include "shell.h"

/* 初始化 Lua 虚拟机，注册 wash.* API */
int lua_kernel_init(wash_state_t *state);

/* 关闭 Lua 虚拟机 */
void lua_kernel_close(void);

/* 加载并执行插件，返回 0 成功，-1 插件不存在，-2 运行错误 */
int lua_kernel_run_module(const char *name, int argc, char *argv[]);

/* 列出插件目录下所有可用插件 */
void lua_kernel_list_modules(void);

/* 切换到不受限模式：恢复所有被屏蔽的原生接口 */
void lua_kernel_unrestrict(void);

/* 切回受限模式：重新屏蔽原生接口 */
void lua_kernel_restrict(void);

/* 查询当前是否为不受限模式 */
int lua_kernel_is_unrestricted(void);

#endif /* WASH_LUA_KERNEL_H */
