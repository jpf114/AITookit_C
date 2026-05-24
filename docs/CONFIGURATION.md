# 配置参考

本文档汇总构建时 CMake 选项、环境变量与运行时 QSettings 键。

## 构建时环境变量

| 变量 | 必需 | 说明 |
|------|------|------|
| `VCPKG_ROOT` | 是 | vcpkg 安装根目录（全局模式） |
| `AI_CODESIGN_THUMBPRINT` | 否 | CI 代码签名证书指纹（Secret） |

## CMake 选项

| 选项 | 默认 | 说明 |
|------|------|------|
| `AI_ENABLE_CUDA` | OFF | 启用 ONNX Runtime CUDA GPU 推理 |
| `AI_ENABLE_TENSORRT` | OFF | 编译 TensorRT 插件骨架（需 SDK 才能完成推理） |
| `AI_ENABLE_OPENVINO` | OFF | 编译 OpenVINO 插件骨架（需 SDK 才能完成推理） |
| `USE_STATIC_QT` | OFF | 静态链接 Qt（需注意 LGPL 合规） |
| `VCPKG_MANIFEST_MODE` | OFF | 强制 vcpkg 全局模式 |

示例：

```bash
cmake --preset release -DAI_ENABLE_CUDA=ON
```

## 运行时 QSettings

组织名：`AIToolkit`，应用名：`AIToolkit`（Windows 注册表）

| 键 | 类型 | 默认值 | 说明 |
|----|------|--------|------|
| `recent/models` | QStringList | 空 | 最近使用的模型清单路径 |
| `recent/inputs` | QStringList | 空 | 最近使用的输入路径 |
| `export/defaultDirectory` | QString | 空 | 默认导出目录 |
| `state/lastModelManifest` | QString | 空 | 上次加载的模型清单 |
| `state/windowGeometry` | QByteArray | 空 | 窗口位置与大小 |
| `inference/threadCount` | int | 1 | ONNX Runtime CPU 线程数 |
| `inference/useGPU` | bool | false | 是否使用 GPU（CUDA） |
| `language` | QString | 空 | 界面语言（`zh_CN` / `en`，空=跟随系统） |
| `catalog/modelUrl` | QString | 空 | 自定义模型目录 URL（HTTPS + GitHub 域名） |
| `updates/checkOnStartup` | bool | true | 启动时检查更新 |
| `updates/lastCheckEpochMs` | qint64 | 0 | 上次更新检查时间戳 |

## 模型目录 URL 安全策略

- 必须为 **HTTPS**
- Catalog 主机仅允许 `github.com` / `*.githubusercontent.com`
- 模型下载 URL 仅允许 `github.com`（Ultralytics 官方 release）
- 下载完成后按 [`models/checksums.json`](../models/checksums.json) 校验 SHA256

## 脚本索引

| 脚本 | 用途 |
|------|------|
| `scripts/install_vcpkg_deps.ps1` | 首次安装 vcpkg 依赖 |
| `scripts/download_sample_model.ps1` | UI「下载示例模型」/ 模型目录下载调用 |
| `scripts/download_builtin_models.ps1` | CI E2E 最小模型集（catalog + manifest 模板） |
| `scripts/model_manifest.ps1` | 共享模型 manifest 生成（catalog 字段 → JSON） |
| `scripts/refresh_catalog_checksums.ps1` | 下载 catalog 全部模型并刷新校验和 |
| `scripts/update_model_checksums.ps1` | 根据本地 ONNX 更新 checksums.json |
| `scripts/sync_checksums_from_catalog.ps1` | 从 catalog 同步校验项名称 |
| `scripts/run_coverage.ps1` | 本地覆盖率报告 |
| `scripts/test_builtin_models.py` | CI E2E 模型验证 |
| `scripts/benchmark_models.py` | 本地推理性能基准（warmup + mean/p50/p95） |
| `scripts/generate_sbom.py` | 生成 SBOM |

## NSIS 静默安装（企业部署）

```powershell
AIToolkit-1.2.1-win64.exe /S
```

静默安装后可通过「应用和功能」卸载。安装路径默认为 `%ProgramFiles%\AIToolkit`。
