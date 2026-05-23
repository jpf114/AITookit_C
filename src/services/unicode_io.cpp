#include "services/unicode_io.h"

#include <QFile>

#include <opencv2/imgcodecs.hpp>
#include <opencv2/videoio.hpp>

#ifdef Q_OS_WIN
#include <Windows.h>
#endif

namespace aitoolkit::services {

namespace {

#ifdef Q_OS_WIN
QString toShortPath(const QString& longPath) {
    const std::wstring wLongPath = longPath.toStdWString();
    const DWORD length = GetShortPathNameW(wLongPath.c_str(), nullptr, 0);
    if (length == 0) {
        return longPath;
    }

    std::wstring shortPath(length, L'\0');
    if (GetShortPathNameW(wLongPath.c_str(), &shortPath[0], length) == 0) {
        return longPath;
    }

    shortPath.resize(std::wcslen(shortPath.c_str()));
    return QString::fromStdWString(shortPath);
}
#endif

}

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

    constexpr qint64 kMaxReadBytes = 512LL * 1024 * 1024;
    if (file.size() > kMaxReadBytes) {
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
    cv::VideoCapture capture;
    if (!openVideoCapture(videoPath, capture)) {
        return -1;
    }
    const int count = static_cast<int>(capture.get(cv::CAP_PROP_FRAME_COUNT));
    capture.release();
    return count;
}

bool openVideoCapture(const QString& videoPath, cv::VideoCapture& capture) {
#ifdef Q_OS_WIN
    const QString shortPath = toShortPath(videoPath);
    capture.open(shortPath.toUtf8().constData());
#else
    capture.open(videoPath.toStdString());
#endif
    return capture.isOpened();
}

}  // namespace aitoolkit::services
