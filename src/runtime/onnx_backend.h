#pragma once

#include <QString>

#include <onnxruntime_cxx_api.h>

#include <cstdint>
#include <memory>
#include <string>
#include <vector>

#include "runtime/inference_backend.h"

namespace aitoolkit::runtime {

class OnnxBackend {
public:
    explicit OnnxBackend(const QString& modelPath, int threadCount = 1);

    const QString& modelPath() const noexcept;
    bool isLoaded() const noexcept;
    const std::vector<std::string>& inputNames() const noexcept;
    const std::vector<std::string>& outputNames() const noexcept;
    const std::vector<int64_t>& inputShape() const noexcept;
    void warmup();

    std::vector<InferenceTensor> run(
        const std::vector<float>& inputData,
        const std::vector<int64_t>& inputShape) const;

private:
    static std::vector<std::string> readNodeNames(
        const Ort::Session& session,
        std::size_t count,
        bool readInputs);
    static std::vector<int64_t> readInputShape(const Ort::Session& session);

    QString modelPath_;
    Ort::Env env_;
    Ort::SessionOptions sessionOptions_;
    std::unique_ptr<Ort::Session> session_;
    std::vector<std::string> inputNames_;
    std::vector<std::string> outputNames_;
    std::vector<int64_t> inputShape_;
};

}  // namespace aitoolkit::runtime
