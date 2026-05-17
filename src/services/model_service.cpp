#include "services/model_service.h"

#include <QDir>
#include <QFileInfo>
#include <QJsonArray>
#include <QJsonObject>

#include "core/json_utils.h"
#include "runtime/backend_registry.h"

#include <stdexcept>

namespace aitoolkit::services {

core::ModelManifest ModelService::loadManifest(const QString& manifestPath) const {
    return core::loadModelManifest(manifestPath);
}

std::unique_ptr<models::InferenceBackend> ModelService::loadModel(const QString& manifestPath) const {
    const core::ModelManifest manifest = loadManifest(manifestPath);
    runtime::BackendPlugin* const backend =
        runtime::BackendRegistry::instance().getBackend(manifest.backendType);
    if (backend == nullptr || !backend->info().isAvailable) {
        throw std::runtime_error(
            QStringLiteral("Backend is not available: %1").arg(manifest.backendType).toStdString());
    }

    if (!backend->supportedTaskTypes().contains(manifest.taskType, Qt::CaseInsensitive)) {
        throw std::runtime_error(
            QStringLiteral("Unsupported task type for backend %1: %2")
                .arg(manifest.backendType, manifest.taskType)
                .toStdString());
    }

    return backend->createModel(manifest, threadCount_, useGPU_);
}

core::ModelManifest ModelService::createManifestFromOnnx(
    const QString& onnxPath,
    const QString& modelName,
    const int inputWidth,
    const int inputHeight,
    const double confidenceThreshold,
    const double nmsThreshold,
    const QStringList& labels) const {
    const QFileInfo onnxInfo(onnxPath);
    if (!onnxInfo.exists()) {
        throw std::runtime_error(QStringLiteral("ONNX file not found: %1").arg(QDir::toNativeSeparators(onnxPath)).toStdString());
    }

    const QDir onnxDir = onnxInfo.absoluteDir();
    const QString manifestFileName = onnxInfo.completeBaseName() + QStringLiteral(".json");
    const QString manifestPath = QDir::cleanPath(onnxDir.filePath(manifestFileName));

    QJsonObject root;
    root.insert(QStringLiteral("name"), modelName);
    root.insert(QStringLiteral("task_type"), QStringLiteral("detection"));
    root.insert(QStringLiteral("backend"), QStringLiteral("onnxruntime"));
    root.insert(QStringLiteral("model"), onnxInfo.fileName());
    root.insert(QStringLiteral("input_width"), inputWidth);
    root.insert(QStringLiteral("input_height"), inputHeight);
    root.insert(QStringLiteral("confidence_threshold"), confidenceThreshold);
    root.insert(QStringLiteral("nms_threshold"), nmsThreshold);

    if (!labels.isEmpty()) {
        QJsonArray labelsArray;
        for (const QString& label : labels) {
            labelsArray.append(label.trimmed());
        }
        root.insert(QStringLiteral("labels_inline"), labelsArray);
    }

    core::writeJsonObject(manifestPath, root);

    return core::loadModelManifest(manifestPath);
}

void ModelService::setThreadCount(const int count) {
    threadCount_ = count;
}

void ModelService::setUseGPU(const bool useGPU) {
    useGPU_ = useGPU;
}

}  // namespace aitoolkit::services
