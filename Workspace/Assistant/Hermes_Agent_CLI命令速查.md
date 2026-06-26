# Hermes Agent CLI 命令速查

> 基于官方文档整理，来源：https://hermes-agent.nousresearch.com/docs/reference/cli-commands

---

## 一、全局入口

```bash
hermes [全局选项] [子命令]
```

### 全局选项

| 选项 | 简写 | 说明 |
|------|------|------|
| `--version` | `-V` | 显示版本号 |
| `--profile` | `-p` | 指定使用的配置文件 |
| `--resume <id>` | `-r` | 恢复指定 ID 或标题的会话 |
| `--continue [name]` | `-c` | 恢复最近会话，可指定名称 |
| `--worktree` | `-w` | 在隔离 git worktree 中启动 |
| `--yolo` | | 跳过危险命令确认提示 |
| `--ignore-user-config` | | 忽略 `~/.hermes/config.yaml`，使用默认配置 |
| `--ignore-rules` | | 跳过 AGENTS.md、SOUL.md、.cursorrules 等自动注入 |
| `--tui` | | 启动 TUI 界面（等同于 `HERMES_TUI=1`）|
| `--cli` | | 强制使用经典 REPL 界面 |
| `--dev` | | 以 `--tui` 运行 TypeScript 源码（通过 tsx）|

---

## 二、顶级子命令

| 命令 | 用途 |
|------|------|
| `hermes chat` | 与 Agent 交互聊天 |
| `hermes model` | 选择默认模型和 provider |
| `hermes moa` | 配置 Mixture of Agents 预设 |
| `hermes fallback` | 管理备用 provider |
| `hermes gateway` | 运行消息网关服务 |
| `hermes proxy` | 本地 OpenAI 兼容代理（带 OAuth 凭证）|
| `hermes setup` | 交互式安装向导 |
| `hermes auth` | 管理凭证（OAuth 流程）|
| `hermes send` | 单次发消息（不启动 Agent 循环）|
| `hermes secrets` | 管理外部密钥（Bitwarden）|
| `hermes status` | 显示 Agent、认证和平台状态 |
| `hermes cron` | Cron 定时任务管理 |
| `hermes doctor` | 诊断配置和依赖问题 |
| `hermes backup` | 备份 Hermes 配置到 zip |
| `hermes config` | 显示 / 编辑 / 查询配置 |
| `hermes skills` | 浏览、安装、发布、审计 skills |
| `hermes sessions` | 浏览、导出、清理、重命名会话 |
| `hermes logs` | 查看、追踪、过滤日志 |
| `hermes update` | 拉取最新代码并更新 |
| `hermes uninstall` | 卸载 Hermes |

---

## 三、Slash 命令（会话内）

> 在 CLI 中输入 `/-` 可触发自动补全。内置命令**不区分大小写**。

### 3.1 会话管理

| 命令 | 别名 | 说明 |
|------|------|------|
| `/new [name]` | `/reset` | 开启新会话，可选名称作为标题 |
| `/clear` | — | 清屏并开启新会话 |
| `/history` | — | 显示对话历史 |
| `/save` | — | 保存当前会话 |
| `/retry` | — | 重新发送上一条消息 |
| `/undo` | — | 删除最后一轮对话 |
| `/title <name>` | — | 设置会话标题 |
| `/compress [here [N] \| focus <topic>]` | — | 压缩上下文，保留最近 N 轮或聚焦主题 |
| `/rollback [number]` | — | 列出 / 恢复文件系统检查点 |
| `/snapshot [create\|restore\|prune]` | `/snap` | 管理状态快照 |
| `/stop` | — | 终止所有后台进程 |
| `/queue <prompt>` | `/q` | 排队下一条消息（不打断当前工作）|
| `/steer <prompt>` | — | 在下一轮工具调用后注入提示（不打断）|
| `/goal <text>` | — | 设置持续目标，自动持续直到完成 |
| `/resume [name]` | — | 恢复指定会话 |
| `/sessions` | `/switch` | 浏览 / 恢复会话 |
| `/branch [name]` | `/fork` | 分叉当前会话 |
| `/handoff <platform>` | — | 将会话转接到消息平台（CLI 专用）|

> 跳过确认：命令后加 `now`、`--yes` 或 `-y`（如 `/reset now`）

### 3.2 配置相关

