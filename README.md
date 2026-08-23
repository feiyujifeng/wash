# WASH (Wash Aren't Shell)

> C 内核 + 内嵌 Lua 5.1 虚拟机的可扩展 Windows Shell

## 项目定名

**WASH = Wash Aren't Shell**

采用开源经典递归自指命名（对标 GNU 命名体系）。Wash 并非传统意义上的 CMD、Bash、PowerShell 类命令行解释器，而是一款融合宏内核与微内核优势、基于 C 内核与 Lua 虚拟机的**模块化可扩展终端架构实验框架**。

命名采用复数 `Aren't` 而非单数 `Isn't`，因为 Wash 由**常驻核心内核 + 海量可扩展模块化插件**组成复数生态体系，语义贴合架构。

## 架构

- **C 内核** (`src/`)：维护会话状态（CWD）、命令解析、Lua 虚拟机管理、内置命令
- **Lua 插件** (`wash-modules/*.wash`)：纯文本 Lua 脚本，放入目录即可加载，无需编译、无需重启
- **默认不受限**：全部 Lua 原生接口（`os.execute`、`io.*`、`loadfile` 等）直接可用
- **wash.* API**：17 个标准化 C 接口，与 Lua 原生接口并存，插件可自由选择
- **可选受限模式**：`-r` 启动或 `restrict` 命令可屏蔽原生接口，仅允许 `wash.*` API

## 快速开始

从 [Releases](https://github.com/feiyujifeng/wash/releases) 下载 `WASH-Portable.exe`，双击解压后运行 `wash.bat`。

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
| `unrestrict` | 切换为不受限模式（默认） |
| `restrict` | 切回受限模式 |

## 内置插件（18个）

文件操作：`ls` `cat` `rm` `mkdir` `rmdir` `cp` `mv` `touch` `tree`
文本处理：`grep` `wc` `head` `tail` `echo`
系统信息：`whoami` `date` `find`
演示：`demo`

输入 `modules` 查看完整列表。

## wash.* API（17个）

| 分类 | API |
|------|-----|
| 会话与终端 | `get_cwd` `print` `clear` `sleep` `get_key` |
| 文件与目录 | `read_dir` `make_dir` `remove_dir` `remove_file` `copy_file` `move_file` `read_file` `write_file` `file_exists` `is_dir` `is_file` |
| 进程 | `spawn_exe` |

其中 6 个与 Lua 原生接口重复（`print`/`read_file`/`write_file`/`remove_file`/`move_file`/`spawn_exe`），保留作为标准化接口。

详见 [docs/API清单.md](docs/API清单.md)。

## 编译

需要 MinGW-w64 GCC。确保 `gcc` 在 PATH 中，然后运行：

```cmd
build.bat
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
├── installer/            安装包制作配置
├── build.bat             一键编译脚本
├── THIRD_PARTY_NOTICES.md  第三方软件声明
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

-- 不受限模式下可直接使用 Lua 原生接口
os.execute("echo hello")
local f = io.open("test.txt", "w")
f:write("world\n")
f:close()

-- 也可以使用 wash.* 标准化 API
wash.write_file("test2.txt", "via wash API\n")
```

保存后直接在 Wash 中输入 `yourcmd` 即可运行，无需重启、无需编译。

## 已实现功能

- [x] C 原生内核，静态链接 Lua 5.1（单 exe，无外部依赖）
- [x] 会话状态管理（CWD）
- [x] 命令行分词解析（支持双引号）
- [x] 内置命令（exit/cd/pwd/clear/help/modules/unrestrict/restrict）
- [x] Lua 插件动态加载，即插即用
- [x] wash.* C API 暴露给 Lua（17个）
- [x] 默认不受限，全部 Lua 原生接口可用
- [x] 可选受限模式（屏蔽 os.execute/io/loadfile 等）
- [x] 完整 UTF-8 中文支持（输入、输出、文件名）
- [x] 实时按键读取、休眠、清屏

## 待开发

- [ ] 管道 `|`
- [ ] 输入输出重定向 `<` `>`
- [ ] 环境变量管理
- [ ] 历史记录（上下箭头）
- [ ] Tab 自动补全
- [ ] 插件间状态共享
- [ ] 更多内置工具插件

## 免责声明

本项目默认不受限模式，插件拥有与普通 Lua 脚本完全相同的权限，可执行任意系统操作。受限模式仅为架构分层约束，非安全沙箱。请只加载信任来源的插件，造成的任何损失由使用者自行承担。

## 协议

本项目基于 **MIT License** 开源。

包含第三方组件 Lua 5.1.5（MIT License），详见 [THIRD_PARTY_NOTICES.md](THIRD_PARTY_NOTICES.md)。
