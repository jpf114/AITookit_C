#pragma once

#include <QDir>
#include <QImage>
#include <QString>

namespace test_helpers {

inline QString writeImageFile(const QString& directoryPath,
                               const QString& fileName,
                               int width = 64,
                               int height = 64) {
    const QString imagePath = QDir(directoryPath).filePath(fileName);
    QImage image(width, height, QImage::Format_ARGB32);
    image.fill(Qt::red);
    return image.save(imagePath) ? imagePath : QString();
}

}  // namespace test_helpers