| 命令 | 说明 |
|------|------|
| `/config` | 显示当前配置 |
| `/model [model-name]` | 显示 / 切换模型，支持 `provider:model` 格式 |
| `/model --global` | 持久化模型配置到 config.yaml |
| `/verbose` | 切换工具进度显示：off → new → all → verbose |
| `/fast [normal\|fast\|status]` | 切换 OpenAI/Anthropic 快速模式 |
| `/reasoning [level\|show\|hide]` | 管理推理强度 |
| `/voice [on\|off\|tts\|status]` | 切换语音模式 |
| `/yolo` | 跳过危险命令确认 |
| `/busy [queue\|steer\|interrupt\|status]` | 控制工作时的 Enter 键行为 |
| `/indicator [kaomoji\|emoji\|unicode\|ascii]` | 选择忙碌指示器样式 |
| `/personality` | 设置预定义人格 |

### 3.3 工具与 Skills

| 命令 | 说明 |
|------|------|
| `/tools [list\|disable\|enable] [name...]` | 管理当前会话的工具 |
| `/toolsets` | 列出可用工具集 |
| `/skills` | 搜索 / 安装 / 管理 skills |
| `/memory [pending\|approve\|reject\|approval]` | 审核 / 控制记忆写入审批门 |
| `/bundles` | 列出已配置的 skill 捆绑包 |
| `/learn <description>` | 从经验中提炼可复用 skill |
| `/cron` | 管理定时任务 |
| `/curator [status\|run\|pin\|archive]` | 后台 skill 维护 |
| `/kanban <action>` | 驱动协作看板 |

### 3.4 文件与代码

| 命令 | 说明 |
|------|------|
| `/browse <url>` | 在 CLI 中浏览网页 |
| `/read <path>` | 读取文件 |
| `/edit <path>` | 编辑文件（触发编辑器）|
| `/lsp [on\|off\|status]` | 切换 LSP 语义诊断 |
| `/websearch <query>` | 执行 Web 搜索 |
| `/webextract <urls>` | 提取网页正文内容 |

---

## 四、hermes chat 用法

```bash
hermes chat [选项]
```

| 选项 | 简写 | 说明 |
|------|------|------|
| `--query "..."` | `-q` | 单次查询（非交互模式）|
| `--model` | `-m` | 覆盖此次运行的模型 |
| `--toolsets` | `-t` | 启用指定工具集（逗号分隔）|
| `--provider` | | 指定 provider（如 `openrouter`、`anthropic`）|
| `--skills` | `-s` | 预加载 skills |
| `--verbose` | `-v` | 详细输出 |
| `--quiet` | `-Q` | 隐藏横幅 / 加载动画 / 工具预览 |
| `--image` | | 附加本地图片到单次查询 |

---

## 五、hermes config 用法

```bash
hermes config [动作]
```

| 动作 | 说明 |
|------|------|
| `hermes config show` | 显示当前配置 |
| `hermes config set <key> <value>` | 设置配置项 |
| `hermes config get <key>` | 读取配置项 |
| `hermes config edit` | 打开编辑器编辑配置 |
| `hermes config migrate` | 迁移配置到新版本 |

---

## 六、hermes skills 用法

```bash
hermes skills [动作]
```

| 动作 | 说明 |
|------|------|
| `list` | 列出已安装的 skills |
| `install <name>` | 从注册表安装 skill |
| `publish <path>` | 发布 skill 到注册表 |
| `audit` | 检查 skills 问题 |
| `curator` | 后台维护任务 |

---

## 七、hermes sessions 用法

```bash
hermes sessions [动作]
```

| 动作 | 说明 |
|------|------|
| `list` | 列出所有会话 |
| `export <id>` | 导出会话 |
| `prune <id>` | 删除指定会话 |
| `rename <id> <name>` | 重命名会话 |

---

## 八、常用快捷场景

| 场景 | 命令 |
|------|------|
| 启动交互式聊天 | `hermes chat` |
| 单次查询 | `hermes chat -q "你好"` |
| 切换模型 | `/model` 或 `/model anthropic:claude-sonnet-4` |
| 新会话 | `/new` |
| 恢复之前会话 | `hermes chat --resume <session-id>` |
| 查看配置 | `/config` |
| 备份配置 | `hermes backup` |
| 诊断问题 | `hermes doctor` |
| 安装 skill | `/skills install <skill-name>` |
| 管理定时任务 | `/cron` |
| 更新 Hermes | `hermes update` |