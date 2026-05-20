#include "models/yolo_detection_model.h"
#include "models/postprocess_registry.h"

#include <QRectF>

#include <opencv2/imgproc.hpp>

#include <algorithm>
#include <limits>
#include <map>
#include <mutex>
#include <stdexcept>
#include <utility>
#include <vector>

namespace aitoolkit::models {

namespace {

using aitoolkit::core::DetectionItem;
using aitoolkit::core::ModelManifest;

struct CandidateDetection {
    DetectionItem item;
    QRectF box;
};

std::runtime_error modelError(const QString& message) {
    return std::runtime_error(message.toStdString());
}

float intersectionOverUnion(const QRectF& left, const QRectF& right) {
    const QRectF overlap = left.intersected(right);
    if (overlap.isEmpty()) {
        return 0.0f;
    }

    const double intersectionArea = overlap.width() * overlap.height();
    const double unionArea =
        (left.width() * left.height()) +
        (right.width() * right.height()) -
        intersectionArea;

    if (unionArea <= 0.0) {
        return 0.0f;
    }

    return static_cast<float>(intersectionArea / unionArea);
}

QRectF scaleBoxToOriginalImage(
    const float centerX,
    const float centerY,
    const float width,
    const float height,
    const QSize& networkSize,
    const QSize& originalSize) {
    const double xScale = static_cast<double>(originalSize.width()) / static_cast<double>(networkSize.width());
    const double yScale = static_cast<double>(originalSize.height()) / static_cast<double>(networkSize.height());

    const double scaledWidth = width * xScale;
    const double scaledHeight = height * yScale;
    const double left = (centerX - (width * 0.5)) * xScale;
    const double top = (centerY - (height * 0.5)) * yScale;

    return QRectF(left, top, scaledWidth, scaledHeight);
}

QString labelForClassId(const ModelManifest& manifest, const int classId) {
    if (classId >= 0 && classId < manifest.labels.size()) {
        return manifest.labels.at(classId);
    }

    return classId >= 0 ? QStringLiteral("class_%1").arg(classId) : QString();
}

QColor colorForClassId(const int classId) {
    static const QColor palette[] = {
        QColor(239, 68, 68),
        QColor(59, 130, 246),
        QColor(34, 197, 94),
        QColor(249, 115, 22),
        QColor(168, 85, 247),
        QColor(236, 72, 153),
        QColor(20, 184, 166),
        QColor(234, 179, 8),
        QColor(99, 102, 241),
        QColor(244, 63, 94),
    };
    static constexpr int kPaletteSize = sizeof(palette) / sizeof(palette[0]);

    if (classId < 0) {
        return QColor(239, 68, 68);
    }
    return palette[classId % kPaletteSize];
}

std::vector<int> runNms(const std::vector<CandidateDetection>& candidates, const float threshold) {
    std::vector<int> sortedIndices(candidates.size());
    for (int index = 0; index < static_cast<int>(candidates.size()); ++index) {
        sortedIndices[static_cast<std::size_t>(index)] = index;
    }

    std::sort(
        sortedIndices.begin(),
        sortedIndices.end(),
        [&candidates](const int leftIndex, const int rightIndex) {
            return candidates[static_cast<std::size_t>(leftIndex)].item.confidence >
                   candidates[static_cast<std::size_t>(rightIndex)].item.confidence;
        });

    std::vector<int> keptIndices;
    for (const int candidateIndex : sortedIndices) {
        const QRectF& candidateBox = candidates[static_cast<std::size_t>(candidateIndex)].box;
        bool suppressed = false;
        for (const int keptIndex : keptIndices) {
            const QRectF& keptBox = candidates[static_cast<std::size_t>(keptIndex)].box;
            if (intersectionOverUnion(candidateBox, keptBox) > threshold) {
                suppressed = true;
                break;
            }
        }

        if (!suppressed) {
            keptIndices.push_back(candidateIndex);
        }
    }

    return keptIndices;
}

int resolveExpectedAttributes(const int expectedNumClasses) {
    constexpr int kBboxAttributes = 4;
    constexpr int kObjectnessAttribute = 1;

    if (expectedNumClasses < 0) {
        return kBboxAttributes + kObjectnessAttribute;
    }

    return kBboxAttributes + kObjectnessAttribute + expectedNumClasses;
}

bool isRecognizedAttributeCount(const int dimension, const int expectedNumClasses) {
    constexpr int kBboxAttributes = 4;
    constexpr int kObjectnessAttribute = 1;
    constexpr int kCocoClasses = 80;

    if (dimension == kBboxAttributes + kObjectnessAttribute) {
        return true;
    }
    if (dimension == kBboxAttributes + kObjectnessAttribute + kCocoClasses) {
        return true;
    }
    if (dimension == kBboxAttributes) {
        return true;
    }

    if (expectedNumClasses < 0) {
        return false;
    }

    return dimension == resolveExpectedAttributes(expectedNumClasses) ||
           dimension == (kBboxAttributes + expectedNumClasses);
}

}  // namespace

YoloDetectionModel::YoloDetectionModel(ModelManifest manifest, const int threadCount, const bool useGPU)
    : manifest_(std::move(manifest)),
      backend_(manifest_.modelPath, threadCount, useGPU) {
    registerBuiltinDecoders();
    if (manifest_.inputWidth <= 0 || manifest_.inputHeight <= 0) {
        throw modelError(QStringLiteral("YOLO input dimensions must be greater than zero"));
    }
    backend_.warmup();
}

const ModelManifest& YoloDetectionModel::manifest() const noexcept {
    return manifest_;
}

QString YoloDetectionModel::backendName() const noexcept {
    return QStringLiteral("ONNX Runtime");
}

QVector<DetectionItem> YoloDetectionModel::detect(
    const cv::Mat& image,
    const double confidenceThreshold,
    const double nmsThreshold) const {
    const YoloPreprocessResult prepared = preprocessImage(image, manifest_);
    const std::vector<int64_t> inputShape{
        1,
        3,
        static_cast<int64_t>(prepared.networkSize.height()),
        static_cast<int64_t>(prepared.networkSize.width()),
    };

    const std::vector<runtime::InferenceTensor> outputs = backend_.run(prepared.blob, inputShape);

    std::string decoderName = manifest_.decoder.toStdString();
    auto decoder = PostprocessRegistry::instance().getDecoder(decoderName);
    if (!decoder) {
        decoder = PostprocessRegistry::instance().getDecoder("yolo_v8");
    }

    if (!decoder) {
        throw std::runtime_error("No postprocess decoder registered for: " + decoderName);
    }

    PostprocessInput ppInput{
        std::move(outputs),
        prepared.networkSize,
        prepared.originalSize,
        confidenceThreshold < 0 ? manifest_.confidenceThreshold : confidenceThreshold,
        nmsThreshold < 0 ? manifest_.nmsThreshold : nmsThreshold,
        manifest_
    };

    return decoder(ppInput);
}

YoloPreprocessResult YoloDetectionModel::preprocessImage(
    const cv::Mat& image,
    const ModelManifest& manifest) {
    if (image.empty()) {
        throw std::runtime_error("Input image is empty");
    }
    if (manifest.inputWidth <= 0 || manifest.inputHeight <= 0) {
        throw std::runtime_error("YOLO network dimensions must be greater than zero");
    }

    cv::Mat convertedImage;
    switch (image.channels()) {
        case 1:
            cv::cvtColor(image, convertedImage, cv::COLOR_GRAY2RGB);
            break;
        case 3:
            cv::cvtColor(image, convertedImage, cv::COLOR_BGR2RGB);
            break;
        case 4:
            cv::cvtColor(image, convertedImage, cv::COLOR_BGRA2RGB);
            break;
        default:
            throw std::runtime_error("Unsupported channel count for YOLO preprocessing");
    }

    cv::Mat resizedImage;
    cv::resize(
        convertedImage,
        resizedImage,
        cv::Size(manifest.inputWidth, manifest.inputHeight),
        0.0,
        0.0,
        cv::INTER_LINEAR);

    cv::Mat floatImage;
    resizedImage.convertTo(floatImage, CV_32F, 1.0 / 255.0);

    std::vector<cv::Mat> channels;
    cv::split(floatImage, channels);

    YoloPreprocessResult result;
    result.originalSize = QSize(image.cols, image.rows);
    result.networkSize = QSize(manifest.inputWidth, manifest.inputHeight);
    result.blob.resize(
        static_cast<std::size_t>(manifest.inputWidth) *
        static_cast<std::size_t>(manifest.inputHeight) *
        3U);

    std::size_t offset = 0;
    for (const cv::Mat& channel : channels) {
        const std::size_t channelSize = static_cast<std::size_t>(channel.rows * channel.cols);
        std::copy(channel.ptr<float>(), channel.ptr<float>() + channelSize, result.blob.begin() + static_cast<std::ptrdiff_t>(offset));
        offset += channelSize;
    }

    return result;
}

cv::Mat YoloDetectionModel::tensorToDetectionMatrix(
    const runtime::InferenceTensor& tensor,
    const int expectedNumClasses) {
    if (tensor.shape.size() == 2) {
        if (tensor.shape[1] < 5) {
            throw std::runtime_error("YOLO 2D output tensor must contain at least 5 attributes per detection");
        }
        return cv::Mat(
            static_cast<int>(tensor.shape[0]),
            static_cast<int>(tensor.shape[1]),
            CV_32F,
            const_cast<float*>(tensor.values.data())).clone();
    }

    if (tensor.shape.size() == 3) {
        if (tensor.shape[0] != 1) {
            throw std::runtime_error("Only batch size 1 YOLO outputs are supported");
        }

        const int dim1 = static_cast<int>(tensor.shape[1]);
        const int dim2 = static_cast<int>(tensor.shape[2]);
        const bool dim1LooksLikeAttributes = isRecognizedAttributeCount(dim1, expectedNumClasses);
        const bool dim2LooksLikeAttributes = isRecognizedAttributeCount(dim2, expectedNumClasses);

        if (dim1LooksLikeAttributes == dim2LooksLikeAttributes) {
            throw std::runtime_error("Unable to determine whether the YOLO 3D output layout is [1, N, attrs] or [1, attrs, N]");
        }

        if (dim2LooksLikeAttributes) {
            return cv::Mat(
                dim1,
                dim2,
                CV_32F,
                const_cast<float*>(tensor.values.data())).clone();
        }

        return cv::Mat(
            dim1,
            dim2,
            CV_32F,
            const_cast<float*>(tensor.values.data())).t();
    }

    throw std::runtime_error("Unsupported YOLO output tensor shape");
}

QVector<DetectionItem> YoloDetectionModel::postprocessDetections(
    const cv::Mat& output,
    const QSize& networkSize,
    const ModelManifest& manifest,
    const QSize& originalSize,
    const double confidenceThresholdOverride,
    const double nmsThresholdOverride,
    const bool isYoloV8) {
    if (output.empty()) {
        return {};
    }
    if (output.type() != CV_32F) {
        throw std::runtime_error("YOLO output matrix must have CV_32F type");
    }
    if (output.cols < 4) {
        throw std::runtime_error("YOLO output matrix must contain at least 4 columns");
    }
    if (networkSize.width() <= 0 || networkSize.height() <= 0) {
        throw std::runtime_error("Network size must be greater than zero");
    }
    if (originalSize.width() <= 0 || originalSize.height() <= 0) {
        throw std::runtime_error("Original image size must be greater than zero");
    }

    const float confidenceThreshold = static_cast<float>(
        confidenceThresholdOverride >= 0.0 ? confidenceThresholdOverride : manifest.confidenceThreshold);
    const float nmsThreshold = static_cast<float>(
        nmsThresholdOverride >= 0.0 ? nmsThresholdOverride : manifest.nmsThreshold);

    const int bboxCols = 4;
    const int classStartCol = isYoloV8 ? 4 : 5;
    const bool hasObjectness = !isYoloV8;

    std::map<int, std::vector<CandidateDetection>> groupedCandidates;
    for (int row = 0; row < output.rows; ++row) {
        const float* values = output.ptr<float>(row);

        float confidence = 0.0f;
        int classId = -1;
        float classScore = 0.0f;

        if (hasObjectness && output.cols > 5) {
            const float objectness = values[4];
            classScore = -std::numeric_limits<float>::infinity();
            for (int column = classStartCol; column < output.cols; ++column) {
                if (values[column] > classScore) {
                    classScore = values[column];
                    classId = column - classStartCol;
                }
            }
            confidence = objectness * classScore;
        } else if (isYoloV8 && output.cols > 4) {
            classScore = -std::numeric_limits<float>::infinity();
            for (int column = classStartCol; column < output.cols; ++column) {
                if (values[column] > classScore) {
                    classScore = values[column];
                    classId = column - classStartCol;
                }
            }
            confidence = classScore;
        } else {
            confidence = hasObjectness ? values[4] : 1.0f;
        }

        if (confidence < confidenceThreshold) {
            continue;
        }

        CandidateDetection candidate;
        candidate.box = scaleBoxToOriginalImage(
            values[0],
            values[1],
            values[2],
            values[3],
            networkSize,
            originalSize);
        candidate.item.classId = classId;
        candidate.item.label = labelForClassId(manifest, classId);
        candidate.item.confidence = confidence;
        candidate.item.boundingBox = candidate.box;
        candidate.item.renderColor = colorForClassId(classId);
        groupedCandidates[classId].push_back(std::move(candidate));
    }

    QVector<DetectionItem> results;
    for (auto& entry : groupedCandidates) {
        std::vector<CandidateDetection>& candidates = entry.second;
        const std::vector<int> keptIndices = runNms(candidates, nmsThreshold);
        for (const int keptIndex : keptIndices) {
            results.push_back(candidates[static_cast<std::size_t>(keptIndex)].item);
        }
    }

    std::sort(
        results.begin(),
        results.end(),
        [](const DetectionItem& left, const DetectionItem& right) {
            return left.confidence > right.confidence;
        });

    return results;
}

void YoloDetectionModel::registerBuiltinDecoders() {
    static std::once_flag onceFlag;
    std::call_once(onceFlag, []() {
        PostprocessRegistry::instance().registerDecoder("yolo_v8",
            [](const PostprocessInput& input) -> QVector<core::DetectionItem> {
                if (input.tensors.empty()) {
                    return {};
                }
                const int numClasses = static_cast<int>(input.manifest.labels.size());
                const cv::Mat output = YoloDetectionModel::tensorToDetectionMatrix(
                    input.tensors.front(), numClasses);
                return YoloDetectionModel::postprocessDetections(
                    output, input.networkSize, input.manifest,
                    input.originalSize, input.confidenceThreshold, input.nmsThreshold,
                    true);
            });

        PostprocessRegistry::instance().registerDecoder("yolo_v5",
            [](const PostprocessInput& input) -> QVector<core::DetectionItem> {
                if (input.tensors.empty()) {
                    return {};
                }
                const cv::Mat output = YoloDetectionModel::tensorToDetectionMatrix(
                    input.tensors.front());
                return YoloDetectionModel::postprocessDetections(
                    output, input.networkSize, input.manifest,
                    input.originalSize, input.confidenceThreshold, input.nmsThreshold);
            });

        PostprocessRegistry::instance().registerDecoder("yolo_x",
            [](const PostprocessInput& input) -> QVector<core::DetectionItem> {
                if (input.tensors.empty()) {
                    return {};
                }
                const cv::Mat output = YoloDetectionModel::tensorToDetectionMatrix(
                    input.tensors.front());
                return YoloDetectionModel::postprocessDetections(
                    output, input.networkSize, input.manifest,
                    input.originalSize, input.confidenceThreshold, input.nmsThreshold);
            });
    });
}

}  // namespace aitoolkit::models
