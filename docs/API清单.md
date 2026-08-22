# Wash 内核 API 清单

> 本文档列出 Wash 内核已暴露给插件的 `wash.*` API，以及规划中待实现的接口。
> 所有 API 均为 C 函数注册到 Lua 虚拟机，插件只能通过 `wash.*` 调用，禁止直接访问系统底层。

## 一、已提供 API（8 个）

### 1. 会话与输出

| API | 签名 | 返回值 | 说明 |
|-----|------|--------|------|
| `wash.get_cwd` | `wash.get_cwd()` | `string` | 获取内核维护的当前工作目录 |
| `wash.print` | `wash.print(text)` | - | 打印一行文本到终端，自动换行 |

### 2. 文件与目录

| API | 签名 | 返回值 | 说明 |
|-----|------|--------|------|
| `wash.read_dir` | `wash.read_dir(path?)` | `table` (array) | 列出目录内容，跳过 `.` 和 `..`；path 省略时用当前目录 |
| `wash.make_dir` | `wash.make_dir(path)` | `boolean` | 创建文件夹，成功返回 true |
| `wash.remove_file` | `wash.remove_file(path)` | `boolean` | 删除文件，成功返回 true |
| `wash.read_file` | `wash.read_file(path)` | `string` 或 `nil, err` | 读取文件全部内容；失败返回 nil 和错误信息 |
| `wash.write_file` | `wash.write_file(path, content)` | `boolean` | 写入文件（覆盖模式），成功返回 true |

### 3. 进程

| API | 签名 | 返回值 | 说明 |
|-----|------|--------|------|
| `wash.spawn_exe` | `wash.spawn_exe(exe, args_table?)` | `boolean` | 调用外部可执行程序，args 为参数数组 |

### API 使用示例

```lua
-- 获取当前目录
local cwd = wash.get_cwd()

-- 列出目录
local entries = wash.read_dir(cwd)
for i, name in ipairs(entries) do
    wash.print(i .. ". " .. name)
end

-- 读写文件
wash.write_file("test.txt", "hello\n")
local content = wash.read_file("test.txt")
wash.print(content)

-- 调用外部程序
wash.spawn_exe("where", {"cmd"})
```

## 二、待提供 API（按优先级排序）

### 高优先级（文件操作补全）

| API | 签名 | 返回值 | 说明 | 阻塞的插件 |
|-----|------|--------|------|-----------|
| `wash.copy_file` | `wash.copy_file(src, dst)` | `boolean` | 复制文件 | `cp` |
| `wash.move_file` | `wash.move_file(src, dst)` | `boolean` | 移动/重命名文件 | `mv` |
| `wash.remove_dir` | `wash.remove_dir(path, recursive?)` | `boolean` | 删除目录，可选递归 | `rmdir`, `rm -r` |
| `wash.file_exists` | `wash.file_exists(path)` | `boolean` | 判断文件/目录是否存在 | 通用 |
| `wash.is_dir` | `wash.is_dir(path)` | `boolean` | 判断是否为目录 | `tree`, `find`, `stat` |
| `wash.is_file` | `wash.is_file(path)` | `boolean` | 判断是否为文件 | `tree`, `find`, `stat` |

### 中优先级（文件信息与环境）

| API | 签名 | 返回值 | 说明 | 阻塞的插件 |
|-----|------|--------|------|-----------|
| `wash.file_size` | `wash.file_size(path)` | `number` | 获取文件大小（字节） | `stat`, `du`, `ls -l` |
| `wash.file_time` | `wash.file_time(path)` | `table` | 获取文件创建/修改/访问时间 | `stat`, `ls -l` |
| `wash.file_attr` | `wash.file_attr(path)` | `table` | 获取文件属性（只读/隐藏/系统等） | `stat`, `ls -l` |
| `wash.get_env` | `wash.get_env(name)` | `string` | 获取环境变量 | `env`, 通用 |
| `wash.set_env` | `wash.set_env(name, value)` | `boolean` | 设置环境变量 | `env`, `set` |
| `wash.list_env` | `wash.list_env()` | `table` | 列出所有环境变量 | `env` |
| `wash.temp_dir` | `wash.temp_dir()` | `string` | 获取系统临时目录路径 | 通用 |
| `wash.temp_file` | `wash.temp_file(prefix?)` | `string` | 创建临时文件并返回路径 | 通用 |

