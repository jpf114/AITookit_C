#include "services/model_service.h"

#include <QDir>
#include <QFileInfo>
#include <QJsonArray>
#include <QJsonObject>

#include "core/json_utils.h"
#include "models/classification_model.h"
#include "models/segmentation_model.h"
#include "models/yolo_detection_model.h"

#include <stdexcept>

namespace aitoolkit::services {

core::ModelManifest ModelService::loadManifest(const QString& manifestPath) const {
    return core::loadModelManifest(manifestPath);
}

std::unique_ptr<models::InferenceBackend> ModelService::loadModel(const QString& manifestPath) const {
    const core::ModelManifest manifest = loadManifest(manifestPath);

    if (manifest.backendType.compare(QStringLiteral("onnxruntime"), Qt::CaseInsensitive) != 0) {
        throw std::runtime_error("Only onnxruntime backend is currently supported");
    }

    if (manifest.taskType.compare(QStringLiteral("detection"), Qt::CaseInsensitive) == 0) {
        return std::make_unique<models::YoloDetectionModel>(manifest, threadCount_);
    }

    if (manifest.taskType.compare(QStringLiteral("classification"), Qt::CaseInsensitive) == 0) {
        return std::make_unique<models::ClassificationModel>(manifest, threadCount_);
    }

    if (manifest.taskType.compare(QStringLiteral("segmentation"), Qt::CaseInsensitive) == 0) {
        return std::make_unique<models::SegmentationModel>(manifest, threadCount_);
    }

    throw std::runtime_error(
        QStringLiteral("Unsupported task type: %1").arg(manifest.taskType).toStdString());
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

}  // namespace aitoolkit::services
