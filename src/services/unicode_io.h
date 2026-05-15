#pragma once

#include <QString>

#include <opencv2/core.hpp>
#include <opencv2/videoio.hpp>

#include <vector>

namespace aitoolkit::services {

cv::Mat imreadUnicode(const QString& filePath, int flags);

std::vector<char> readFileToBuffer(const QString& filePath);

int probeVideoFrameCount(const QString& videoPath);

bool openVideoCapture(const QString& videoPath, cv::VideoCapture& capture);

}  // namespace aitoolkit::services
