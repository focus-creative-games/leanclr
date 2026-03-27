# Integrated into Unity

## Editor设置

- 设置 UNITY_IL2CPP_PATH
- 使用 Mono Hook 迫使 GetIl2CppFolder isDevelopmentLocation 参数设置true

## Local Il2cpp目录

- 假设 deploy_dir 为 `il2cpp/build/deploy`
- 复制 deploy_dir 到 `deploy-2022`，将`deploy-2022`放到 deploy_dir目录下
- 复制 `{deploy_dir}/il2cpp.exe` 为 `{deploy_dir}/il2cpp.exe`
- 复制 il2cpp-wrapper.exe 为 `{deploy-dir}/il2cpp.exe`

## 修改 {deplay-dir} 下的 Bee.Toolchain.GNU

修改类Bee.Toolchain.GNU.GccLikeCompilerSettings.`<FlagsForLanguageVersion>`d__62.MoveNext

新增

```

			case CppLanguageVersion.Default:
				yield return "-std=" + str + "17";
				break;
			case CppLanguageVersion.Cpp11:
				yield return "-std=" + str + "11";
				break;
```