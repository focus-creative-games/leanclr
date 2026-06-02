# LeanCLR 测试工程指南

本文档描述 `src/tests/` 下 managed 测试工程的职责划分、编写约定，以及构建与运行方式。

更完整的仓库级脚本说明见 [`scripts/README.md`](../../scripts/README.md)。

---

## 目录结构

```
tests/
├── basic-tester/          # C++ 测试 runner（CI 主路径，加载 CoreTests + CorlibTests）
├── aot-tester/            # AOT 测试 runner（加载 AotTests）
├── managed/
│   ├── managed.sln        # 主 solution（不含 ILTests，见下文）
│   ├── Common/            # 共享基础设施（Assert、UnitTest 等）
│   ├── SharedTests/       # 跨项目共享测试源（编译进 CoreTests / AotTests）
│   ├── CoreTests/         # CLR 运行时测试（解释器、IL 指令、C# 语义）
│   ├── CorlibTests/       # BCL 测试（internalcall、intrinsic、P/Invoke）
│   ├── AotTests/          # LeanAOT 测试（IL → C++ 正确性）
│   ├── ILTests/           # IL asm 测试（尚未接入 solution / CI）
│   └── RunTests/          # C# 反射 runner（本地调试用）
└── TESTING.md             # 本文件
```

---

## 测试项目职责

| 项目 | 测什么 | 不应放什么 |
|------|--------|------------|
| **CoreTests** | CLR 自身实现：解释器、IL 指令（C# 可写部分）、C# 语言特性、回归用例 | BCL internalcall 细节、AOT 编译链路专有测 |
| **CorlibTests** | mscorlib / System / System.Core：internalcall、intrinsic、P/Invoke | 纯 CLR 指令语义（应在 CoreTests） |
| **ILTests** | C# 无法或难以稳定构造的 IL 用例（`.il` + ilasm） | 能用 C# 表达的测试 |
| **AotTests** | LeanAOT 将 IL 翻译为 C++ 后的正确性 | 与 CoreTests 完全重复的指令测（计划合并到 SharedTests，见重构路线图） |
| **Common** | `Assert`、`[UnitTest]`、`GeneralTestCaseBase` 等 | 测试用例本身 |

---

## Runner 矩阵

| Runner | 加载的程序集 | 用途 |
|--------|-------------|------|
| **basic-tester** (`test.exe`) | CoreTests, CorlibTests, Common | **CI 主路径** |
| **RunTests.exe** | CoreTests, CorlibTests, AotTests | 本地一次性跑三套 managed 测试 |
| **aot-tester** | AotTests | AOT 流水线（`scripts/test/aot-runner/`） |
| **AotTests.App.Main** | AotTests（自扫描） | AOT 生成 C++ 时的入口 |

三套 runner 的 discovery 规则基本一致，但存在细微差异：

| 规则 | basic-tester | RunTests | AotTests.App |
|------|-------------|----------|--------------|
| 标记 | `[UnitTest]` | `[UnitTest]` | `[UnitTest]` |
| 签名 | `void`，无参 | `void`，无参 | `void`，无参 |
| static 方法 | 不支持（总是 `new` 实例） | 支持 | 支持 |
| 跳过 | 无 | `[IgnoreTest]` 类级 | 无 |
| 推荐基类 | `GeneralTestCaseBase` | 同左 | 多数类未继承（待统一） |

---

## 编写测试

### 测试类

- 推荐继承 `GeneralTestCaseBase`（位于 Common 项目）
- 需要无参构造函数（隐式默认构造即可）
- 测试类文件名推荐 `TC_{主题}.cs`

```csharp
namespace Tests.CSharp
{
    class TC_MyFeature : GeneralTestCaseBase
    {
        [UnitTest]
        public void Addition()
        {
            Assert.Equal(3, 1 + 2);
        }
    }
}
```

### 测试方法

| 要求 | 说明 |
|------|------|
| 访问修饰符 | `public` |
| 返回类型 | `void` |
| 参数 | 无 |
| 属性 | `[UnitTest]` |
| 实例方法 | basic-tester 要求非 static；RunTests 允许 static |

