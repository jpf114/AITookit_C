#include "models/inference_backend.h"

namespace aitoolkit::models {

QVector<core::ClassificationItem> InferenceBackend::classify(
    const cv::Mat&, double) const {
    return {};
}

QVector<core::SegmentationItem> InferenceBackend::segment(
    const cv::Mat&, double, double) const {
    return {};
}

}  // namespace aitoolkit::models
