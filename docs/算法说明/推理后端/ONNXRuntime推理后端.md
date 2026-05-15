# 推理后端 / ONNX Runtime 推理后端

## 功能说明

基于 ONNX Runtime C++ API 加载 ONNX 模型文件并执行推理计算，提供统一的模型会话管理和张量输入输出接口。

## 输入与输出

- 输入：ONNX 模型文件路径 + float32 输入张量（NCHW 布局）
- 输出：float32 输出张量列表

## 参数

### 会话参数

| 参数 | 类型 | 默认值 | 说明 |
| --- | --- | --- | --- |
| graph_optimization_level | 枚举 | `ORT_ENABLE_EXTENDED` | 图优化级别 |
| intra_op_num_threads | int | `1` | 算子内并行线程数 |
| log_level | 枚举 | `ORT_LOGGING_LEVEL_WARNING` | 日志级别 |

## 算法内容

### 模型加载

1. 创建 ONNX Runtime 环境（`Ort::Env`），设置日志级别为 WARNING。
2. 配置会话选项（`Ort::SessionOptions`）：
   - 图优化级别：`ORT_ENABLE_EXTENDED`（扩展优化，包含算子融合、常量折叠等）
   - 算子内线程数：1（单线程推理，避免线程竞争）
3. 创建推理会话（`Ort::Session`），加载模型文件。
4. 读取模型输入/输出节点名称和输入形状信息。

### 推理执行

1. **输入验证**：
   - 检查会话是否已初始化
   - 检查模型是否为单输入
   - 验证输入张量形状与模型期望一致
   - 验证输入数据元素数量与形状匹配

2. **创建输入张量**：
   - 使用 `Ort::Value::CreateTensor<float>()` 创建输入张量
   - 内存分配器：`OrtArenaAllocator` + `OrtMemTypeDefault`
   - 数据不拷贝，直接引用输入缓冲区

3. **执行推理**：
   - 调用 `Ort::Session::Run()` 执行模型前向计算
   - 输入节点名称和输出节点名称通过 C 字符串指针传递

4. **解析输出**：
   - 遍历输出值列表
   - 仅处理 float 类型的张量输出
   - 读取张量形状和元素值
   - 封装为 `InferenceTensor` 结构体

### 输入形状验证

验证规则：
- 模型输入形状与用户提供的输入形状的秩（维度数）必须一致
- 对于模型中的静态维度（> 0），用户提供的对应维度必须匹配
- 对于模型中的动态维度（= -1），用户可提供任意正值

## 处理流程

1. 构造 `OnnxBackend`：加载模型文件，创建推理会话。
2. 准备输入数据：float32 向量 + 形状向量。
3. 调用 `OnnxBackend::run()`：验证 → 创建张量 → 执行推理 → 解析输出。
4. 获取输出张量列表。

## 数据结构

### InferenceTensor

| 字段 | 类型 | 说明 |
| --- | --- | --- |
| name | std::string | 输出节点名称 |
| shape | std::vector\<int64_t\> | 张量形状 |
| values | std::vector\<float\> | 张量数据（行优先） |

## 输出说明

- 输出为 `std::vector<InferenceTensor>`，每个元素对应一个模型输出节点
- 仅包含 float 类型的张量输出
- 张量数据按行优先顺序存储

## 适用场景

- YOLO 系列模型的 ONNX 推理
- 任何导出为 ONNX 格式的单输入 float32 模型

## 注意事项

- 当前仅支持单输入模型（`inputNames_.size() == 1`）
- 当前仅支持 float32 类型的输出张量
- 输入数据不进行拷贝，调用期间需保证输入缓冲区有效
- ONNX Runtime 在 Windows Debug 模式下可能存在兼容性问题，建议使用 Release 模式
- 模型文件路径支持中文路径（通过 `toStdWString()` 转换）
