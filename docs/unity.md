# Integrated into Unity

## Editor设置

- 设置 UNITY_IL2CPP_PATH
- 使用 Mono Hook 迫使 GetIl2CppFolder isDevelopmentLocation 参数设置true

## 重定向il2cpp

重定向il2cpp convert-to-cpp命令。

原始命令类似如下：

```bat
D:\workspace\wasmclr\TestWeb\LeanCLR\LocalIl2CppData-WindowsEditor\il2cpp\build\deploy\deploy-2022\il2cpp.exe --convert-to-cpp --assembly=Library/Bee/artifacts/WebGL/ManagedStripped/Assembly-CSharp.dll --assembly=Library/Bee/artifacts/WebGL/ManagedStripped/Mono.Security.dll --assembly=Library/Bee/artifacts/WebGL/ManagedStripped/mscorlib.dll --assembly=Library/Bee/artifacts/WebGL/ManagedStripped/System.Configuration.dll --assembly=Library/Bee/artifacts/WebGL/ManagedStripped/System.Core.dll --assembly=Library/Bee/artifacts/WebGL/ManagedStripped/System.dll --assembly=Library/Bee/artifacts/WebGL/ManagedStripped/System.Xml.dll --assembly=Library/Bee/artifacts/WebGL/ManagedStripped/UnityEngine.AudioModule.dll --assembly=Library/Bee/artifacts/WebGL/ManagedStripped/UnityEngine.CoreModule.dll --assembly=Library/Bee/artifacts/WebGL/ManagedStripped/UnityEngine.dll --assembly=Library/Bee/artifacts/WebGL/ManagedStripped/UnityEngine.PropertiesModule.dll --assembly=Library/Bee/artifacts/WebGL/ManagedStripped/UnityEngine.SharedInternalsModule.dll --generatedcppdir=D:/workspace/wasmclr/TestWeb/Library/Bee/artifacts/WebGL/il2cppOutput/cpp --symbols-folder=D:/workspace/wasmclr/TestWeb/Library/Bee/artifacts/WebGL/il2cppOutput/cpp/Symbols --enable-analytics --emit-null-checks --enable-array-bounds-check --emit-method-map --dotnetprofile=unityaot-linux --profiler-report --profiler-output-file=D:/workspace/wasmclr/TestWeb/Library/Bee/artifacts/il2cpp_conv_6gg8.traceevents --print-command-line --data-folder=D:/workspace/wasmclr/TestWeb/Library/Bee/artifacts/WebGL/il2cppOutput/data
```

当命令过长时，会使用rsp文件传递参数，此时命令行变成这样 `il2cpp.exe @{rsp file}`。


## 替换 il2cpp

- 假设 deploy_dir 为 `il2cpp/build/deploy`
- 复制 deploy_dir 到 `deploy-2022`，将`deploy-2022`放到 deploy_dir目录下
- 复制 `{deploy_dir}/il2cpp.exe` 为 `{deploy_dir}/il2cpp.exe`
- 复制 il2cpp-wrapper.exe 为 `{deploy-dir}/il2cpp.exe`

