#include "runtime/onnx_backend.h"

#include <QDir>

#include <numeric>
#include <stdexcept>

namespace aitoolkit::runtime {

namespace {

std::runtime_error runtimeError(const QString& message) {
    return std::runtime_error(message.toStdString());
}

std::size_t elementCount(const std::vector<int64_t>& shape) {
    if (shape.empty()) {
        return 0;
    }

    std::size_t count = 1;
    for (const int64_t dimension : shape) {
        if (dimension <= 0) {
            throw std::runtime_error("Tensor shape contains a non-positive dimension");
        }
        count *= static_cast<std::size_t>(dimension);
    }
    return count;
}

void validateInputShapeAgainstModel(
    const std::vector<int64_t>& modelShape,
    const std::vector<int64_t>& inputShape) {
    if (modelShape.empty()) {
        return;
    }

    if (modelShape.size() != inputShape.size()) {
        throw std::runtime_error("Input tensor rank does not match the model input rank");
    }

    for (std::size_t index = 0; index < modelShape.size(); ++index) {
        const int64_t modelDimension = modelShape[index];
        const int64_t inputDimension = inputShape[index];
        if (inputDimension <= 0) {
            throw std::runtime_error("Input tensor shape contains a non-positive dimension");
        }
        if (modelDimension > 0 && modelDimension != inputDimension) {
            throw std::runtime_error("Input tensor shape does not match the model's static input dimensions");
        }
    }
}

}  // namespace

OnnxBackend::OnnxBackend(const QString& modelPath)
    : modelPath_(QDir::cleanPath(modelPath)),
      env_(ORT_LOGGING_LEVEL_WARNING, "ai_toolkit_c"),
      sessionOptions_() {
    if (modelPath_.isEmpty()) {
        throw runtimeError(QStringLiteral("ONNX model path must not be empty"));
    }

    sessionOptions_.SetGraphOptimizationLevel(GraphOptimizationLevel::ORT_ENABLE_EXTENDED);
    sessionOptions_.SetIntraOpNumThreads(1);

    try {
        session_ = std::make_unique<Ort::Session>(env_, modelPath_.toStdWString().c_str(), sessionOptions_);
        inputNames_ = readNodeNames(*session_, session_->GetInputCount(), true);
        outputNames_ = readNodeNames(*session_, session_->GetOutputCount(), false);
        inputShape_ = readInputShape(*session_);
    } catch (const Ort::Exception& error) {
        throw runtimeError(
            QStringLiteral("Failed to create ONNX Runtime session for %1: %2")
                .arg(QDir::toNativeSeparators(modelPath_), QString::fromUtf8(error.what())));
    }
}

const QString& OnnxBackend::modelPath() const noexcept {
    return modelPath_;
}

bool OnnxBackend::isLoaded() const noexcept {
    return session_ != nullptr;
}

const std::vector<std::string>& OnnxBackend::inputNames() const noexcept {
    return inputNames_;
}

const std::vector<std::string>& OnnxBackend::outputNames() const noexcept {
    return outputNames_;
}

const std::vector<int64_t>& OnnxBackend::inputShape() const noexcept {
    return inputShape_;
}

std::vector<OnnxTensor> OnnxBackend::run(
    const std::vector<float>& inputData,
    const std::vector<int64_t>& inputShape) const {
    if (!session_) {
        throw std::runtime_error("ONNX Runtime session is not initialized");
    }
    if (inputNames_.empty()) {
        throw std::runtime_error("ONNX Runtime model does not expose any inputs");
    }
    if (inputNames_.size() != 1) {
        throw std::runtime_error("Only single-input ONNX models are supported");
    }

    validateInputShapeAgainstModel(inputShape_, inputShape);

    const std::size_t expectedElementCount = elementCount(inputShape);
    if (inputData.size() != expectedElementCount) {
        throw std::runtime_error("Input tensor element count does not match the supplied shape");
    }

    Ort::MemoryInfo memoryInfo = Ort::MemoryInfo::CreateCpu(OrtArenaAllocator, OrtMemTypeDefault);
    Ort::Value inputTensor = Ort::Value::CreateTensor<float>(
        memoryInfo,
        const_cast<float*>(inputData.data()),
        inputData.size(),
        inputShape.data(),
        inputShape.size());

    std::vector<const char*> inputNamePtrs;
    inputNamePtrs.reserve(inputNames_.size());
    for (const std::string& inputName : inputNames_) {
        inputNamePtrs.push_back(inputName.c_str());
    }

    std::vector<const char*> outputNamePtrs;
    outputNamePtrs.reserve(outputNames_.size());
    for (const std::string& outputName : outputNames_) {
        outputNamePtrs.push_back(outputName.c_str());
    }

    std::vector<Ort::Value> outputValues;
    try {
        outputValues = session_->Run(
            Ort::RunOptions{nullptr},
            inputNamePtrs.data(),
            &inputTensor,
            1,
            outputNamePtrs.data(),
            outputNamePtrs.size());
    } catch (const Ort::Exception& error) {
        throw std::runtime_error(QStringLiteral("ONNX inference failed: %1").arg(QString::fromUtf8(error.what())).toStdString());
    }

    std::vector<OnnxTensor> tensors;
    tensors.reserve(outputValues.size());
    for (std::size_t index = 0; index < outputValues.size(); ++index) {
        Ort::Value& value = outputValues.at(index);
        if (!value.IsTensor()) {
            continue;
        }

        const Ort::TensorTypeAndShapeInfo tensorInfo = value.GetTensorTypeAndShapeInfo();
        if (tensorInfo.GetElementType() != ONNX_TENSOR_ELEMENT_DATA_TYPE_FLOAT) {
            throw std::runtime_error("Only float ONNX output tensors are supported");
        }

        const std::vector<int64_t> shape = tensorInfo.GetShape();
        const std::size_t tensorElementCount = tensorInfo.GetElementCount();
        const float* rawValues = value.GetTensorData<float>();

        OnnxTensor tensor;
        if (index < outputNames_.size()) {
            tensor.name = outputNames_.at(index);
        }
        tensor.shape = shape;
        tensor.values.assign(rawValues, rawValues + tensorElementCount);
        tensors.push_back(std::move(tensor));
    }

    return tensors;
}

std::vector<std::string> OnnxBackend::readNodeNames(
    const Ort::Session& session,
    const std::size_t count,
    const bool readInputs) {
    std::vector<std::string> names;
    names.reserve(count);

    Ort::AllocatorWithDefaultOptions allocator;
    for (std::size_t index = 0; index < count; ++index) {
        Ort::AllocatedStringPtr name = readInputs
            ? session.GetInputNameAllocated(index, allocator)
            : session.GetOutputNameAllocated(index, allocator);
        names.emplace_back(name.get() == nullptr ? "" : name.get());
    }

    return names;
}

std::vector<int64_t> OnnxBackend::readInputShape(const Ort::Session& session) {
    if (session.GetInputCount() == 0) {
        return {};
    }

    const Ort::TypeInfo typeInfo = session.GetInputTypeInfo(0);
    return typeInfo.GetTensorTypeAndShapeInfo().GetShape();
}

}  // namespace aitoolkit::runtime
