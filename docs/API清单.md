# Wash 内核 API 清单

> 本文档列出 Wash 内核暴露给插件的 `wash.*` API。
> v0.2.2 起默认**不受限模式**，全部 Lua 原生接口（`os.execute`、`io.*`、`loadfile`、`require` 等）均可直接使用。
> `wash.*` API 作为标准化接口保留，与 Lua 原生接口并存。

## 一、已提供 API（17 个）

### 1. 会话与终端

| API | 签名 | 返回值 | 说明 | 与 Lua 重复 |
|-----|------|--------|------|------------|
| `wash.get_cwd` | `wash.get_cwd()` | `string` | 获取内核维护的当前工作目录 | 否 |
| `wash.print` | `wash.print(text)` | - | 打印一行文本到终端，自动换行 | 是（`print`） |
| `wash.clear` | `wash.clear()` | - | 清屏 | 否 |
| `wash.sleep` | `wash.sleep(ms)` | - | 休眠指定毫秒数 | 否 |
| `wash.get_key` | `wash.get_key()` | `number` | 实时读取按键（不回显），功能键返回 `0x100+` | 否 |

### 2. 文件与目录

| API | 签名 | 返回值 | 说明 | 与 Lua 重复 |
|-----|------|--------|------|------------|
| `wash.read_dir` | `wash.read_dir(path?)` | `table` | 列出目录内容，跳过 `.` 和 `..` | 否 |
| `wash.make_dir` | `wash.make_dir(path)` | `boolean` | 创建文件夹 | 否 |
| `wash.remove_dir` | `wash.remove_dir(path, recursive?)` | `boolean` | 删除目录，可选递归 | 否 |
| `wash.remove_file` | `wash.remove_file(path)` | `boolean` | 删除文件 | 是（`os.remove`） |
| `wash.copy_file` | `wash.copy_file(src, dst)` | `boolean` | 复制文件 | 否 |
| `wash.move_file` | `wash.move_file(src, dst)` | `boolean` | 移动/重命名文件 | 是（`os.rename`） |
| `wash.read_file` | `wash.read_file(path)` | `string` 或 `nil, err` | 读取文件全部内容 | 是（`io.*`） |
| `wash.write_file` | `wash.write_file(path, content)` | `boolean` | 写入文件（覆盖模式） | 是（`io.*`） |
| `wash.file_exists` | `wash.file_exists(path)` | `boolean` | 判断文件/目录是否存在 | 否 |
| `wash.is_dir` | `wash.is_dir(path)` | `boolean` | 判断是否为目录 | 否 |
| `wash.is_file` | `wash.is_file(path)` | `boolean` | 判断是否为文件 | 否 |

### 3. 进程

| API | 签名 | 返回值 | 说明 | 与 Lua 重复 |
|-----|------|--------|------|------------|
| `wash.spawn_exe` | `wash.spawn_exe(exe, args_table?)` | `boolean` | 调用外部可执行程序 | 是（`os.execute`） |

### API 使用示例

```lua
-- 获取当前目录
local cwd = wash.get_cwd()

-- 列出目录
local entries = wash.read_dir(cwd)
for i, name in ipairs(entries) do
    wash.print(i .. ". " .. name)
end

-- 读写文件（wash.* 方式）
wash.write_file("test.txt", "hello\n")
local content = wash.read_file("test.txt")
wash.print(content)

-- 也可以直接用 Lua 原生 io（不受限模式下）
local f = io.open("test.txt", "r")
wash.print(f:read("*a"))
f:close()

-- 调用外部程序
wash.spawn_exe("where", {"cmd"})
-- 或直接用 os.execute
os.execute("dir")

-- 实时按键
wash.print("按任意键继续...")
local key = wash.get_key()
wash.print("按键码: " .. key)

-- 休眠
wash.sleep(1000)  -- 1秒

-- 清屏
wash.clear()
```

## 二、待提供 API（按优先级排序）

### 中优先级（文件信息与环境）

| API | 签名 | 返回值 | 说明 |
|-----|------|--------|------|
| `wash.file_size` | `wash.file_size(path)` | `number` | 获取文件大小（字节） |
| `wash.file_time` | `wash.file_time(path)` | `table` | 获取文件创建/修改/访问时间 |
| `wash.file_attr` | `wash.file_attr(path)` | `table` | 获取文件属性（只读/隐藏/系统等） |
| `wash.get_env` | `wash.get_env(name)` | `string` | 获取环境变量 |
| `wash.set_env` | `wash.set_env(name, value)` | `boolean` | 设置环境变量 |
| `wash.list_env` | `wash.list_env()` | `table` | 列出所有环境变量 |
| `wash.temp_dir` | `wash.temp_dir()` | `string` | 获取系统临时目录路径 |
| `wash.temp_file` | `wash.temp_file(prefix?)` | `string` | 创建临时文件并返回路径 |

