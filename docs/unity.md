# Integrated into Unity

## Unity WebGL 构建流程

- 生成ManagedStripped assemblies. 在 `Library/Bee/artifacts/WebGL/ManagedStripped`。
- 单独编译libil2cpp源码。
- 调用 `il2cpp --convert-to-cpp xxx` 生成 cpp代码、符号文件、global-metadata.dat等文件
- 编译生成的cpp文件

替换il2cpp需要Hook这个流程，需要实现以下功能：

- 替换libil2cpp源码为leanclr源码
- 由assembly生成cpp文件
- 生成data文件，由参数`--data-folder=xxx`指定，包含以下文件：
  - `Metadata/global-metadata.dat`
  - `Resouces/mscorlib.dll-resources.dat`
- 生成 symbol文件，由参数 `--symbols-folder=`指定

## Hook实现

### 替换libil2cpp源码

通过设置UNITY_IL2CPP_PATH实现。

在Unity 2022上需要同时满足两个条件：

- 设置 UNITY_IL2CPP_PATH
- 使用 Mono Hook 迫使 GetIl2CppFolder isDevelopmentLocation 参数设置true

### 重定向 il2cpp

- 假设 deploy_dir 为 `il2cpp/build/deploy`
- 复制`leanaot`工具目录到 deploy_dir目录下
- 重命名`il2cpp.exe`为`il2cpp-origin.exe`
- 复制`il2cpp-wrapper.exe`为`il2cpp.exe`

## 实现il2cpp-wrapper工具

重定向`il2cpp convert-to-cpp`命令到`leanaot\LeanAOT.exe`，其余命令仍然调用`il2cpp-origin.exe`

原始命令类似如下：

```bat
D:\workspace\wasmclr\TestWeb\LeanCLR\LocalIl2CppData-WindowsEditor\il2cpp\build\deploy\deploy-2022\il2cpp.exe --convert-to-cpp --assembly=Library/Bee/artifacts/WebGL/ManagedStripped/Assembly-CSharp.dll --assembly=Library/Bee/artifacts/WebGL/ManagedStripped/Mono.Security.dll --assembly=Library/Bee/artifacts/WebGL/ManagedStripped/mscorlib.dll --assembly=Library/Bee/artifacts/WebGL/ManagedStripped/System.Configuration.dll --assembly=Library/Bee/artifacts/WebGL/ManagedStripped/System.Core.dll --assembly=Library/Bee/artifacts/WebGL/ManagedStripped/System.dll --assembly=Library/Bee/artifacts/WebGL/ManagedStripped/System.Xml.dll --assembly=Library/Bee/artifacts/WebGL/ManagedStripped/UnityEngine.AudioModule.dll --assembly=Library/Bee/artifacts/WebGL/ManagedStripped/UnityEngine.CoreModule.dll --assembly=Library/Bee/artifacts/WebGL/ManagedStripped/UnityEngine.dll --assembly=Library/Bee/artifacts/WebGL/ManagedStripped/UnityEngine.PropertiesModule.dll --assembly=Library/Bee/artifacts/WebGL/ManagedStripped/UnityEngine.SharedInternalsModule.dll --generatedcppdir=D:/workspace/wasmclr/TestWeb/Library/Bee/artifacts/WebGL/il2cppOutput/cpp --symbols-folder=D:/workspace/wasmclr/TestWeb/Library/Bee/artifacts/WebGL/il2cppOutput/cpp/Symbols --enable-analytics --emit-null-checks --enable-array-bounds-check --emit-method-map --dotnetprofile=unityaot-linux --profiler-report --profiler-output-file=D:/workspace/wasmclr/TestWeb/Library/Bee/artifacts/il2cpp_conv_6gg8.traceevents --print-command-line --data-folder=D:/workspace/wasmclr/TestWeb/Library/Bee/artifacts/WebGL/il2cppOutput/data
```

当命令过长时，会使用rsp文件传递参数，此时命令行变成这样 `il2cpp.exe @{rsp file}`。

### LeanAOT 额外参数（不改 Unity 命令行）

Unity 传给 `il2cpp.exe` 的实参无法随意扩展时，可在启动 Unity / Bee 之前设置环境变量 **`LEANAOT_EXTRA_ARGS`**（与工具名 **LeanAOT** 一致：`L-E-A-N-A-O-T`，勿漏写 `A`）。值为一段与 rsp 相同的「单行命令」：空白分词，双引号可包住含空格的片段；这些 token **追加**在 Unity 解析出的参数之后，与 Unity 原有实参**合并成同一组** `effectiveArgs`，再交给 `CommandLineParser` 一次解析（与 `--leanaot-aot-rule-file` 等相同，无单独字符串扫描）。

当前支持的 Lean 专有开关示例（均可放在 **`LEANAOT_EXTRA_ARGS`** 中，或直接写在命令行）：

- `--leanaot-aot-rule-file=<path>`：方法 AOT 规则文件（`aot.xml`）；可重复指定多个路径；详见 `docs/aot-rule-file.md`。
- `--leanaot-exclude-assembly-from-global-metadata=<shortName>`：将已在 `-a` / `--assembly`（或 `--directory` 发现）中的程序集**短名**（与 `-a` 一致，无 `.dll` 后缀）从 **`Metadata/global-metadata.dat` 的 COPH 打包中排除**；可重复；不影响 C++ 生成与 `MethodMap.tsv`。仅允许排除当前 AOT 列表中的程序集，否则报错。
- `--leanaot-enable-layout-validation`：开启托管类型布局校验（**默认关闭**；需要校验时在 `LEANAOT_EXTRA_ARGS` 或命令行中显式加上）。

## 实现leanaot工具

需要实现以下功能：

- 由assembly生成cpp文件
- 生成data文件
- 生成 symbol文件

