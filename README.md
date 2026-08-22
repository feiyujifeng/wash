# WASH (Wash Aren't Shell)

> C 内核 + 内嵌 Lua 5.1 虚拟机的可扩展 Windows Shell

## 项目定名

**WASH = Wash Aren't Shell**

采用开源经典递归自指命名（对标 GNU 命名体系）。Wash 并非传统意义上的 CMD、Bash、PowerShell 类命令行解释器，而是一款融合宏内核与微内核优势、基于 C 内核与 Lua 虚拟机的**模块化可扩展终端架构实验框架**。

命名采用复数 `Aren't` 而非单数 `Isn't`，因为 Wash 由**常驻核心内核 + 海量可扩展模块化插件**组成复数生态体系，语义贴合架构。

## 架构

- **C 内核** (`src/`)：维护会话状态（CWD）、命令解析、Lua 虚拟机管理、内置命令
- **Lua 插件** (`wash-modules/*.wash`)：纯文本 Lua 脚本，放入目录即可加载，无需编译、无需重启
- **API 收口**：受限模式下插件只能调用 `wash.*` 系列标准化 C 接口，原生 `os.execute`、`io` 库等被屏蔽
- **双模式**：默认受限模式，输入 `unrestrict` 切换不受限（恢复全部 Lua 原生接口），`restrict` 切回

## 快速开始

从 [Releases](https://github.com/飞宇之峰/wash/releases) 下载 `WASH-Portable.exe`，双击解压后运行 `wash.bat`。

或自行编译（见下方）。

## 内置命令

| 命令 | 说明 |
|------|------|
| `exit` | 退出 Wash |
| `cd` | 切换工作目录 |
| `pwd` | 显示当前目录 |
| `clear` | 清屏 |
| `help` | 显示帮助 |
| `modules` | 列出可用插件 |
| `unrestrict` | 切换为不受限模式 |
| `restrict` | 切回受限模式 |

## 内置插件（8个）

`ls` `cat` `rm` `mkdir` `echo` `whoami` `tree` `demo`

输入 `modules` 查看完整列表。

## 插件 API（wash.*）

| API | 说明 |
|-----|------|
| `wash.get_cwd()` | 获取当前工作目录 |
| `wash.print(text)` | 终端输出 |
| `wash.read_dir(path)` | 列出目录内容（返回 table） |
| `wash.make_dir(path)` | 创建文件夹 |
| `wash.remove_file(path)` | 删除文件 |
| `wash.read_file(path)` | 读取文件 |
| `wash.write_file(path, content)` | 写入文件 |
| `wash.spawn_exe(exe, args)` | 调用外部程序 |

## 编译

需要 MinGW-w64 GCC。在项目根目录执行：

```cmd
set PATH=你的MinGW\bin;%PATH%

:: 编译 Lua 静态库
for %f in (lapi lcode ldebug ldo ldump lfunc lgc llex lmem loadlib lobject lopcodes lparser lstate lstring ltable ltm lundump lvm lzio lauxlib lbaselib ldblib liolib lmathlib loslib ltablib lstrlib linit) do gcc -O2 -c lua-src\%f.c -o lua-src\%f.o -Ilua-src
ar rcs lua-src\liblua.a lua-src\*.o

:: 编译 Wash
gcc -O2 -Wall -finput-charset=UTF-8 -fexec-charset=UTF-8 -c src\shell.c -o src\shell.o -Ilua-src
gcc -O2 -Wall -finput-charset=UTF-8 -fexec-charset=UTF-8 -c src\builtin.c -o src\builtin.o -Ilua-src
gcc -O2 -Wall -finput-charset=UTF-8 -fexec-charset=UTF-8 -c src\lua_kernel.c -o src\lua_kernel.o -Ilua-src
gcc -O2 -Wall -finput-charset=UTF-8 -fexec-charset=UTF-8 -c src\main.c -o src\main.o -Ilua-src
gcc -O2 -o wash.exe src\main.o src\shell.o src\builtin.o src\lua_kernel.o lua-src\liblua.a -lm
```

生成 `wash.exe`，与 `wash-modules/` 放在同一目录即可运行。

## 目录结构

```
wash-project/
├── src/                  C 内核源码
│   ├── main.c            入口、启动参数解析
│   ├── shell.h/.c        会话状态、命令解析、主循环
│   ├── builtin.h/.c      内置命令
│   └── lua_kernel.h/.c   Lua 虚拟机、API 暴露、插件加载、模式切换
├── lua-src/              Lua 5.1.5 源码（静态链接）
├── wash-modules/         插件目录（.wash 纯文本 Lua 脚本）
├── docs/                 文档（插件清单、API清单）
├── LICENSE               MIT 协议
└── README.md
```

## 编写自定义插件

在 `wash-modules/` 下新建 `yourcmd.wash`：

```lua
-- yourcmd.wash
local args = ...  -- 命令行参数表（array）

wash.print("Hello from yourcmd!")
wash.print("Args count: " .. #args)
for i, v in ipairs(args) do
    wash.print("  [" .. i .. "] " .. v)
end
```

保存后直接在 Wash 中输入 `yourcmd` 即可运行，无需重启、无需编译。

## 已实现功能

- [x] C 原生内核，静态链接 Lua 5.1（单 exe，无外部依赖）
- [x] 会话状态管理（CWD）
- [x] 命令行分词解析（支持双引号）
- [x] 内置命令（exit/cd/pwd/clear/help/modules/unrestrict/restrict）
- [x] Lua 插件动态加载，即插即用
- [x] wash.* C API 暴露给 Lua
- [x] 高危接口屏蔽（os.execute/io/loadfile 等）
- [x] 受限/不受限双模式运行时切换
- [x] 完整 UTF-8 中文支持（输入、输出、文件名）

## 待开发

- [ ] 管道 `|`
- [ ] 输入输出重定向 `<` `>`
- [ ] 环境变量管理
- [ ] 历史记录（上下箭头）
- [ ] Tab 自动补全
- [ ] 插件间状态共享
- [ ] 更多内置工具插件

## 免责声明

本项目允许加载第三方插件；不受限模式下插件拥有访问本地文件、执行程序的完整权限。请只加载信任来源的插件，加载未知插件造成的任何损失由使用者自行承担。

## 协议

MIT License
