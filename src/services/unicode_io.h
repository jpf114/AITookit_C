#pragma once

#include <QString>

#include <opencv2/core.hpp>

#include <vector>

namespace aitoolkit::services {

cv::Mat imreadUnicode(const QString& filePath, int flags);

std::vector<char> readFileToBuffer(const QString& filePath);

}  // namespace aitoolkit::services