### 生成cpp文件

往 `--generatedcppdir=`目录输出代码。

### 生成data文件

在`--data-folder=xxx`指定目录下生成以下文件：

- `Metadata/global-metadata.dat`
- `Resouces/mscorlib.dll-resources.dat`

`Resouces/mscorlib.dll-resources.dat`目前没有用到，直接生成空文件即可。

LeanAOT 可通过 `--leanaot-exclude-assembly-from-global-metadata`（可经 `LEANAOT_EXTRA_ARGS` 传入）从上述 COPH 中省略部分仍参与 AOT 的程序集短名；见上文「LeanAOT 额外参数」列表。

global-metadata.data 为无压缩的文件bundle格式。格式如下：

`Signature | AssemblyCount | AssemblyInfos | AssemblyContents`。

|字段 | 类型 | 说明|
|-|-|-|
| Signature| `char[4]`| 文件签名, 固定值 为 `COPH`|
| AssemblyCount | int32_t| 程序集个数|
| AssemblyInfos| `AssemblyInfo[AssemblyCount]` |  AssemblyCount个AssemblyInfo，连续排列， AssemblyInfo结构：  name: 程序集名，以`\0`结尾的字符串，如果总长度不是4字节对齐，则填充0直到按4字节对齐； file_length: uint32_t， dll文件长度;  file_offset: uint32_t,  程序集对应的AssemblyContent结构的相对于AssemblyBytes字段的字节偏移值。|
| AssemblyBytes| `AssemblyContent[AssemblyCount]` | 多个AssemblyContent，每个AssemblyContent仅一个字段 `char[AssemblyBytesSize]`， 如果未按4字节对齐，则填充0直至对齐|

### 生成symbol 文件

在 `--symbols-folder=xxx` 目录下生成 `MethodMap.tsv` 文件，文件记录了 每个托管函数原始名和生成的cpp函数名的对应关系。文件内容如下：

```tsv
Array_UnsafeCreateInstance_m74110ACD8ED2B900DFFFC775369DE1D1D4E9C8DB	System.Array System.Array::UnsafeCreateInstance(System.Type,System.Int32[])	mscorlib
Array_CreateInstance_m13B202130951A03AF5F52470A19E17D3AD2A8983	System.Array System.Array::CreateInstance(System.Type,System.Int32)	mscorlib
Array_CreateInstance_m26D0A9871BDB4F6BF311062037068338CA803E97	System.Array System.Array::CreateInstance(System.Type,System.Int32,System.Int32)	mscorlib
Array_CreateInstance_mBF8B23348B319D1FD04F122AB56F0EE8B8825061	System.Array System.Array::CreateInstance(System.Type,System.Int32,System.Int32,System.Int32)	mscorlib
Array_CreateInstance_m97DC551619A43DA8AE15F1A6D33662D66E5DA817	System.Array System.Array::CreateInstance(System.Type,System.Int32[])	mscorlib
Array_CreateInstance_mA9618DC57381BAEA1C1A56C039D253EA70159649	System.Array System.Array::CreateInstance(System.Type,System.Int32[],System.Int32[])	mscorlib
Array_Clear_m50BAA3751899858B097D3FF2ED31F284703FE5CB	System.Void System.Array::Clear(System.Array,System.Int32,System.Int32)	mscorlib
Array_ClearInternal_m8F656BEFDF4E155342F154F6850352B5E0F1F255	System.Void System.Array::ClearInternal(System.Array,System.Int32,System.Int32)	mscorlib
Array_Copy_m4233828B4E6288B6D815F539AAA38575DE627900	System.Void System.Array::Copy(System.Array,System.Array,System.Int32)	mscorlib
Array_Copy_mB4904E17BD92E320613A3251C0205E0786B3BF41	System.Void System.Array::Copy(System.Array,System.Int32,System.Array,System.Int32,System.Int32)	mscorlib
Array_CreateArrayTypeMismatchException_m4A6D7A642A2C8402D0631B4717FE1FD0093C5E7E	System.ArrayTypeMismatchException System.Array::CreateArrayTypeMismatchException()	mscorlib
Array_CanAssignArrayElement_m8BAA866356EC45821E515933FEBC27F02DD2B579	System.Boolean System.Array::CanAssignArrayElement(System.Type,System.Type)	mscorlib
Array_ConstrainedCopy_mE1AE4AB9DAF1DFDF9CB7FB0131B08B7B0DF05EC8	System.Void System.Array::ConstrainedCopy(System.Array,System.Int32,System.Array,System.Int32,System.Int32)	mscorlib
NULL	T[] System.Array::Empty()	mscorlib
Array_Initialize_mA2B5E07BC65B448E268C932D5A686F3A50DB0A82	System.Void System.Array::Initialize()	mscorlib
NULL	System.Int32 System.Array::IndexOfImpl(T[],T,System.Int32,System.Int32)	mscorlib
NULL	System.Int32 System.Array::LastIndexOfImpl(T[],T,System.Int32,System.Int32)	mscorlib

```

格式介绍：

- 每一行为一个映射关系， 格式为 `{cpp method name}\t{managed method name}\t{assembly}`
- 如果某个函数没有生成对应的aot函数，例如是它是internal call函数或者特殊处理的intrinsic或runtime函数，则`{cpp method name}`为 字符串`NULL`

## 替换 il2cpp

- 假设 deploy_dir 为 `il2cpp/build/deploy`
- 复制 deploy_dir 到 `deploy-2022`，将`deploy-2022`放到 deploy_dir目录下
- 复制 `{deploy_dir}/il2cpp.exe` 为 `{deploy_dir}/il2cpp.exe`
- 复制 il2cpp-wrapper.exe 为 `{deploy-dir}/il2cpp.exe`
