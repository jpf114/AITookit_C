# Changelog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/), and the project adheres to [Semantic Versioning](https://semver.org/).

## [1.2.2] - 2026-05-24

### Added

- COCO 80 类标签单一数据源：`resources/labels/coco80.txt`
- `SECURITY.md` 安全策略与漏洞报告指南
- CMake 模块 `cmake/qt_platform_plugin.cmake`（Qt 插件复制逻辑复用）

### Changed

- 内置 COCO 模型 manifest 改为引用外部标签文件，移除 4×80 行重复 `labels_inline`
- `download_sample_model.ps1` / `test_builtin_models.py` 从标签文件加载 COCO 类别
- GUI 集成测试 CMake 配置去重（`ai_configure_gui_integration_test`）
- 安装包包含 `resources/labels/` 目录

## [1.2.1] - 2026-05-24

### Added

- 完整 `model_catalog.json`（13 款 YOLO 模型，Ultralytics v8.4.0 官方 ONNX）
- CMake 选项 `AI_ENABLE_TENSORRT` / `AI_ENABLE_OPENVINO`（插件骨架）
- 扩展推理后端开发文档、`.clang-format` 配置
- 模型页显示可用推理后端列表
- C++ 模型下载 SHA256 校验（`model_checksum` + 完整 `checksums.json`）
- Catalog / 模型下载 URL 安全校验（HTTPS + GitHub 域名白名单）
- 网络请求 30 秒超时（`network_fetch`、catalog 异步拉取）
- 发布与配置文档：`docs/RELEASING.md`、`docs/CONFIGURATION.md`
- 测试：`test_model_checksum`、`test_catalog_url_validator`、`test_peers` 测试访问器
- 脚本：`refresh_catalog_checksums.ps1`、`check_coverage_threshold.ps1`

### Changed

- TensorRT/OpenVINO 插件错误提示与版本标识更清晰
- README 同步 ONNX 任务类型自动推断说明
- 模型 catalog URL 升级至 Ultralytics v8.4.0（修复 s/m 型号 404）
- 移除不可用的 YOLOv5nu（官方无 ONNX 发布）
- 设置页「检查更新」改为异步，不阻塞 UI
- 模型目录对话框改为本地优先 + 异步远程刷新，移除硬编码 catalog
- PRIVACY/TERMS 版本同步至 v1.2.1；README 修正 vcpkg 全局模式表述
- CI：cppcheck 阻塞、clang-format 检查、覆盖率 60% 阈值门禁
- 推理 Worker / InferenceService 错误提示中文化；删除未使用的 `AppError` 死代码

### Removed

- `hardcodedCatalogEntries()` 硬编码模型目录（约 130 行）
- `yolov5nu` catalog 条目（无官方 ONNX 资产）

## [1.2.0] - 2026-05-24

### Added

- 启动时异步检查更新（24 小时节流，可设置页关闭）
- 关于对话框展示全部推理后端及可用状态
- yolo_v5 / yolo_x 解码器注册测试
- `scripts/sync_checksums_from_catalog.ps1` 从 catalog 同步校验项

### Changed

- 设置页新增「启动时检查更新」选项

## [1.1.0] - 2026-05-24

### Added

- ONNX 导入按文件名自动推断任务类型（`-cls` / `-seg`）
- 设置页可配置模型目录 URL（高级选项）
- 模型目录远程拉取失败时使用本地缓存
- 检查更新时可一键打开 GitHub Release 下载页
- 用户指南补充企业代码签名说明

### Changed

- 导入分类 ONNX 时默认输入尺寸自动调整为 224×224

## [1.0.2] - 2026-05-23

### Added

- 模型 SHA256 校验值（yolov8n/v8s 检测/分类/分割）
- `test_onnx_backend` 真实 ONNX 会话与推理测试
- CI OpenCppCoverage 覆盖率报告（可选 artifact）
- `scripts/update_model_checksums.ps1`、`scripts/run_coverage.ps1`
- 模型目录远程 catalog 拉取（GitHub raw 回退本地/内置）
- 导航、模型、结果、设置页主要控件无障碍名称

### Changed

- UpdateChecker 仓库地址修正为 `jpf114/AITookit_C`
- `app_version.h` 新增 `AITOOLKIT_GITHUB_REPO` 宏

## [1.0.1] - 2026-05-23

### Added

- PRIVACY.md、THIRD_PARTY_NOTICES.md 法律合规文档
- docs/USER_GUIDE.md 用户指南（GPU 设置、FAQ）
- CMake/CI 自动生成 i18n `.qm` 翻译文件
- `AI_ENABLE_CUDA` 编译选项启用 GPU 推理
- CI：E2E 模型验证、打包冒烟测试、cppcheck 静态分析、SBOM 生成
- 模型下载 SHA256 校验基础设施（models/checksums.json）
- AppLogger 结构化本地日志、UpdateChecker 检查更新
- TensorRT / OpenVINO 后端插件骨架
- resources/model_catalog.json 可配置模型目录
- 4 个新测试套件（backend_registry、unicode_io、inference_worker、settings 扩展）
- vcpkg 全局安装模式（`scripts/install_vcpkg_deps.ps1` 最小依赖集）
- TERMS.md 服务条款
- 主要控件无障碍名称（accessibleName）
- CMake 统一版本号（`app_version.h`）

