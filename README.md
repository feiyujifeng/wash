# WASH (Wash Aren't Shell)

> C 内核 + 内嵌 Lua 5.1 虚拟机的可扩展 Windows Shell

## 项目定名

**WASH = Wash Aren't Shell**

采用开源经典递归自指命名（对标 GNU 命名体系）。Wash 并非传统意义上的 CMD、Bash、PowerShell 类命令行解释器，而是一款融合宏内核与微内核优势、基于 C 内核与 Lua 虚拟机的**模块化可扩展终端架构实验框架**。

命名采用复数 `Aren't` 而非单数 `Isn't`，因为 Wash 由**常驻核心内核 + 海量可扩展模块化插件**组成复数生态体系，语义贴合架构。

## 架构

- **C 内核** (`src/`)：维护会话状态（CWD、环境变量）、命令解析、Lua 虚拟机管理、内置命令
- **Lua 插件** (`wash-modules/*.wash`)：纯文本 Lua 脚本，放入目录即可加载，无需编译
- **API 收口**：插件只能调用 `wash.*` 系列标准化 C 接口，原生 `os.execute`、`io` 库等已被屏蔽

## 内置命令（不可插件化）

| 命令 | 说明 |
|------|------|
| `exit` | 退出 Wash |
| `cd` | 切换工作目录 |
| `pwd` | 显示当前目录 |
| `clear` | 清屏 |
| `help` | 显示帮助 |
| `modules` | 列出可用插件 |

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

## 编译与运行

### 环境要求
- MinGW-w64 GCC（项目已内置在 `mingw/` 目录）
- Windows 10/11

### 编译
```cmd
build.bat
```
生成 `wash.exe`。

### 运行
```cmd
wash.bat
```
或直接运行 `wash.exe`。

## 目录结构

```
wash-project/
├── wash.exe              # 编译后的可执行文件
├── wash.bat              # 一键启动脚本
├── build.bat             # 编译脚本
├── README.md
├── src/                  # C 内核源码
│   ├── main.c            # 入口
│   ├── shell.h/.c        # 会话状态、命令解析、主循环
│   ├── builtin.h/.c      # 内置命令
│   └── lua_kernel.h/.c   # Lua 虚拟机、API 暴露、插件加载
├── lua-src/              # Lua 5.1.5 源码（静态链接）
├── mingw/                # 内置 MinGW-w64 编译器
└── wash-modules/         # 插件目录
    ├── ls.wash
    ├── mkdir.wash
    ├── cat.wash
    ├── rm.wash
    ├── echo.wash
    └── whoami.wash
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
- [x] 内置命令（exit/cd/pwd/clear/help/modules）
- [x] Lua 插件动态加载
- [x] wash.* C API 暴露给 Lua
- [x] 高危接口屏蔽（os.execute/io/loadfile 等）
- [x] 插件即插即用，无需重启

## 待开发

- [ ] 管道 `|`
- [ ] 输入输出重定向 `<` `>`
- [ ] 环境变量管理
- [ ] 历史记录（上下箭头）
- [ ] Tab 自动补全
- [ ] 插件间状态共享
- [ ] 更多内置工具插件

## 免责声明

本项目允许加载第三方插件；插件拥有访问本地文件、执行程序的权限。请只加载信任来源的插件，加载未知插件造成的数据损坏由使用者自行承担。
