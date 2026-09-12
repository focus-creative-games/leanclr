# 点亮CLR技能树：LeanCLR 全面接入 .NET 10 开发笔记

对普通 C# 项目来说，切换运行时通常意味着“用新的 SDK 编译”。但对 LeanCLR 这类自研 runtime 来说，问题会下沉到更底层：它不只是要看懂用户写的代码，还要看懂 `.NET 10` 基础类库对运行时的要求。

LeanCLR 已经有自己的 metadata loader、类型系统、对象模型、解释器、GC、异常、委托、泛型、基础反射和 AOT 生成框架。因此，接入 `.NET 10` 不应被理解为重写一个完整 CLR。更准确地说，这是一次 **BCL 与 runtime contract 的 profile 化适配**：让 LeanCLR 能在明确边界内运行受控的 `net10.0` 纯逻辑程序集，同时把不支持的 BCL 能力暴露为清晰诊断。

如果你平时主要写业务层 C#，下面这几个基础概念先铺一下，会更容易理解后面的架构设计。

## C# 程序不是直接变成机器码

C# 源码经过 `dotnet build` 后，通常会生成 `.dll` 或 `.exe` 托管程序集。这个程序集里最重要的两类内容是：

- `IL`：Intermediate Language，中间语言。它不是 CPU 机器码，而是一组面向 .NET runtime 的指令。
- `metadata`：描述类型、字段、方法、泛型、特性、程序集引用等信息的数据表。

可以把程序集理解成“代码指令 + 自我描述”。runtime 加载程序集后，会先读 metadata，知道里面有哪些类型和方法，再按 IL 指令去执行方法体。

```mermaid
flowchart LR
    A["C# source"] --> B["dotnet build"]
    B --> C["托管程序集 .dll / .exe"]
    C --> D["IL 指令"]
    C --> E["metadata: 类型 / 方法 / 字段 / 引用"]
    D --> F["runtime 执行"]
    E --> F
```

所以 LeanCLR 要运行一个 C# 程序，最基础的能力不是“认识 C# 语法”，而是能加载程序集、读取 metadata、解释或编译 IL、创建对象、调用方法，并维护异常、GC、反射等运行时状态。

## Runtime 和 BCL 分别负责什么

日常写 C# 时，我们会直接用 `object`、`string`、`List<T>`、`Task`、`File`、`Console`、`Type` 这些类型。它们大多来自 .NET 的基础类库，也就是 BCL。

但 BCL 不是完全独立运行的普通库。很多底层能力必须找 runtime 帮忙。例如：

- `object.GetType()` 需要 runtime 知道某个对象真实的运行时类型。
- `new string(...)` 需要 runtime 按字符串对象布局分配和初始化内存。
- `Array.GetLength()` 需要 runtime 读取数组对象头里的长度。
- `Type.GetMethods()` 需要 runtime 能把 metadata 转成反射对象。
- `GC.Collect()` 需要 runtime 真的执行对象扫描和回收。
- `FileStream`、`Console` 这类 API 可能需要进入操作系统或宿主平台。

两者的分工大致是：

| 层次 | 负责什么 | 例子 |
| --- | --- | --- |
| 用户程序集 | 项目自己的业务逻辑 | 游戏逻辑、工具代码、框架核心 DLL |
| BCL | .NET 基础类型和库 API | `System.String`、`System.Collections`、`System.Reflection` |
| runtime | 执行 IL，管理对象、类型、GC、异常、线程、反射和 native 边界 | LeanCLR、CoreCLR、自研托管运行时 |
| 宿主 / OS | 提供平台能力 | 文件、控制台、Unity/Godot 主线程 API |

这也是为什么“能编译”不等于“能运行”。编译器只需要 reference assemblies 来检查 API 是否存在；runtime 运行时必须真的满足 BCL 的底层假设。

## corlib 是整个世界的地基

.NET 里有一个最核心的基础库，通常叫 `corlib`。它包含 `System.Object`、`System.String`、`System.Array`、`System.Type`、`System.Exception`、`System.Delegate` 等最基础类型。

在 `.NET 10` 里，这个 core library 是 `System.Private.CoreLib`。LeanCLR 要承载 `.NET 10`，首先就要能把 `System.Private.CoreLib` 当作地基加载起来，并理解它对 runtime 的要求。

这不是改个文件名那么简单。`System.Private.CoreLib` 里的核心类型、私有字段、内部方法、反射对象、句柄模型和 runtime 调用方式共同定义了 `.NET 10` BCL 期待的运行时形状。LeanCLR 需要重新确认：

- 核心类型从哪里加载。
- 对象、数组、字符串、异常等基础对象如何初始化。
- `RuntimeType`、`RuntimeMethodInfo`、`RuntimeFieldInfo` 等反射对象如何和 runtime 内部结构对应。
- BCL 里没有 IL 方法体的底层方法，应该由哪个 native helper 实现。

这就是后文反复出现的 `runtime contract`：BCL 期待 runtime 暴露出来的形状、入口和语义。

## TFM、runtime pack 和真正的运行支持

`TargetFramework=net10.0` 里的 TFM 是 Target Framework Moniker。它告诉 SDK：“请按 .NET 10 的 API 面来编译这个项目”。

但运行时还涉及另一组东西：

- reference assemblies：给编译器看的 API 声明。
- runtime assemblies：运行时真正加载的 BCL 实现。
- runtime pack：包含目标平台所需的 runtime assemblies 和 native 组件。
- runtime contract：BCL 运行时调用 native helper 时，runtime 必须提供的入口和语义。

