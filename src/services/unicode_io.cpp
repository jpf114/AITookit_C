#include "services/unicode_io.h"

#include <QFile>

#include <opencv2/imgcodecs.hpp>
#include <opencv2/videoio.hpp>

namespace aitoolkit::services {

cv::Mat imreadUnicode(const QString& filePath, const int flags) {
    const std::vector<char> buffer = readFileToBuffer(filePath);
    if (buffer.empty()) {
        return {};
    }
    return cv::imdecode(buffer, flags);
}

std::vector<char> readFileToBuffer(const QString& filePath) {
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        return {};
    }

    const QByteArray data = file.readAll();
    file.close();

    if (data.isEmpty()) {
        return {};
    }

    return std::vector<char>(data.constData(), data.constData() + data.size());
}

int probeVideoFrameCount(const QString& videoPath) {
    cv::VideoCapture capture(videoPath.toStdString());
    if (!capture.isOpened()) {
        return -1;
    }
    const int count = static_cast<int>(capture.get(cv::CAP_PROP_FRAME_COUNT));
    capture.release();
    return count;
}

}  // namespace aitoolkit::services