### 跳过测试

在类或方法上标记 `[IgnoreTest]`（RunTests 支持类级跳过）。

### Assert API

`Assert` 定义在 Common 项目中，常用方法：

| 方法 | 说明 |
|------|------|
| `Fail()` / `Fail(string)` | 立即失败 |
| `IsTrue` / `IsFalse` / `True` / `False` | 布尔断言 |
| `Null` / `NotNull` | 空引用断言 |
| `Equal(T, T)` | 值相等（支持 int、long、float、double、string、Type 等） |
| `NotEqual` | 不等断言 |
| `EqualAny` | 使用 `Object.Equals` 比较 |

断言失败时抛出异常，runner 捕获后报告 `[FAIL]`。

### 各项目放置指南

| 场景 | 目标项目 | 目录建议 |
|------|----------|----------|
| IL 指令（C# 可写） | CoreTests | `Instructions/`（如 `Instructions/Arithmetic/`） |
| C# 语言特性 | CoreTests | `Runtime/` |
| 杂项运行时测 | CoreTests | `Runtime/Misc/` |
| 历史 Bug 回归 | CoreTests | `Regression/` |
| 测试辅助类型 / fixture | CoreTests | `Shared/Fixtures/`（`namespace AOTDefs`） |
| C++ 引导测 | CoreTests | `Bootstrap/`（`namespace BootstrapTests`） |
| BCL internalcall | CorlibTests | `InternalCall/` |
| 纯 IL asm | ILTests | `*.il` |
| 跨解释器 + AOT 共享 | SharedTests | `SharedTests/Instructions/` 或 `Runtime/` |
| AOT 编译/链接特有 | AotTests | 项目根目录 |

### CoreTests 目录结构（PR4 后）

```
CoreTests/
├── Runtime/                 # C# 语言特性（原 Tests/CSharp）
│   └── Misc/                # 杂项（原 Tests/Mics）
├── Instructions/            # IL 指令（原 Tests/Instruments）
│   ├── Arithmetic/          # 原 Ariths
│   ├── Arrays/
│   ├── Boxing/              # 原 Boxs
│   ├── Branches/
│   ├── Compare/             # 原 Cmps
│   ├── Exceptions/
│   ├── Fields/
│   ├── Funcs/
│   ├── Memory/              # 原 Mems
│   ├── Misc/
│   ├── NotImpls/
│   └── Objects/             # 原 Objs
├── Regression/              # Bug 回归（原 Tests/Bugs）
├── Shared/Fixtures/         # 辅助类型（原 AOTDefs，namespace 仍为 AOTDefs）
├── Bootstrap/               # C++ 硬编码引导测（namespace 仍为 BootstrapTests）
├── App.cs, App2.cs, CustomPInvoke.cs
└── Properties/
```

命名空间（`Tests.CSharp`、`Tests.Instruments.*`、`AOTDefs` 等）暂未变更，仅调整物理目录。

---

## 构建

### C++ basic-tester

```batch
scripts\test\basic-tester\build.bat [Config] [Arch]
rem 例：scripts\test\basic-tester\build.bat Debug x64
```

输出：`out/cmake/tests/basic-tester/<Config>-<Arch>/bin/<Config>/test.exe`

### Managed 测试程序集

使用 MSBuild 或 `dotnet build` 构建 solution：

```batch
dotnet build src\tests\managed\managed.sln -c Debug
dotnet build src\tests\managed\managed.sln -c Release
```

输出目录（由根目录 `Directory.Build.props` 配置）：

```
out/dotnet/<ProjectName>/<Config>/
```

例如：`out/dotnet/CoreTests/Debug/CoreTests.dll`

### 部署 DLL 到 runner

basic-tester 从可执行文件旁的 `dlls/` 目录加载程序集。构建 managed 项目后，需将以下 DLL 复制到 runner 输出目录的 `dlls/` 下：