```mermaid
flowchart TD
    A["TargetFramework=net10.0"] --> B["编译器使用 .NET 10 reference assemblies"]
    B --> C["生成 net10.0 用户程序集"]
    C --> D{"LeanCLR 能否运行"}
    D --> E["加载 .NET 10 runtime assemblies"]
    D --> F["识别 System.Private.CoreLib"]
    D --> G["支持 .NET 10 metadata / IL 组合"]
    D --> H["实现 BCL 需要的 runtime contract"]
```

所以，对 LeanCLR 来说，`.NET 10` 支持不是一个项目文件属性，而是一套从程序集加载、corlib 识别、metadata 解析到 BCL/native 边界的完整适配。

## 什么是 runtime contract

BCL 里有些方法是普通 C# 写的，有 IL 方法体，runtime 只要照着 IL 执行即可。还有一些方法没有普通 IL，或者需要 runtime 特殊处理，这些就是 contract 最密集的地方。

常见边界包括：

| 名词 | 可以先这样理解 |
| --- | --- |
| internal call / icall | BCL 方法没有 IL 方法体，实际实现写在 C++ runtime 里 |
| intrinsic | runtime 或 AOT 对某个方法做特殊识别，不按普通方法调用处理 |
| P/Invoke | 托管代码调用 native 函数，例如操作系统 API 或宿主 API |
| runtime handle | BCL 用来代表类型、方法、字段、模块等 runtime 对象的轻量句柄 |
| reflection | 托管代码在运行时查询类型、方法、字段、特性等 metadata 的能力 |

LeanCLR 适配 `.NET 10` 的核心，就是让 `System.Private.CoreLib` 和必要 BCL 在这些边界上看到它们期待的 contract；LeanCLR 内部则继续映射到自己的类型系统、metadata cache、解释器、GC 和反射框架。

## BCL contract 是从哪里发起的

这里容易产生一个误解：好像 LeanCLR 自己先定义了一套“BCL 接口协议”，然后让 BCL 来配合它。实际顺序正好相反。

**contract 的源头是当前选择的 BCL assemblies。** 编译器会把用户代码里的 `object.GetType()`、`typeof(int)`、`new string(...)`、`GC.Collect()` 等调用编码成 metadata token 和方法引用。LeanCLR 加载用户程序集后，会继续解析这些 token 指向哪个 BCL 类型、哪个 BCL 方法。只要这些方法来自 core library 或 runtime pack，它们就会反过来要求 runtime 提供某些能力。

可以把发起路径理解成：

```mermaid
flowchart LR
    A["用户 C# 代码"] --> B["编译器生成 IL + metadata token"]
    B --> C["token 指向 BCL 方法"]
    C --> D{"BCL 方法如何实现"}
    D --> E["普通 IL: runtime 解释执行"]
    D --> F["InternalCall / intrinsic / PInvoke"]
    F --> G["BCL 向 runtime 发起 contract 要求"]
    G --> H["LeanCLR profile/catalog 映射到 C++ runtime"]
```

所以 `runtime API catalog` 不是 contract 的源头，它只是 LeanCLR 对这套 contract 的登记表。真正的源头是：

- 当前 profile 选择的 BCL assemblies。
- 这些 assemblies 里的 metadata、IL、方法签名和 P/Invoke 声明。
- 对 `.NET 10` 来说，还要参考 `System.Private.CoreLib` 和 CoreCLR VM 源码理解语义。

这也是为什么 profile 很关键。本文只讨论 `coreclr-net10`：它表示 LeanCLR 当前面向 `.NET 10` runtime pack 和 `System.Private.CoreLib` 建立一套独立的 BCL/runtime contract。

对 `coreclr-net10` 来说，关键假设是：

- core library 是 `System.Private.CoreLib`。
- BCL contract 来源是 `.NET 10` runtime assemblies 里的 metadata、IL、方法签名和 P/Invoke 声明。
- native 边界按 `.NET 10` 的 internal call、intrinsic、generated P/Invoke 和 runtime handle 形态登记。
- LeanCLR 内部仍然映射到自己的 `RtClass`、`RtMethodInfo`、`RtFieldInfo`、metadata cache 和解释器。

从仓库结构上看，这个选择已经被显式写进 `coreclr-net10` profile：

```json
{
  "name": "coreclr-net10",
  "coreLibraryModules": [
    "System.Private.CoreLib",
    "System.Runtime",
    "System.Console",
    "System.Collections",
    "System.Linq",
    "System.Threading",
    "System.Runtime.InteropServices",
    "System.Reflection",
    "System.Private.Uri",
    "netstandard",
    "LeanCLR"
  ]
}
```

这段配置来自 `src/leanaot/LeanAOT/runtime-apis/coreclr-net10/profile.json`。它告诉 LeanCLR 的工具链：这些模块属于当前 `.NET 10` profile 的核心 BCL 面，后续判断 icall、intrinsic、P/Invoke 时要按这套世界来解释。

runtime 侧也有对应入口。`src/runtime/vm/assembly.cpp` 中的 `Assembly::load_corlib()` 当前会加载 `System.Private.CoreLib`：

```cpp
RtResult<metadata::RtAssembly*> Assembly::load_corlib()
{
    metadata::RtAssembly* loaded_corlib = get_corlib();
    if (loaded_corlib)
    {
        RET_OK(loaded_corlib);
    }

    return load_by_name(STR_SYSTEM_PRIVATE_CORELIB_NAME);
}
```