### 低优先级（高级功能）

| API | 签名 | 返回值 | 说明 | 阻塞的插件 |
|-----|------|--------|------|-----------|
| `wash.disk_free` | `wash.disk_free(path?)` | `table` | 获取磁盘总空间/可用空间 | `df` |
| `wash.list_processes` | `wash.list_processes()` | `table` | 列出系统进程 | `ps` |
| `wash.kill_process` | `wash.kill_process(pid, signal?)` | `boolean` | 终止进程 | `kill` |
| `wash.sleep` | `wash.sleep(ms)` | - | 休眠指定毫秒数 | 通用 |
| `wash.clipboard_get` | `wash.clipboard_get()` | `string` | 读取剪贴板 | 通用 |
| `wash.clipboard_set` | `wash.clipboard_set(text)` | `boolean` | 写入剪贴板 | 通用 |
| `wash.http_get` | `wash.http_get(url)` | `string, err` | HTTP GET 请求 | 网络插件 |
| `wash.http_post` | `wash.http_post(url, data)` | `string, err` | HTTP POST 请求 | 网络插件 |
| `wash.glob` | `wash.glob(pattern, path?)` | `table` | 通配符匹配文件 | `find`, 通用 |
| `wash.regex_match` | `wash.regex_match(text, pattern)` | `table, nil` | 正则匹配 | `grep`, 通用 |
| `wash.json_encode` | `wash.json_encode(table)` | `string` | JSON 编码 | 通用 |
| `wash.json_decode` | `wash.json_decode(string)` | `table` | JSON 解码 | 通用 |
| `wash.base64_encode` | `wash.base64_encode(data)` | `string` | Base64 编码 | `base64` |
| `wash.base64_decode` | `wash.base64_decode(data)` | `string` | Base64 解码 | `base64` |
| `wash.hash_md5` | `wash.hash_md5(data)` | `string` | MD5 哈希 | `md5sum` |
| `wash.hash_sha256` | `wash.hash_sha256(data)` | `string` | SHA256 哈希 | `sha256sum` |

### 管道相关（待管道机制实现后）

| API | 签名 | 返回值 | 说明 |
|-----|------|--------|------|
| `wash.stdin_read` | `wash.stdin_read()` | `string` | 读取标准输入（管道上游输出） |
| `wash.stdout_write` | `wash.stdout_write(text)` | - | 写入标准输出（管道下游输入） |
| `wash.stderr_write` | `wash.stderr_write(text)` | - | 写入标准错误 |
| `wash.exit` | `wash.exit(code)` | - | 插件以指定退出码结束 |

### 插件间交互（远期规划）

| API | 签名 | 返回值 | 说明 |
|-----|------|--------|------|
| `wash.load_module` | `wash.load_module(name)` | `function` | 加载其他插件并返回其函数 |
| `wash.call_module` | `wash.call_module(name, args)` | `...` | 调用其他插件命令 |
| `wash.register_command` | `wash.register_command(name, func)` | - | 动态注册子命令 |
| `wash.add_hook` | `wash.add_hook(event, func)` | - | 注册事件钩子（如命令执行前后） |
| `wash.current_module` | `wash.current_module()` | `string` | 获取当前插件名称 |

## 三、已屏蔽的原生接口与危险性评级

为保证架构分层，以下 Lua 原生接口在内核初始化时已被移除，插件无法调用。
按危险性分为 S/A/B/C 四级：

