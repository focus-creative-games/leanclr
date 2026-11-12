# LeanCLR 架构与分支维护

## 最终分支结构

| 长期分支 | 来源 | 定位 |
| --- | --- | --- |
| `mono-minimal` | 原 `core` | Mono BCL 的精简、可移植目标；主 clone 默认分支 |
| `mono-full` | 原 `mono` | Mono BCL 的扩展平台/线程能力开发目标 |
| `coreclr-minimal` | 原 `coreclr-net10` | .NET 10 CoreCLR BCL 的受控精简范围 |
| `coreclr-full` | 尚不存在 | 有实际开发需求和实现后再建立，不创建占位分支 |

`full` 不表示全部 BCL API 或多线程已经实现。两个 Mono 分支在改名前均位于 `771b1d0a`；此次只统一命名和维护方式，不用旧实现覆盖重构代码。CoreCLR 的后续迁移基于 `32990b5a` 保留全部已交付工作。

GitHub 默认分支为 `mono-minimal`。本地使用一个主 clone checkout mono-minimal，其余两个运行时分支使用 linked worktree。目录名不参与能力选择。旧引擎、第三方和实验分支退出活跃分支结构；独有历史保存于本地离线 bundle 和 `refs/archive/branch-layout-20261112/...`，不作为 tag 推送，避免把同一 clone 中的私有开发历史发布到公开仓库。整理只修改官方 LeanCLR 远端，不修改 private 或第三方仓库。

## 架构与宿主边界

```text
LeanCLR
├── Shared Runtime：类型系统、元数据、解释器、LeanAOT、GC、分配与公共 API
├── BCL Adapter
│   ├── MONO
│   └── CORECLR_NET10（在 coreclr-minimal 中开发）
├── Capability Scope：minimal / full（开发定位，不是自动启用所有 API）
├── Host Provider：PORTABLE / NATIVE / CUSTOM
└── Standalone：leanrun、简单样例、公共宿主契约测试

Independent host repositories
├── Unity / 团结引擎 SDK 插件
├── Godot GDExtension 与对象/Variant/资源绑定
└── Unreal 插件、模块、对象绑定与打包
```

不建立 standalone、unity、godot 或 UE 运行时分支，也不建立上述维度的笛卡尔积分支。主仓库保留 SDK 无关的公共 ABI、生命周期、线程/重入和错误契约及其测试；具体引擎 SDK 集成位于独立仓库。引擎适配器固定支持的 LeanCLR release/ABI 与引擎版本，不能包含运行时私有 BCL 布局或复制运行时核心。

`src/runtime` 的核心、BCL、host 三层通过 CMake 对象目标独立编译；`cmake/verify_layers.cmake` 校验依赖。`src/tools/leanrun` 保留为最小 standalone 和排障入口，不需要引擎 SDK，也不需要新分支。当前平台服务选择 NATIVE 不会启用多线程；多线程需要线程状态、GC 协作和同步语义的独立实现。当前仅支持小端运行时与 C++17。

## 构建入口与兼容性

底层 host provider 与 BCL 能力级别是不同维度。现有 `mono-minimal`、`mono-native`、`mono-custom` 构建预设和脚本参数继续有效；新增 `mono-full` 预设作为 mono-native 的显式别名。两个名称映射当前相同的原生服务范围，不能据此声称线程或完整平台 API 已交付。

```powershell
scripts\runtime\build.bat Release x64 minimal
scripts\runtime\build.bat Release x64 native
cmake --preset mono-full -S src/runtime
```

Linux/macOS 对应 `./scripts/runtime/build.sh Release minimal` 或 `native`。既有脚本默认仍为 Native。CoreCLR 的 `CORECLR_NET10` 与工具 profile `coreclr-net10` 保留版本语义，不随 Git 分支改名。

## 维护与验收

公共核心修复应在所有受影响分支同步，使用 merge/cherry-pick 保留审计记录，并各自运行 BCL 回归。minimal/full 通过明确的 API 清单和能力实现演进，不能通过分支名推断兼容率。运行时 ABI/GC/AOT 测试与引擎 SDK 加载、生命周期和打包测试分别验收。

2026-11-10 的 Core/Mono 统一记录：起点为 `core=8f5ab4ad`、`mono=8e024dfe`，共同祖先 `b5e4e46d`。Mono 独有的 16 个提交已有等价补丁进入 Core，rebase 后对齐共享核心；下列测试是当时的历史证据，不代表 Full 或所有平台已经通过。

## 本次本地验证（Windows x64 / Release）

| 验证项 | Native | Minimal | Custom |
| --- | --- | --- | --- |
| CTest 回归 | 41/41 | 41/41 | 3/3（公共 API 与宿主能力） |
| 托管普通测试 | 5175/5176 | 5137/5176 | 不运行未注入宿主的完整测试集 |
| 托管 GC 测试 | 166/166 | 166/166 | 同上 |

Native 剩余失败为 `TC_System_Console::TreatControlCAsInput_ToggleWithoutThrow`，
与整合前保留的测试日志一致。Minimal 的 39 个失败涉及 Marshal/布局、控制台、
系统环境、路径与其他平台相关绑定；未在本次迁移中修复，也未将它们全部归因为
预期缺失能力。完整托管测试集不是当前已全绿的发布门槛，仍需后续按能力验证与排查。

另外验证了三种 Windows 脚本的解决方案生成、独立输出目录及宿主缓存选择，
`mono-minimal` 预设静态库构建和 Bash 脚本语法。CI 中的 Linux、ARM64、Wasm
及 sanitizer/fuzz 配置本次未在本机执行，不应计入上述本地通过结果。