这就是 BCL 对齐的本质：不是把所有 API 都实现一遍，而是先选定 BCL 世界，再让 LeanCLR 的类型系统、metadata、解释器和 native helper 能够满足这个世界发起的 contract。

## 一个 C# 方法如何走到 LeanCLR runtime

下面用一个很小的例子把路径串起来。用户代码只是做 boxing、取类型、比较类型、再拆箱：

```csharp
using System;

namespace BlogDemo;

public static class Program
{
    public static int Run()
    {
        object value = 123;
        Type type = value.GetType();
        return type == typeof(int) ? (int)value + 1 : -1;
    }
}
```

项目文件选择 `.NET 10`：

```xml
<Project Sdk="Microsoft.NET.Sdk">
  <PropertyGroup>
    <TargetFramework>net10.0</TargetFramework>
  </PropertyGroup>
</Project>
```

编译后，`Run()` 里不会保存“C# 源码语义”，而是变成类似下面的 IL 和 metadata 引用。这里是简化后的形态：

```il
ldc.i4.s 123
box        [System.Private.CoreLib]System.Int32
stloc.0

ldloc.0
callvirt   instance class [System.Private.CoreLib]System.Type
           [System.Private.CoreLib]System.Object::GetType()
stloc.1

ldloc.1
ldtoken    [System.Private.CoreLib]System.Int32
call       class [System.Private.CoreLib]System.Type
           [System.Private.CoreLib]System.Type::GetTypeFromHandle(
               valuetype [System.Private.CoreLib]System.RuntimeTypeHandle)
call       bool [System.Private.CoreLib]System.Type::op_Equality(
               class [System.Private.CoreLib]System.Type,
               class [System.Private.CoreLib]System.Type)
```

注意几个细节：

- `box System.Int32` 要求 runtime 知道 value type 如何装箱成对象。
- `System.Object::GetType()` 来自 `System.Private.CoreLib`。
- `RuntimeTypeHandle`、`System.Type`、`Type.GetTypeFromHandle()` 都是 core library 的反射 contract。
- 这些引用的归属都是 `[System.Private.CoreLib]`，因此 runtime 必须按 `.NET 10` 的核心库形态解释。

如果用 `leanrun` 执行这个入口，命令大致长这样：

```powershell
leanrun `
  -l out\dotnet\BlogDemo\Release\net10.0 `
  -l <dotnet10-runtime-pack-dir> `
  -e BlogDemo.Program::Run `
  BlogDemo
```

`leanrun` 内部路径可以简化成这样，对应 `src/tools/leanrun/main.cpp`：

```cpp
vm::Runtime::initialize();

auto ass = vm::Assembly::load_by_name(dll_name.c_str());
metadata::RtModuleDef* mod = ass.unwrap()->mod;

const metadata::RtMethodInfo* entry_method =
    find_entry_method(mod, "BlogDemo.Program::Run");

vm::Runtime::invoke_array_arguments_with_run_cctor(
    entry_method,
    nullptr,
    nullptr);
```

进入解释器后，普通 IL 比如 `ldc.i4`、`stloc`、条件分支可以由 LeanCLR 自己执行。但当解释到 `System.Object::GetType()` 时，事情变成 BCL contract：这个方法来自 `System.Private.CoreLib`，它要求 runtime 返回当前对象的真实运行时类型。

在 `coreclr-net10` profile 里，LeanCLR 用 catalog 登记这个入口。摘自 `src/leanaot/LeanAOT/runtime-apis/coreclr-net10/icalls.json`：

```json
{
  "name": "System.Object::GetType",
  "func": "SystemObject::get_type",
  "header": "icalls/system_object.h"
}
```

这个登记表达的是：

- BCL 侧调用的是 `System.Object::GetType`。
- LeanCLR runtime 侧用 `SystemObject::get_type` 实现。
- 这个入口属于 `coreclr-net10` profile 的 native helper 边界。

对应 C++ 实现在 `src/runtime/icalls/system_object.cpp`：

```cpp
RtResult<vm::RtReflectionType*> SystemObject::get_type(vm::RtObject* obj) noexcept
{
    if (obj == nullptr)
        RET_ERR(RtErr::NullReference);

    return vm::Reflection::get_klass_reflection_object(obj->klass);
}
```

这个函数做的事很直接：

1. 从托管对象指针 `obj` 取出 LeanCLR 内部的 `klass`。
2. 把 `klass` 映射成托管世界可见的 reflection type 对象。
3. 返回给 `System.Private.CoreLib` 的 `Object.GetType()` 调用点。

用链路图表示就是：

```mermaid
sequenceDiagram
    participant User as "BlogDemo.Program.Run"
    participant IL as "IL interpreter"
    participant BCL as "System.Private.CoreLib method"
    participant Catalog as "coreclr-net10 icalls.json"
    participant Native as "SystemObject::get_type"
    participant VM as "LeanCLR Reflection / TypeSystem"

    User->>IL: value.GetType()
    IL->>BCL: 解析到 System.Object::GetType
    IL->>Catalog: 方法属于 core library, 查 runtime helper
    Catalog-->>IL: 映射到 SystemObject::get_type
    IL->>Native: 传入 RtObject*
    Native->>VM: obj->klass -> reflection type object
    VM-->>Native: RtReflectionType*
    Native-->>IL: 返回 System.Type 对象
    IL-->>User: 继续执行 typeof(int) 比较
```

现在就能直观看到为什么整条链路必须落在同一个 `coreclr-net10` contract 下：

