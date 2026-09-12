# LeanCLR 文档索引

这个目录只保留当前仍能指导开发和验收的文档。历史迁移记录已经移入 [`archive/`](archive/)，默认阅读时不需要从旧步骤日志里找结论。

## 推荐阅读顺序

| 文档 | 用途 |
| --- | --- |
| [`../README.md`](../README.md) | 项目定位、版本、平台和对外能力概览 |
| [`dotnet10-support-review.md`](dotnet10-support-review.md) | `.NET 10` 当前支持状态、边界和非目标 |
| [`net10-runtime-contract.md`](net10-runtime-contract.md) | `minimal net10 profile` 的 runtime contract、façade 和验收规则 |
| [`dotnet10-runtime-and-engine-integration.md`](dotnet10-runtime-and-engine-integration.md) | `.NET 10` runtime 与 Unity / Godot host bridge 的接入流程 |
| [`dotnet10-runtime-architecture-blog.md`](dotnet10-runtime-architecture-blog.md) | 面向外部读者的架构解释长文 |
| [`../src/tests/TESTING.md`](../src/tests/TESTING.md) | 当前测试命令和验证矩阵 |
| [`../scripts/README.md`](../scripts/README.md) | 脚本目录、构建和清理入口 |

## 文档维护规则

- 主文档写当前事实、架构边界、支持矩阵和验收命令。
- 不在主文档追加逐日迁移记录、单次命令输出或临时排障过程。
- 需要保留的历史分析放进 [`archive/`](archive/)，并在主文档只保留一条摘要链接。
- 新增 runtime contract 时，同时说明来源、LeanCLR 内部映射、非目标边界和对应 gate。
- `.NET 10` 能力描述默认指 `LeanCLR minimal net10 profile`，不要暗示完整 `Microsoft.NETCore.App` 或完整 CoreCLR 行为兼容。

