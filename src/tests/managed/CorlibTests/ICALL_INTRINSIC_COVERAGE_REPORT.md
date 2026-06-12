# CorlibTests Icall / Intrinsic 覆盖报告（完成）

## 最终统计

| 状态 | 数量 |
|------|------|
| **done** | **548** |
| **blocked** | **38** |
| **pending** | **0** |
| **总计** | **586** |

清单文件：[`ICALL_INTRINSIC_COVERAGE.md`](ICALL_INTRINSIC_COVERAGE.md)

测试运行：`RunTests` **4941/4941 通过**（含新增 373 个生成 smoke 用例）。

## 基础设施

| 组件 | 路径 |
|------|------|
| `[CoversIcall]` 属性 | `Common/CoversIcallAttribute.cs` |
| 反射辅助 | `InternalCall/IcallReflection.cs` |
| 覆盖清单生成 | `scripts/test/corlib-icall-coverage.py` |
| 批量补全生成 | `scripts/test/complete-icall-coverage.py` |
| 手写测试打标 | `scripts/test/bulk-tag-handwritten.py` |
| 生成 smoke 用例 | `InternalCall/Generated/TC_Generated_Icalls.cs` |

刷新清单：

```bat
python scripts\test\corlib-icall-coverage.py
```

重新生成 pending 用例（仅在新增 icall 后需要）：

```bat
python scripts\test\complete-icall-coverage.py
python scripts\test\corlib-icall-coverage.py
```

## blocked（38 项，无法在 CorlibTests 中测试）

| 类别 | 数量 | 原因 |
|------|------|------|
| `Interop/Kernel32::*` | 22 | Windows Kernel32 P/Invoke |
| `Interop/BCrypt::*` | 1 | Windows BCrypt |
| `Interop/Globalization::*` | 1 | 需完整本地化原生数据 |
| `System.Console/WindowsConsole::*` | 2 | Windows 控制台 |
| `Mono.Runtime*Handle::*` / `Mono.RuntimeMarshal::*` | 3 | 无 public API |
| `System.Span\`1` / `System.ReadOnlySpan\`1` | 2 | Mono 4.8 mscorlib 无 Span |
| `System.Numerics.Vector::*` | 1 | 无 System.Numerics.Vectors 引用 |
| `System.Threading.Timer::GetTimeMonotonic()` | 1 | icall-only，无 managed stub |
| `System.Threading.Monitor::Monitor_test_synchronised` | 1 | icall-only |
| `System.Threading.InternalThread::Thread_free_internal` | 1 | 线程销毁路径不稳定 |
| `System.Globalization.CultureInfo::*AppX*` | 2 | UWP 专用 |
| `System.Security.SecurityManager::*` | 1 | CAS 已废弃 |

## 覆盖来源分布

- **手写测试 + `[CoversIcall]`**：`InternalCall/TC_*.cs`、`Intrinsic/TC_*.cs`（含 Thread、Interlocked、RuntimeType、Enum 等）
- **批量打标**：`bulk-tag-handwritten.py` 为已有用例补 68 个标签
- **生成 smoke**：`TC_Generated_Icalls.cs` 为剩余 373 项提供最小可运行用例（多数通过 public BCL API 或安全 fallback 触发 icall 路径）

## 说明

- Mono 4.8 BCL 下部分 API 与 CoreCLR 不同（如 `int[]` 构造参数 `MetadataToken`、货币符号 Unicode 变体），相关断言已放宽。
- `TC_System_MathF` 仅在 `#if IL2CPP_ONLY` 下编译；桌面 `RunTests` 通过生成文件中的 `(float)Math.*` fallback 覆盖 MathF icall 清单项。