- 如果用户程序集引用的是 `[System.Private.CoreLib]System.Object::GetType()`，runtime API catalog 里就必须有 `.NET 10` 视角下的这个入口。
- 如果 `.NET 10` BCL 期待返回的是它能识别的 `RuntimeType` / `RuntimeTypeHandle` 形态，LeanCLR 就必须把内部 `RtClass` 映射成对应的托管反射对象。
- 如果某个入口只按短名宽松匹配，不按完整签名匹配，就可能把 `.NET 10` 的包装方法、不同参数形态或不兼容 native helper 误连到一起。

所以，LeanCLR 对齐 BCL 的核心工作不是“实现一个叫 `GetType` 的函数”这么简单，而是要保证整条链路都在同一个 profile 下成立：

```text
用户 IL token
  -> System.Private.CoreLib 方法
  -> coreclr-net10 catalog
  -> LeanCLR native helper
  -> LeanCLR 类型系统 / metadata / reflection 对象
  -> 返回给 .NET 10 BCL 可理解的对象形态
```

## minimal net10 profile 的边界

直接承载完整 `Microsoft.NETCore.App` 相当于把 LeanCLR 推向“小型 CoreCLR”。这会把 ThreadPool、网络、文件系统、反射全量语义、动态代码生成、平台 PAL、COM/Interop 等大量子系统全部拉进当前范围。

更稳妥的边界是 **minimal net10 profile**：

- 运行项目自己的、受控的、纯逻辑 `net10.0` DLL。
- 支持基础对象模型、数组、字符串、异常、委托、泛型、基础反射和必要 Span/Unsafe 路径。
- 使用 `.NET 10` runtime pack 作为托管 BCL 输入，但只承诺当前 profile 白名单内的 runtime API。
- 对越界的 BCL、ThreadPool、Timer、I/O、网络、动态代码生成等能力给出可诊断失败，而不是静默 stub。
- Unity/Godot 等引擎 API 通过宿主 bridge 暴露，不把引擎程序集当作普通 BCL 直接加载进 runtime。

这个边界的关键是：LeanCLR 不是复刻整个 CoreCLR，而是实现 `.NET 10` BCL 在当前 profile 下真正跨 native 边界所需的 contract。

## 托管代码在 LeanCLR 中如何执行

一个 C# 程序进入 LeanCLR 后，大致会经过以下路径：

```mermaid
flowchart TD
    A["C# source"] --> B["dotnet build 生成程序集"]
    B --> C["LeanCLR 加载用户程序集"]
    C --> D["解析 AssemblyRef / TypeRef / MemberRef"]
    D --> E["选择 runtime API profile"]
    E --> F["加载 profile 对应 core library"]
    F --> G["初始化核心类型 Object / String / Array / Type"]
    G --> H["读取 metadata: 类型 / 字段 / 方法 / 泛型 / 特性"]
    H --> I{"方法如何执行"}
    I --> J["解释执行 IL"]
    I --> K["LeanAOT 生成 native 路径"]
    J --> L["调用 LeanCLR runtime 服务"]
    K --> L
    L --> M{"遇到 runtime 边界"}
    M --> N["internal call / QCall / FCall 风格入口"]
    M --> O["intrinsic"]
    M --> P["P/Invoke / host bridge"]
    N --> Q["C++ runtime 或宿主 native API"]
    O --> Q
    P --> Q
    Q --> R["返回托管世界继续执行"]
```

对 `coreclr-net10` 来说，图里的 profile 选择会把 core library 固定到 `System.Private.CoreLib`，把 BCL runtime API 固定到 `.NET 10` runtime pack 导出的入口和签名。如果这些入口没有被登记，或者登记后返回的对象形态不符合 `.NET 10` BCL 的预期，失败通常会出现在 core library 加载、核心类型初始化、反射句柄解析或 BCL runtime API 调用处。

## profile 是隔离边界

profile 决定当前 runtime 面向哪套 BCL 和 runtime API contract。本文只讨论 `coreclr-net10`，它的作用是把 `.NET 10` core library、runtime pack、runtime API catalog 和 LeanCLR native helper 绑定在同一条解释规则下。

```mermaid
flowchart TD
    A["选择 coreclr-net10 profile"] --> B["core library: System.Private.CoreLib"]
    A --> C["BCL 输入: .NET 10 runtime pack"]
    A --> D["runtime API catalog: coreclr-net10"]
    D --> E["icall / intrinsic / PInvoke entries"]
    E --> F["LeanCLR C++ runtime helpers"]
    F --> G["RtClass / RtMethodInfo / RtFieldInfo / metadata cache"]
```

正确做法是让 corlib 识别、required type 表、internal call 表、intrinsic 表、P/Invoke facade 和诊断逻辑都跟随 `coreclr-net10`。这样读者看到某个 BCL 入口时，可以一路追到它在 `.NET 10` runtime pack 中的来源、catalog 中的登记，以及 LeanCLR C++ runtime 里的实现。

## runtime API catalog 的作用

`.NET 10` BCL 中有不少方法没有普通 IL 方法体，或者需要 runtime 特殊处理。它们通常落在几类边界上：

| 边界 | 含义 | LeanCLR 需要做的事 |
| --- | --- | --- |
| internal call / QCall / FCall | 托管 BCL 直接调用 runtime native helper | 按完整类型名、方法名和签名注册 C++ 实现 |
| intrinsic | 解释器或 AOT 对特定方法做特殊识别 | 为 `Span<T>`、`RuntimeHelpers`、`Interlocked` 等建立 profile 内映射 |
| P/Invoke / LibraryImport | 托管代码调用 native 动态库或 generated P/Invoke stub | 映射到 LeanCLR 平台层、host bridge 或明确 NotSupported |
| runtime implemented method | 方法语义由 runtime 提供，不适合按普通 IL 执行 | 在解释器和 AOT 中共享同一 contract |

