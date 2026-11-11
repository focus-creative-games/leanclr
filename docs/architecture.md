# LeanCLR 架构与分支收敛

## 版本树

Core 和 Mono 不再代表两套运行时实现。当前统一实现以重构后的 `core`
为开发基线，`mono` 对齐该基线，作为迁移期兼容分支。构建结果由 BCL 和宿主选择决定，
不能根据 Git 分支名称判断能力。

```text
LeanCLR
├── Shared Runtime：唯一的类型系统、元数据、执行器、GC、分配与公共 API
├── BCL Adapter
│   ├── Mono：当前实现
│   └── CoreCLR：后续整合，当前仍在独立分支开发
├── Host Provider
│   ├── PORTABLE：C/C++ 标准库实现，不使用 Windows/POSIX SDK
│   ├── NATIVE：原生平台服务
│   └── CUSTOM：应用注入内存与宿主服务
└── Host Integration
    ├── Standalone：leanrun、样例及测试
    └── Unity / Godot / UE：目标为适配包，本次不迁移其代码
```

当前交付组合：

| 配置名 | BCL | 宿主 | 原版本对应 | 线程模型 |
| --- | --- | --- | --- | --- |
| `mono-minimal` | MONO | PORTABLE | Core 的精简部署定位 | SINGLE |
| `mono-native` | MONO | NATIVE | Mono 的原生平台部署定位 | SINGLE |
| `mono-custom` | MONO | CUSTOM | 应用接管宿主能力 | SINGLE |

Minimal 不表示完全禁止文件或时间 API：它仍提供已有标准库服务，应用也可以注入服务。
要求完全接管服务、避免默认文件系统访问的宿主应选择 CUSTOM。
Native 表示选择现有原生实现，不承诺所有平台 API 已完整实现。两种配置均支持
解释执行与 LeanAOT；它们的 CLI/类型系统实现相同。

多线程需要运行时级别的线程状态、GC 协作、同步和测试，不能通过选择 NATIVE 开启。
当前不提供 MULTI 构建选项。未来线程能力应作为独立维度实现，不能再创建一套运行时分支。
目前只支持小端运行时宿主；C++17 是语言要求，不等于所有目标平台都已验证。

## 代码与依赖树

```text
src/runtime/
├── alloc, core, metadata, interp, gc, codegen, vm, ...
│   └── leanclr-core-objects
├── bcl/adapter.h              核心访问 BCL 的契约
├── bcl/mono/                 Mono 布局、运行时语义和兼容桥接
├── icalls, intrinsics, pinvokes, vmutils
│   └── leanclr-bcl-mono-objects（保留原路径，避免破坏生成代码和集成）
├── host/                     原生数据契约、服务表和宿主组合
├── platform/                 不依赖托管对象的原生实现
│   └── leanclr-host-objects
└── CMakeLists.txt            将三个对象层组合为 leanclr 静态库或动态库
```

核心只通过 `bcl/adapter.h` 和宿主契约使用外部能力；宿主实现不包含托管对象、
元数据或 Mono 私有布局。BCL 层负责托管参数转换与平台错误映射。
`platform/rt_event.cpp` 当前是 BCL 的单线程事件模拟，由 BCL 对象层编译，
不能当作原生多线程能力。

`cmake/verify_layers.cmake` 在配置时验证依赖方向；现有对象目标分别编译三个层。
这次保留已重构 core 的物理布局，不再为版本名称复制目录或引入另一套核心。
详细接口见[运行时分层说明](../src/runtime/bcl/README.md)。

## 构建

以下命令可在 `core` 或 `mono` checkout 使用。脚本保留原来的 native 默认行为，
不同宿主使用不同输出目录，避免复用另一组合的缓存或产物：

```powershell
scripts\runtime\build.bat Release x64 minimal
scripts\runtime\build.bat Release x64 native
scripts\runtime\build.bat Release x64 custom
```

```bash
./scripts/runtime/build.sh Release minimal
./scripts/runtime/build.sh Release native
./scripts/runtime/build.sh Release custom
```

也可以使用 CMake 3.21+ 的命名预设（运行时本身仍支持 CMake 3.15）：

```powershell
cd src/runtime
cmake --preset mono-minimal
cmake --build --preset mono-minimal
# 同理使用 mono-native 或 mono-custom；首次配置可追加 -A x64。
```

原有 CMake 集成不必使用预设：

```powershell
cmake -S src/runtime -B out/minimal -DLEANCLR_BCL_ADAPTER=MONO -DLEANCLR_HOST=PORTABLE
cmake --build out/minimal --config Release
```

CUSTOM 构建成功不代表可直接启动；应用必须在第一次服务使用及初始化前配置完整分配器、
UTC 和单调时钟等必需服务。缺少可选平台能力时遵循既有 NotSupported 契约。

## 分支迁移与维护

本次迁移起点：`core=8f5ab4ad`，`mono=8e024dfe`，共同祖先 `b5e4e46d`。
`git cherry core mono` 确认 Mono 独有的 16 个提交均已有等价补丁进入 core。
原 Mono 保留在 `backup/mono-before-core-unification-20261110`，随后执行
`git rebase core`，全部等价提交被跳过，Mono 对齐 core，未使用旧文件覆盖重构结果。

后续公共修改只在 core 基线实现，再让 mono 快进对齐；避免在 mono 上独立重写核心逻辑。
迁移期保留两个分支名供现有集成使用，最终默认分支的重命名及远端分支清理另行处理。
不要将原 Standard 的历史兼容率直接当作重构后配置的验证结果。

CI 对 core、mono 都使用同一工作流，覆盖 Mono native 和 minimal 组合，
并继续验证 custom、Wasm 及 sanitizer/fuzz 路径。新增平台或引擎先增加适配实现及
明确的验证组合；不要承诺所有 BCL、宿主、平台和引擎的笛卡尔积。

本次不修改 CoreCLR、Unity 分支，不增加多线程实现，也不发布或强推远端。

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
