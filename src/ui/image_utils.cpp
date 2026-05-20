#include "ui/image_utils.h"

#include <QFile>

#include "core/image_algorithms.h"

namespace aitoolkit::ui {

QImage loadUsableImage(const QString& imagePath) {
    if (imagePath.isEmpty()) {
        return QImage();
    }
    QFile file(imagePath);
    if (!file.open(QIODevice::ReadOnly)) {
        return QImage();
    }
    const QByteArray data = file.readAll();
    if (data.isEmpty()) {
        return QImage();
    }
    return QImage::fromData(data);
}

QImage colorizeMask(const QImage& mask, const QSize& targetSize, QColor color) {
    return core::colorizeMask(mask, targetSize, color);
}

}  // namespace aitoolkit::ui