runtime API catalog 是这些边界的索引。它告诉 LeanCLR：在当前 profile 下，哪些托管方法需要 runtime 提供实现，签名是什么，应该走 icall、intrinsic 还是 P/Invoke 路径。

即使 AOT 暂时后置，这张表仍然重要。解释器遇到 BCL 内部方法时同样需要解析 runtime helper；未来 AOT codegen 也应复用同一套 catalog，而不是维护另一份语义。

```mermaid
flowchart TD
    A["输入程序集和 BCL"] --> B["读取 metadata / IL"]
    B --> C["按 profile 加载 runtime API catalog"]
    C --> D{"遇到方法调用"}
    D --> E["普通 IL: 解释器执行"]
    D --> F["icall: 调用 runtime helper"]
    D --> G["intrinsic: runtime 特殊处理"]
    D --> H["P/Invoke: 进入 native / host 边界"]
    E --> I["统一 runtime 语义"]
    F --> I
    G --> I
    H --> I
    I --> J["解释执行与 AOT 共享 contract"]
```

## icall 为什么不能宽松匹配

internal call 是 BCL 和 runtime 之间最直接的通道。托管侧可能长这样：

```csharp
[MethodImpl(MethodImplOptions.InternalCall)]
private static extern RuntimeType InternalGetType(object obj);
```

方法没有 IL 方法体，运行时需要根据“类型名 + 方法名 + 签名”找到 native 实现：

```cpp
{"System.Type::internal_from_handle", (vm::InternalCallFunction)&SystemType::internal_from_handle}
```

调用流程可以理解为：

```mermaid
sequenceDiagram
    participant Managed as "托管 BCL 方法"
    participant VM as "LeanCLR 调用解析"
    participant Table as "icall 表"
    participant Native as "C++ runtime 实现"

    Managed->>VM: 调用 InternalCall 方法
    VM->>Table: 按完整签名查找入口
    alt 找到入口
        Table-->>VM: 返回函数指针和 invoker
        VM->>Native: 转换参数并调用 C++ 实现
        Native-->>VM: 返回结果或异常
        VM-->>Managed: 回到托管调用点
    else 没找到入口或签名不匹配
        VM-->>Managed: 抛出清晰诊断
    end
```

这里最容易出错的是“宽松匹配”。如果 runtime 只按 `System.RuntimeFieldHandle::GetToken` 这种短名称匹配，而不校验完整签名，就可能把 `.NET 10` 托管侧的包装方法误认为 native 入口，甚至把托管对象指针当作 native 字段句柄传入。

因此 `coreclr-net10` profile 需要偏向精确签名：

- icall 注册以完整方法签名为主。
- 不属于 `coreclr-net10` catalog 的入口不应在 `System.Private.CoreLib` 路径下隐式兜底。
- 找不到匹配项时应 fail fast，并输出 profile、方法签名和调用来源。

## CoreCLR contract 应映射到 LeanCLR 模型

适配 `.NET 10` 不等于把 CoreCLR VM 代码照搬进 LeanCLR。CoreCLR 源码能提供非常重要的参考：BCL 调用链、QCall/FCall/InternalCall 名称、签名、异常语义和 runtime handle 预期。但 LeanCLR 的对象布局、GC、metadata loader、method table、exception、reflection handle 和解释器调用协议是自己的。

更合理的架构是：外部暴露 CoreCLR BCL 期待的形状，内部映射到 LeanCLR 自身模型。

```mermaid
flowchart LR
    A[".NET 10 System.Private.CoreLib"] --> B["CoreCLR 风格 runtime contract"]
    B --> C["LeanCLR net10 facade"]
    C --> D["RtClass"]
    C --> E["RtMethodInfo"]
    C --> F["RtFieldInfo"]
    C --> G["metadata cache"]
    C --> H["interpreter / GC / exception / reflection"]
```

典型例子包括：

- `RuntimeType` / `RuntimeTypeHandle` 映射到 LeanCLR 的类型系统。
- `RuntimeMethodHandle` 映射到 `RtMethodInfo` 和解释器可调用方法描述。
- `RuntimeFieldHandle` 映射到 `RtFieldInfo`、字段布局和静态字段存储。
- `RuntimeModule`、`MetadataImport` 和 token 查询映射到 LeanCLR metadata cache。
- `CustomAttributeData`、`Activator`、泛型实例化和私有成员访问落到 LeanCLR 反射框架。

这样做的目标是把 `.NET 10` BCL 的外部 contract 和 LeanCLR 内部实现解耦。BCL 看到的是它熟悉的 CoreCLR 风格 facade；LeanCLR 内部仍保持自己的 VM 架构。

## BCL 兼容和引擎 bridge 是两件事

`.NET 10` BCL 兼容解决的是 `System.*`、`System.Private.CoreLib` 和 `Microsoft.NETCore.App` 能不能在 LeanCLR 上运行。Unity/Godot/UE 接入解决的是引擎对象、资源、场景、主线程 API 和异步结果如何暴露给托管业务代码。

这两件事不应该混在一起。

