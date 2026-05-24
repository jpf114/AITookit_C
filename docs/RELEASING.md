# 发布流程

本文档说明 AIToolkit_C 维护者如何发布新版本。

## 前置条件

- `main` 分支 CI 通过（构建、测试、打包、E2E）
- [`CHANGELOG.md`](../CHANGELOG.md) 已更新
- [`CMakeLists.txt`](../CMakeLists.txt) 中 `project(... VERSION x.y.z)` 已 bump
- 模型 catalog / checksums 如有变更已同步（见下方脚本）

## 发布步骤

1. 合并所有待发布改动到 `main`
2. 确认版本号与 CHANGELOG 一致
3. 创建并推送 tag（必须以 `v` 开头）：

```bash
git tag v1.2.2
git push origin v1.2.2
```

4. GitHub Actions [`build.yml`](../.github/workflows/build.yml) 的 `release` job 会自动：
   - 下载 Release 构建产物（NSIS 安装包 + ZIP + SBOM）
   - 创建 GitHub Release 并上传附件

## 产物命名

| 产物 | 说明 |
|------|------|
| `AIToolkit-<version>-win64.zip` | 便携 ZIP 包 |
| `AIToolkit-<version>-win64.exe` | NSIS 安装程序 |
| `sbom.json` | 软件物料清单 |

## 可选：代码签名

在 GitHub 仓库 Settings → Secrets 中配置 `AI_CODESIGN_THUMBPRINT`（EV 证书 SHA1 指纹），Release 构建会自动对 NSIS 安装包签名。

## 模型与校验和维护

```powershell
# 从 catalog 下载全部模型并刷新 SHA256
powershell -ExecutionPolicy Bypass -File scripts/refresh_catalog_checksums.ps1

# 仅更新已有模型的 checksums.json
powershell -ExecutionPolicy Bypass -File scripts/update_model_checksums.ps1 -ModelsDirectory models
```

## 发布后

- 在 GitHub Release 页面核对附件完整性
- 验证安装包 `--version` 输出正确
- 应用内「检查更新」会在 24 小时内检测到新版本