### 低优先级（高级功能）

| API | 签名 | 返回值 | 说明 |
|-----|------|--------|------|
| `wash.disk_free` | `wash.disk_free(path?)` | `table` | 获取磁盘总空间/可用空间 |
| `wash.list_processes` | `wash.list_processes()` | `table` | 列出系统进程 |
| `wash.kill_process` | `wash.kill_process(pid, signal?)` | `boolean` | 终止进程 |
| `wash.clipboard_get` | `wash.clipboard_get()` | `string` | 读取剪贴板 |
| `wash.clipboard_set` | `wash.clipboard_set(text)` | `boolean` | 写入剪贴板 |
| `wash.http_get` | `wash.http_get(url)` | `string, err` | HTTP GET 请求 |
| `wash.http_post` | `wash.http_post(url, data)` | `string, err` | HTTP POST 请求 |
| `wash.glob` | `wash.glob(pattern, path?)` | `table` | 通配符匹配文件 |
| `wash.regex_match` | `wash.regex_match(text, pattern)` | `table, nil` | 正则匹配 |
| `wash.json_encode` | `wash.json_encode(table)` | `string` | JSON 编码 |
| `wash.json_decode` | `wash.json_decode(string)` | `table` | JSON 解码 |
| `wash.base64_encode` | `wash.base64_encode(data)` | `string` | Base64 编码 |
| `wash.base64_decode` | `wash.base64_decode(data)` | `string` | Base64 解码 |
| `wash.hash_md5` | `wash.hash_md5(data)` | `string` | MD5 哈希 |
| `wash.hash_sha256` | `wash.hash_sha256(data)` | `string` | SHA256 哈希 |

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
| `wash.add_hook` | `wash.add_hook(event, func)` | - | 注册事件钩子 |
| `wash.current_module` | `wash.current_module()` | `string` | 获取当前插件名称 |

## 三、运行模式

### 默认模式：不受限（Unrestricted）

v0.2.2 起，WASH 默认运行在**不受限模式**，全部 Lua 原生接口可用：

| 接口 | 状态 | 说明 |
|------|------|------|
| `os.execute` | 可用 | 执行系统命令 |
| `io.*` | 可用 | 文件读写 |
| `os.remove` / `os.rename` | 可用 | 删除/重命名文件 |
| `loadfile` / `dofile` / `require` | 可用 | 加载外部脚本/模块 |
| `os.exit` | 可用 | 退出进程 |
| `debug.*` | 可用 | 调试库 |
| `wash.*` API | 可用 | 标准化接口（与原生并存） |

### 受限模式（Restricted）

如需屏蔽原生接口，可通过以下方式进入受限模式：

| 方式 | 说明 |
|------|------|
| 启动参数 `-r` / `--restricted` | 启动时进入受限模式 |
| 内置命令 `restrict` | 运行时切换为受限模式 |
| 内置命令 `unrestrict` | 切回不受限模式（默认） |

受限模式下以下接口被屏蔽：`os.execute`、`io.*`、`os.remove`、`os.rename`、`os.exit`、`os.tmpname`、`loadfile`、`dofile`、`require`、`debug.*`。插件只能通过 `wash.*` API 操作。

### 危险性评级（受限模式下屏蔽的接口）

| 危险等级 | 接口 | 替代方案 |
|----------|------|----------|
| **S（致命）** | `os.execute` | `wash.spawn_exe` |
| **S（致命）** | `loadfile` / `dofile` / `require` | 内核统一管理插件加载 |
| **A（高危）** | `io.*` | `wash.read_file` / `wash.write_file` |
| **A（高危）** | `os.remove` / `os.rename` | `wash.remove_file` / `wash.move_file` |
| **A（高危）** | `os.exit` | 内置 `exit` 命令 |
| **B（中危）** | `os.tmpname` | `wash.temp_file`（待实现） |
| **C（低危）** | `debug.*` | 无 |

## 四、API 设计原则

1. **并存原则**：`wash.*` API 与 Lua 原生接口并存，插件可自由选择
2. **路径自动补全**：相对路径由内核自动拼接当前 cwd，插件无需处理
3. **错误返回**：失败时返回 `nil, err` 或 `false`，不主动抛出异常
4. **轻量原则**：API 保持简单，复杂逻辑由插件实现
5. **向后兼容**：新增 API 不影响已有插件，已有 API 签名保持稳定

## 五、统计

- 已提供 API：**17 个**
- 与 Lua 原生重复：**6 个**（print / read_file / write_file / remove_file / move_file / spawn_exe）
- 中优先级待实现：**8 个**
- 低优先级待实现：**15 个**
- 管道相关：**4 个**
- 插件间交互：**5 个**
- 运行模式：**2 种**（不受限默认 / 受限可选）