```mermaid
flowchart LR
    A["LeanCLR 托管业务代码"] --> B[".NET 10 / profile BCL"]
    A --> C["引擎抽象接口: asset / scene / game-loop / audio / ui"]
    C --> D["LeanCLR host bridge ABI"]
    D --> E["宿主侧 handle registry"]
    E --> F["Unity/Godot/UE 主线程 dispatcher"]
    F --> G["真实 UnityEngine / Godot / Unreal API"]
```

比较稳的引擎接入方式是：

- LeanCLR 只运行引擎无关核心逻辑、业务程序集和必要 `.NET 10` BCL 子集。
- Unity/Godot/UE 进程作为宿主，负责创建 LeanCLR runtime、装载程序集、驱动 frame pump。
- 引擎对象不直接跨 runtime 边界传递，而是用 `int`、`long`、`IntPtr` 或 opaque handle 表示。
- 宿主维护 handle registry，把 handle 映射到真实 Unity `Object`、Godot `Object`、Unreal `UObject`、Node、Resource、Actor、Component 等。
- 所有必须主线程调用的引擎 API 都进入宿主主线程 dispatcher。
- 异步加载、场景切换、资源请求等结果通过 bridge 回填到 LeanCLR 的 continuation 或等价调度结构。

这种模型把 LeanCLR 放在“native CLR-like runtime”的位置，把 Unity/Godot/UE 放在外部 service provider 的位置。业务代码看到的是抽象服务和 handle，不需要 LeanCLR 直接理解整套引擎对象模型。

## 三类引擎接入的共同骨架

NKGGameFramework 的 Godot 样例已经验证了一条可复用路径：业务程序集面向 `net10.0` 编译，LeanCLR 在引擎进程内加载业务 DLL 和 staged BCL，托管侧每帧输出 host command buffer，宿主侧把命令落到真实引擎对象。这个路径不要求 LeanCLR 引用 GodotSharp、UnityEngine 或 Unreal Engine 的 managed runtime。

通用流程可以抽象成：

```mermaid
flowchart TD
    A["业务 / 框架 C# 源码"] --> B["dotnet build: net10.0 managed assemblies"]
    B --> C["计算 AssemblyRef 闭包"]
    C --> D["stage minimal net10 BCL / adapter assemblies"]
    E["引擎插件 / native module"] --> F["启动 LeanCLR Runtime"]
    F --> G["注册 assembly loader"]
    G --> H["加载业务程序集和 adapter assembly"]
    H --> I["绑定 managed entry: Reset / Tick / Debug / Host callbacks"]
    J["引擎主循环"] --> K["主线程 dispatcher / frame pump"]
    K --> I
    I --> L["RuntimeContext.Update / ECS / gameplay logic"]
    L --> M["host command buffer 或 host service call"]
    M --> N["native bridge 解码"]
    N --> O["handle registry"]
    O --> P["真实引擎对象 / 资源 / 场景"]
    Q["debug transport"] --> K
    K --> R["managed Diagnostics command endpoint"]
```

这里的关键不是三个引擎共用同一份 native 代码，而是共用同一条边界：

| 层次 | 放什么 | 不放什么 |
| --- | --- | --- |
| LeanCLR core | `.NET 10` BCL profile、metadata、IL、GC、反射、icall catalog | 引擎对象模型 |
| managed game assemblies | RuntimeContext、ECS、玩法、debug domain、引擎抽象 contract | UnityEngine/GodotSharp/Unreal managed 全量 API |
| adapter assembly | handle、Variant、command buffer、少量 service interface | 真实引擎对象生命周期 |
| native engine plugin | runtime loader、assembly loader、主线程 pump、object/resource registry、transport | BCL 兼容逻辑 |

### Godot 接入流程

Godot 路径最接近当前 NKGGameFramework 的实际接入过程。`NKGGameFramework.Adapter.Godot` 承载 managed contract 和 `GodotHostCommandBuffer`；GDExtension native 层负责启动 LeanCLR、注册文件加载器、加载 `NKGGameFramework.GodotPlaneSample.dll`，并把 `_process` 转成 managed tick。托管侧返回 `byte[]` command buffer，native 层解码 `CreateNode`、`SetProperty`、`LoadResource`、`InstantiateScene` 等命令，落到 Godot `Object` / `Node` / `Resource` registry。

```mermaid
flowchart TD
    A["dotnet build Godot sample: net10.0"] --> B["NKGGameFramework.GodotPlaneSample.dll"]
    A --> C["NKGGameFramework.dll / Adapter.Godot.dll / Diagnostics.dll"]
    B --> D["stage-leanclr-bcl: 复制 System.Private.CoreLib 和引用闭包"]
    C --> D
    E["Godot project.godot"] --> F["GDExtension: nkg_leanclr_bridge"]
    F --> G["NkgLeanClrRuntimeBridge.configure(managedDir + bclDir)"]
    G --> H["Runtime::initialize + Settings::set_file_loader"]
    H --> I["Assembly::load_by_name(sample assembly)"]
    I --> J["绑定 PlaneGameBridge 静态入口"]
    K["NkgLeanClrPlaneHost._process(delta)"] --> L["ClearInput / Press* / StepSessionCommandBytes"]
    L --> M["RuntimeContext.Update + World.Update"]
    M --> N["GodotHostCommandBuffer byte[]"]
    N --> O["NkgGodotHostCommandReader"]
    O --> P["NkgGodotObjectRegistry / ResourceRegistry"]
    P --> Q["Godot ClassDB / Object::set / Object::callv / ResourceLoader"]
    R["native HTTP/SSE debug transport"] --> S["主线程安全点"]
    S --> T["PlaneGameBridge.HandleDebugRequest"]
    T --> U["Diagnostics GameDebugEndpointDispatcher"]
```