### Changed

- 恢复 vcpkg 全局模式，移除 manifest 清单文件
- 模型目录下载需勾选 AGPL 许可确认
- 关于对话框版权统一为 AIToolkit
- 移除第三方 GitHub 镜像，仅使用官方源
- 大文件读取增加 512MB 上限保护

## [1.0.0] - 2026-05-19

### Added

- 应用图标（.ico + .rc + .qrc），exe 和窗口显示自定义图标
- Windows 版本信息嵌入 exe 属性
- README.md 项目说明文档
- CHANGELOG.md 版本变更记录

### Changed

- 迁移至 vcpkg manifest 模式，添加 builtin-baseline 锁定依赖版本
- vcpkg.json 添加 ICU 依赖，修复 Qt6Core 启动时 ICU DLL 缺失
- 运行时依赖脚本优先使用 manifest 路径（build/vcpkg_installed），回退全局 vcpkg
- VC++ 运行时路径使用 file(TO_CMAKE_PATH) 规范化，修复反斜杠转义问题
- VC++ 运行时 DLL 逐个检查存在性再安装，避免 vcruntime140_2.dll 不存在时安装失败
- main.cpp 在 Windows 上强制设置 QT_QPA_PLATFORM=windows，避免 conda 等环境变量干扰
- 内置 YOLOv8n/v8s 的检测、分类、分割模型及配套清单文件
- 模型页新增 BuiltinModelEntry 结构，支持按任务类型筛选内置模型
- CI 配置移除 VCPKG_ROOT 环境变量设置，适配 manifest 模式
- .gitignore 补充 vcpkg_installed/、*.pt、docs/superpowers/ 等规则

## [0.2.0] - 2026-05-15

### Added

- CUDA Provider 支持，可选 GPU 加速推理，自动回退 CPU
- GPU 推理设置，设置页新增 GPU 复选框，持久化到 QSettings
- 模型目录一键下载，ModelCatalogDialog 展示 4 个内置模型
- 代码去重，提取 InferenceService::runImageFromMat()，净减 60 行重复代码
- 分类模型支持，ClassificationModel + softmax 后处理
- 分割模型支持，SegmentationModel + 掩码后处理
- ModelService 多任务分发，根据 task_type 自动创建检测/分类/分割模型
- 模型管理增强，新增「浏览模型目录」按钮
- 后端动态加载架构，BackendPlugin 接口 + BackendRegistry 注册表 + OnnxRuntimePlugin
- InferenceSummary 扩展，新增 taskType/classifications/segmentations 字段
- InferenceWorker 多任务路由，根据 taskType 自动选择 detect/classify/segment
- 分类结果 UI 展示，Top-K 列表 + 摘要文本动态显示
- 分割结果 UI 展示，图像预览叠加半透明掩码
- 导出服务多任务适配，JSON 和渲染图片支持分类/分割结果
- YOLOX 解码器，注册 yolo_x 到 PostprocessRegistry
- 关于对话框，显示应用名称、版本号和版权信息
- 推理性能监控，状态栏显示延迟、FPS 和总耗时
- 推理线程数配置，设置页新增 1-16 线程数控件

## [0.1.0] - 2026-05-14

### Added

- 修复中文路径，使用 QFile + cv::imdecode 替代 cv::imread
- 修复 NMS 默认值，改为 0.25/0.45
- 修复线程安全，QMutex 保护 + 参数化阈值
- 首次运行引导，首页增加「下载示例模型」按钮
- 自动恢复上次模型，启动时从 QSettings 读取并加载
- 窗口大小/位置持久化，保存/恢复 geometry
- 推理期间关闭确认，重写 closeEvent
- 术语友好化，SpinBox 加 tooltip，术语加中文解释
- 批量/视频结果浏览，QListWidget 结果列表支持切换查看
- 拖放支持，拖放图像/视频/文件夹自动路由
- 键盘快捷键，Ctrl+O/Ctrl+S/Ctrl+Q
- 图像缩放/平移，滚轮缩放 + 拖拽平移 + 双击重置
- 格式扩展，图像加 tiff/webp，视频加 webm/flv，导出加 TIFF
- 大视频预警，超过 1000 帧弹出确认对话框
- CPack/NSIS 打包，NSIS 安装程序 + ZIP 便携包
- CI/CD，GitHub Actions 自动构建 + 测试 + 打包
- 崩溃报告，集成 MiniDump 生成
- vcpkg manifest 模式，依赖版本锁定
- 架构重构，提取 AppController 层
- 推理后端抽象，InferenceBackend 接口 + InferenceTensor
- 默认模型自动发现，首次启动自动扫描 models/ 目录
- 下载后自动加载，无需用户手动选择清单文件
- 快速体验按钮，模型加载后一键推理
- 后处理策略注册表，PostprocessRegistry 根据 decoder 分发
- 推理预热，首次推理前用零张量做 dry run
- 后端能力查询，supportsGPU()/backendName()
- YOLOv5 解码器，注册 yolo_v5 到 PostprocessRegistry
- 应用版本号显示，窗口标题和首页显示版本号
- 模型页信息增强，展示解码器类型和标签列表
- 检测结果按类别过滤，结果页新增类别下拉框
- 批量导出功能，一键导出批量/视频推理结果为 JSON 数组