| 危险等级 | 接口 | 屏蔽原因 | 替代方案 | 潜在危害 |
|----------|------|----------|----------|----------|
| **S（致命）** | `os.execute` | 任意命令执行，完全绕过 API 收口 | `wash.spawn_exe` | 可执行任意系统命令，格式化硬盘、下载木马 |
| **S（致命）** | `loadfile` | 加载任意 Lua 脚本，破坏插件隔离 | 内核统一管理 | 可加载并执行任意外部代码，完全逃逸 |
| **S（致命）** | `dofile` | 执行任意 Lua 脚本 | 同上 | 同上 |
| **S（致命）** | `require` | 加载外部 Lua 模块/C 模块 | 同上 | 可加载恶意 .dll 模块，执行原生代码 |
| **A（高危）** | `io.*` | 直接文件 IO，完全绕过 API 收口 | `wash.read_file` / `wash.write_file` | 可读写任意文件，包括系统文件、配置文件 |
| **A（高危）** | `os.remove` | 直接删除文件，绕过 API 收口 | `wash.remove_file` | 可删除任意文件，无需经过内核审计 |
| **A（高危）** | `os.rename` | 直接重命名/移动，绕过 API 收口 | `wash.move_file`（待实现） | 可移动系统文件，破坏系统 |
| **A（高危）** | `os.exit` | 直接退出进程，绕过内核清理 | 内置 `exit` 命令 | 可异常终止 shell，跳过清理逻辑 |
| **B（中危）** | `os.tmpname` | 直接创建临时文件 | `wash.temp_file`（待实现） | 可在临时目录留下恶意文件 |
| **C（低危）** | `debug.*` | 调试库可访问虚拟机内部状态 | 无 | 理论上可通过 debug 逃逸沙箱，实际利用难度高 |

> **说明**：S 级接口可直接导致系统完全沦陷；A 级可破坏文件系统；B 级有间接风险；C 级理论风险，实际难以利用。

## 四、不受限模式（Unrestricted Mode）

### 设计说明

默认运行在**受限模式（Restricted Mode）**，上述接口全部屏蔽。
对于信任的插件或开发调试场景，可切换到**不受限模式（Unrestricted Mode）**，恢复全部原生接口。

### 切换方式

| 方式 | 说明 |
|------|------|
| 内置命令 `unrestrict` | 在 shell 中输入 `unrestrict`，切换到不受限模式 |
| 内置命令 `restrict` | 切回受限模式 |
| 启动参数 `--unrestricted` | 启动时直接进入不受限模式 |
| 启动参数 `-u` | 同上，短选项 |

### 模式差异

| 能力 | 受限模式（默认） | 不受限模式 |
|------|-----------------|-----------|
| `wash.*` API | 可用 | 可用 |
| `os.execute` | 屏蔽 | 可用 |
| `io.*` | 屏蔽 | 可用 |
| `os.remove` / `os.rename` | 屏蔽 | 可用 |
| `loadfile` / `dofile` / `require` | 屏蔽 | 可用 |
| `os.exit` | 屏蔽 | 可用 |
| `debug.*` | 屏蔽 | 可用 |
| 插件加载方式 | 内核统一加载 | 内核统一加载，插件也可自行加载 |

### 安全提示

> 不受限模式下，插件拥有与普通 Lua 脚本完全相同的权限，可执行任意系统操作。
> 仅在以下场景使用：
> 1. 开发调试插件时
> 2. 运行完全信任的自有插件时
> 3. 需要使用 `wash.*` API 尚未覆盖的功能时
>
> 运行不受信任的插件时，请务必保持受限模式。

## 五、API 设计原则

1. **收口原则**：所有系统操作必须经过 `wash.*` API，内核统一管理权限和错误处理
2. **路径自动补全**：相对路径由内核自动拼接当前 cwd，插件无需处理
3. **错误返回**：失败时返回 `nil, err` 或 `false`，不主动抛出异常
4. **轻量原则**：API 保持简单，复杂逻辑由插件实现
5. **向后兼容**：新增 API 不影响已有插件，已有 API 签名保持稳定

## 六、统计

- 已提供 API：**8 个**
- 高优先级待实现：**6 个**
- 中优先级待实现：**8 个**
- 低优先级待实现：**15 个**
- 管道相关：**4 个**
- 插件间交互：**5 个**
- 已屏蔽原生接口：**10 个**（S级 4 个 / A级 4 个 / B级 1 个 / C级 1 个）
- 运行模式：**2 种**（受限模式 / 不受限模式）