Godot 的经验给 LeanCLR 接入定了几个很实用的约束：

- `leanclr_bcl/net10.0` 是随工程 staged 的 runtime assembly 目录，运行时不启动 CoreCLR host。
- Godot 官方 C# / GodotSharp 不在主路径里，避免把 Godot 的 .NET 支持状态变成 LeanCLR 的平台限制。
- GDScript 只做 autoload、配置和可选 facade，不承载 ECS、Gameplay、Serialization、Debug domain。
- debug transport 在 native/Godot 层，managed 侧只处理 command endpoint 和 payload，不要求 LeanCLR 实现 `System.Net` server。
- 所有 snapshot、mutation、pause/step 都通过主线程安全点执行，避免 transport 线程直接读写 ECS。

### Unity 接入流程

Unity 路径可以复用同一个模型，但宿主胶水会不同。Unity 自身已经有 C# 域，但 LeanCLR 不应该把 Unity 的 managed runtime 当成自己的 BCL，也不应该在 LeanCLR 里直接加载 `UnityEngine.dll` 作为业务依赖。更稳妥的做法是：Unity C# 侧保留一个薄 `MonoBehaviour` host，加载 LeanCLR native plugin，传入 managed/BCL 路径，之后每帧只做输入采集、主线程调度和 command buffer 应用。

```mermaid
flowchart TD
    A["NKG / game logic: net10.0"] --> B["NKGGameFramework.dll"]
    A --> C["NKGGameFramework.Adapter.Unity.dll"]
    B --> D["Unity StreamingAssets 或 package 内 managed 目录"]
    C --> D
    E["Unity Editor / Player"] --> F["LeanCLR native plugin"]
    G["LeanClrUnityHost MonoBehaviour"] --> F
    G --> H["传入 managedDir / bclDir / entry assembly"]
    F --> I["Runtime::initialize + assembly loader"]
    I --> J["绑定 managed Reset / Tick / Debug 方法"]
    K["Update / FixedUpdate / LateUpdate"] --> G
    G --> L["采集 Input / Time / scene context"]
    L --> J
    J --> M["RuntimeContext.Update / gameplay logic"]
    M --> N["UnityHostCommandBuffer"]
    N --> O["Unity native/C# host command reader"]
    O --> P["handle registry: GameObject / Component / Asset"]
    P --> Q["Instantiate / Destroy / SetProperty / CallMethod / LoadAsset"]
    R["Editor debug window / loopback transport"] --> G
    G --> S["managed Diagnostics command endpoint"]
```

Unity 首版不需要追求“在 LeanCLR 里完整写 Unity 脚本”。更合理的分期是：

1. `Adapter.Unity` 先定义 `IUnityGameLoopDriver`、asset、scene、audio、UI 等 host service contract。
2. Unity host 把 `Update` / `FixedUpdate` 映射成 LeanCLR 的 frame pump。
3. 托管业务输出对象命令，Unity host 维护 `int`/`long` handle 到 `GameObject`、`Component`、`ScriptableObject`、asset 的映射。
4. Editor/desktop debug transport 可以由 Unity C# 或 native plugin 承载，再转成 Diagnostics command。
5. 后续如果要暴露更多 Unity API，优先做生成式 adapter 和受控 facade，而不是把 `UnityEngine` 全量塞进 LeanCLR BCL profile。

### Unreal Engine 接入流程

UE 接入要额外处理反射、UObject 生命周期、GC、蓝图和热重载边界。这里可以参考 UnrealSharp 的方向：它面向 UE5，把 C# 放在 `.NET 10` 上，支持从 `UClass` 派生、用特性标注 `UProperty` / `UFunction`，并从 Unreal 反射出来的 C++ 类型生成 C# binding。LeanCLR 的接入可以借鉴这套“生成 binding + native module 承载 UObject 边界”的结构，但运行时仍然是 LeanCLR，而不是把 UnrealSharp 的 .NET host 直接搬进来。

```mermaid
flowchart TD
    A["Unreal C++ project / plugin"] --> B["UHT 生成反射数据"]
    B --> C["LeanCLR Unreal binding generator"]
    C --> D["Leanclr.Unreal.Adapter.dll: UObjectHandle / Actor facade / generated thunks"]
    E["Gameplay C# net10.0"] --> F["managed game assemblies"]
    D --> F
    G["Unreal plugin module"] --> H["StartupModule / GameInstanceSubsystem"]
    H --> I["启动 LeanCLR Runtime"]
    I --> J["加载 BCL / adapter / gameplay assemblies"]
    J --> K["绑定 Tick / BeginPlay / EndPlay / Debug / event dispatch"]
    L["UE game thread Tick"] --> K
    K --> M["managed gameplay / ECS / state machine"]
    M --> N["UnrealHostCommandBuffer 或 host service call"]
    N --> O["native bridge thunks"]
    O --> P["UObject registry: handle -> TWeakObjectPtr<UObject>"]
    P --> Q["SpawnActor / SetProperty / Call UFunction / LoadObject"]
    R["Blueprint / UFunction event"] --> S["native event thunk"]
    S --> K
```

UE 建议拆成两个阶段：

| 阶段 | 目标 | 边界 |
| --- | --- | --- |
| Phase 1 | LeanCLR 托管逻辑驱动 UE 对象命令 | C# 业务不直接继承 `AActor`，只通过 handle facade 控制 Actor、Component、Asset |
| Phase 2 | 生成式 UClass/UFunction 代理 | 参考 UnrealSharp 的 generated binding 思路，为 C# partial 类型生成 native proxy、UFunction thunk 和属性 marshal |