- `Common.dll`
- `CoreTests.dll`
- `CorlibTests.dll`

> **注意**：`scripts/test/build-all.bat` 在文档中有引用，但尚未实现。当前请分别构建 basic-tester 与 managed solution，并手动或通过 CI 脚本复制 DLL。后续 PR 会补齐一键构建脚本。

### AOT 测试

```batch
scripts\build.bat aot-tester gen-cpp
scripts\build.bat aot-tester run [Config] [Arch]
```

---

## 运行

### CI 主路径（解释器 + BCL）

```batch
scripts\test\run.bat [Config] [Arch]
rem 例：scripts\test\run.bat Debug x64
```

前提：已构建 basic-tester，且 `dlls/` 中有所需 managed DLL。

### 本地调试（含 AotTests）

构建并运行 `RunTests` 项目（需先构建整个 solution）：

```batch
dotnet build src\tests\managed\managed.sln -c Debug
out\dotnet\RunTests\Debug\RunTests.exe
```

---

## 命名规范（目标状态）

重构进行中，新用例请尽量遵循以下约定：

| 类别 | 规范 | 示例 |
|------|------|------|
| 测试类 | `TC_{主题}` | `TC_conv_i4` |
| 回归用例 | `Issue_{yyyyMMdd}_{desc}` | `Issue_20220617_ArrayCustomArg` |
| Helper / Fixture | 无 `TC_` 前缀 | `TypeStaticFields` |
| Interp / AOT 对照 | 后缀 `_interp` / `_aot` | `TC_call_interp.cs` |

避免在同一语义上同时使用 `Test*` 与 `TC_*` 两套命名（PR3 已完成 AotTests 侧 `Test*` 清理）。

### AotTests 保留的 AOT 专有用例（PR3 后）

| 文件 | 说明 |
|------|------|
| `TC_PInvoke.cs` + `leanclr_test_pinvoke.js` | Wasm `__Internal` P/Invoke（`#if IL2CPP_ONLY`） |
| `TC_EvalStackNotEmpty.cs` | HL 转换 eval stack |
| `TC_StaticCtorOrder.cs` | 静态构造器继承顺序 |
| `TC_Call_AotInterp.cs` / `TC_CallVir_AotInterp.cs` | `[AotMethod(false)]` AOT/Interp 混编 |
| `TC_ldsflda.cs` | `ldsflda` 指令 |
| `TC_NewMdArray.cs` | 多维数组创建 overflow |
| `TC_MonoPInvokeCallback.cs` | MonoPInvokeCallback 回调 |
| `TC_System_Diagnostics_StopWatch.cs` | Stopwatch 冒烟测试 |

依赖 `AOTDefs` 或 `CoreTests.App` 的测试（委托、反射、CustomAttribute 等）保留在 **CoreTests**（`Shared/Fixtures/`、`Runtime/`），仅由解释器路径执行。

---

## 重构路线图

| PR | 内容 | 状态 |
|----|------|------|
| PR1 | 文档、清理死代码、修复 sln Release 配置 | 已完成 |
| PR2 | SharedTests + 迁移 conv 系列，消除 AotTests 重复 | 已完成 |
| PR3 | AotTests 删除 Test* 镜像，保留 AOT 专有测 | 已完成 |
| PR4 | CoreTests 目录重组 | 已完成 |
| PR5 | ILTests 入 solution + wrapper + basic-tester 加载 | 待做 |

---

## FAQ

### 报错 "Test runner not found"

先构建 basic-tester：`scripts\test\basic-tester\build.bat Debug x64`

### 能否只跑单个测试？

当前 runner 不支持过滤，所有带 `[UnitTest]` 的方法都会执行。

### ILTests 为什么不在 solution 里？

ILTests 尚未接入构建与 runner，计划在 PR5 集成。在此之前，请手动用 ilasm 编译并自行加载。

### Assert 应该写在哪里？

只在 **Common/Assert.cs** 维护一份。不要在各测试项目中复制 Assert 实现。
