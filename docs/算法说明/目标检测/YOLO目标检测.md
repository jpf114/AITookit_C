# 目标检测 / YOLO 目标检测

## 功能说明

基于 YOLO（You Only Look Once）系列模型执行目标检测推理，将输入图像送入模型后输出检测框、类别标签和置信度。

## 输入与输出

- 输入：RGB 图像（PNG / JPG / BMP / TIFF / WebP）+ ONNX 模型文件
- 输出：检测框列表（类别 ID、标签、置信度、边界框坐标）

## 参数

### 模型清单参数 (ModelManifest)

| 参数 | 类型 | 必填 | 默认值 | 说明 |
| --- | --- | --- | --- | --- |
| name | string | 是 | — | 模型名称 |
| task_type | string | 是 | — | 任务类型，检测模型为 `detection` |
| backend | string | 是 | — | 推理后端，当前仅支持 `onnxruntime` |
| model | string | 是 | — | ONNX 模型文件路径（相对于清单文件） |
| input_width | int | 是 | — | 模型输入宽度，范围 32–4096，步长 32 |
| input_height | int | 是 | — | 模型输入高度，范围 32–4096，步长 32 |
| confidence_threshold | double | 否 | `0.25` | 置信度过滤阈值，范围 0.0–1.0 |
| nms_threshold | double | 否 | `0.45` | NMS IoU 阈值，范围 0.0–1.0 |
| labels | string[] | 否 | `[]` | 类别标签列表 |
| labels_inline | string[] | 否 | `[]` | 内联标签列表（优先于 labels 文件） |
| decoder | string | 否 | `yolo_v8` | 后处理解码器：`yolo_v8`、`yolo_v5`、`yolo_x` |

## 算法内容

### 预处理

将输入图像转换为模型所需的 NCHW 张量格式。处理步骤：

1. **通道转换**：将 BGR（OpenCV 默认）转换为 RGB；单通道灰度图转 RGB；四通道 BGRA 转 RGB。
2. **空间缩放**：将图像 Resize 到 `input_width × input_height`，使用双线性插值。
3. **归一化**：将像素值从 `[0, 255]` 归一化到 `[0.0, 1.0]`，公式为 `pixel / 255.0`。
4. **通道分离**：将 HWC 布局转为 NCHW 布局，按 R、G、B 顺序排列通道数据。

预处理输出为形状 `[1, 3, input_height, input_width]` 的 float32 张量。

### 推理

通过 ONNX Runtime 执行模型前向计算：

1. `OnnxBackend` 加载 ONNX 模型文件，创建推理会话。
2. 将预处理后的张量送入模型输入节点。
3. 输出形状为 `[N, attrs]` 或 `[1, N, attrs]` / `[1, attrs, N]` 的检测张量。

输出张量中 `attrs` 列的含义：

| 列索引 | 含义 |
|--------|------|
| 0 | 中心点 X 坐标（cx） |
| 1 | 中心点 Y 坐标（cy） |
| 2 | 边界框宽度（w） |
| 3 | 边界框高度（h） |
| 4 | 目标置信度（objectness） |
| 5+ | 各类别得分（class scores） |

### 后处理

1. **置信度过滤**：对每行检测，计算组合置信度 `confidence = objectness × max(class_scores)`，低于 `confidence_threshold` 的丢弃。
2. **类别判定**：取最大类别得分对应的类别 ID 作为该检测的类别。
3. **坐标缩放**：将网络坐标按 `x_scale = original_width / network_width`、`y_scale = original_height / network_height` 缩放到原图尺寸。
4. **NMS 去重**：按类别分组执行非极大值抑制，IoU 阈值为 `nms_threshold`。详见 [NMS 非极大值抑制](./NMS非极大值抑制.md)。
5. **排序输出**：按置信度降序排列所有保留的检测结果。

## 处理流程

1. 加载模型：`ModelService::loadModel()` 根据 `task_type` 自动创建 `YoloDetectionModel`。
2. 预处理：`YoloDetectionModel::preprocessImage()` 将图像转为 NCHW 张量。
3. 推理：`OnnxBackend::run()` 执行 ONNX 模型前向计算。
4. 张量解析：`YoloDetectionModel::tensorToDetectionMatrix()` 将输出张量转为检测矩阵。
5. 后处理：通过 `PostprocessRegistry` 根据 `decoder` 字段分发到对应解码器，执行置信度过滤 + NMS + 坐标缩放。
6. 结果封装：`InferenceService::runImage()` 封装为 `InferenceSummary`。

## 输出说明

每个检测结果包含以下字段：

| 字段 | 类型 | 说明 |
| --- | --- | --- |
| classId | int | 类别编号（从 0 开始） |
| label | string | 类别标签（未配置时为 `class_N`） |
| confidence | float | 组合置信度（0.0–1.0） |
| boundingBox | QRectF | 原图坐标系下的边界框（x, y, width, height） |

## 结果解释

- `confidence` 为目标置信度与类别得分的乘积
- `boundingBox` 坐标已缩放到原图尺寸
- 若模型清单未配置标签，`label` 字段为 `class_N` 格式
- 检测结果按置信度从高到低排列

## 适用场景

- 单张图像的目标检测
- 文件夹批量图像检测
- 视频逐帧目标检测

## 推荐输出文件名

- JSON 导出：`xxx_detections.json`
- 渲染图导出：`xxx_rendered.png`

## 注意事项

- 模型必须为 ONNX 格式，输入为 NCHW 格式
- `confidence_threshold` 和 `nms_threshold` 必须与模型训练时的设定匹配
- 输入图像尺寸范围：32×32 至 4096×4096
- 当前仅支持单输入 ONNX 模型
- ONNX Runtime 在 Windows Debug 模式下可能存在兼容性问题，建议使用 Release 模式