Phase 1 更适合验证 LeanCLR 的 runtime contract，因为它只要求：

- UE plugin module 在 game thread 初始化 LeanCLR，并在 Tick 安全点调用 managed entry。
- adapter assembly 提供 `UObjectHandle`、`ActorHandle`、`ComponentHandle`、`UnrealVariant`、command buffer 和 host service interface。
- native bridge 用 `TWeakObjectPtr<UObject>` 或等价结构维护 handle registry，避免 LeanCLR 持有裸 UObject 指针。
- 蓝图事件、input、timer、asset load 先进入 UE game thread，再通过 dispatch queue 进入 LeanCLR。
- UE GC 与 LeanCLR GC 不互相扫描对象图，只通过 handle 的 pin/release/weak validity contract 协作。

Phase 2 才引入更接近 UnrealSharp 的体验：C# 类型用特性描述 `UClass`、`UProperty`、`UFunction`，生成器根据 UHT/reflection 数据生成 C# facade 和 C++ thunk。这里仍然要保持一个原则：Unreal 的反射系统属于 UE adapter，不属于 `.NET 10` BCL profile；LeanCLR 看到的是 adapter DLL、metadata 和受控 native bridge，不直接承担整个 UObject runtime。

## 接入验证矩阵

三类引擎都应该先用同一组最小验收来收敛边界，而不是一开始追求完整引擎 API：

| 场景 | Godot | Unity | UE |
| --- | --- | --- | --- |
| Runtime 启动 | GDExtension 初始化 LeanCLR | native plugin 初始化 LeanCLR | plugin module / subsystem 初始化 LeanCLR |
| Assembly 加载 | managed dir + `leanclr_bcl/net10.0` | StreamingAssets/package staged BCL | plugin content staged BCL |
| Frame pump | `_process` / `_physics_process` | `Update` / `FixedUpdate` | game thread Tick |
| 对象输出 | `GodotHostCommandBuffer` | `UnityHostCommandBuffer` | `UnrealHostCommandBuffer` |
| 对象 registry | `Object` / `Node` / `Resource` | `GameObject` / `Component` / asset | `UObject` / `AActor` / `UActorComponent` |
| Debug transport | native HTTP/SSE 或 JS bridge | Editor window / loopback / player bridge | Editor tool / loopback / console command |
| 首个 smoke | managed sampler 每帧驱动可见对象 | managed sampler 创建/移动 GameObject | managed sampler SpawnActor/SetProperty |

只要这张矩阵跑通，后续扩展资源、UI、动画、物理、蓝图、热重载时，就能继续围绕 adapter 和 host bridge 演进，而不是把每个引擎的复杂度反向压进 LeanCLR core。

## 单线程 runtime 的同步边界

如果 LeanCLR 的 Standard/Core 形态明确是单线程 runtime，那么 `.NET 10` profile 也应把同步语义写成架构边界。

这不代表完全不能执行 async/await。许多 async state machine、completed task、同步 continuation、next-frame continuation 都可以映射到单线程 frame scheduler 或宿主 dispatcher。真正需要谨慎的是 ThreadPool worker、Timer、WaitHandle、后台线程、I/O completion port 和跨线程调度。

推荐把同步能力分成三类：

| 类别 | 处理方式 |
| --- | --- |
| 单线程可表达 | async state machine、同步 continuation、frame scheduler、host dispatcher |
| 可降级或替代 | timer/next-frame、资源加载回调、主线程任务队列 |
| 不进入 profile | ThreadPool worker、后台线程、复杂 WaitHandle、跨线程 Monitor 语义 |

对第三类 API，清晰的 NotSupported 诊断比空实现更重要。空 stub 会让业务代码在更远的位置以更难定位的方式失败。

## 哪些 LeanCLR 能力应该复用

`.NET 10` 适配的主战场不是 LeanCLR VM 核心，而是 BCL/runtime contract 层。以下能力应尽量复用：

- 程序集与 metadata 解析。
- 类型系统和对象模型。
- IL 解释器。
- LeanAOT IL 到 C++ 的生成框架。
- GC 和对象扫描。
- 异常处理。
- 委托调用。
- 泛型与泛型共享。
- 基础反射框架。

需要重建或重新校准的是：

- corlib 名称和核心模块识别。
- `System.Private.CoreLib` 对核心类型、runtime handle 和反射对象的预期。
- `coreclr-net10` runtime API catalog。
- internal call、intrinsic、P/Invoke 和 runtime implemented method 的签名与行为。
- assembly resolver 对用户程序集、NuGet 依赖、runtime pack 和 adapter assembly 的搜索策略。
- 现代 C# / .NET metadata 与 IL 组合。
- 单线程 async、continuation、timer 与宿主调度的边界。
- 反射型序列化、private member access、attribute 查询等真实 workload 会组合触发的能力。

## 结语

LeanCLR 承载 `.NET 10` 的关键不是“追平 CoreCLR”，而是把目标拆成清晰的 contract：

- profile 隔离 BCL 语义。
- runtime API catalog 描述 native 边界。
- CoreCLR 风格 facade 对接 `.NET 10` BCL。
- LeanCLR 内部继续复用自己的 VM 模型。
- Unity/Godot 通过 host bridge 接入，而不是混入 BCL 兼容层。

只要这些边界保持清晰，`.NET 10` 支持就可以从一个开放式的兼容黑洞，收敛成可审查、可诊断、可演进的 runtime profile。
