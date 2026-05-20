#include "core/image_algorithms.h"

namespace aitoolkit::core {

QImage colorizeMask(const QImage& mask, const QSize& targetSize, QColor color) {
    QImage alphaMask = mask.scaled(targetSize, Qt::IgnoreAspectRatio, Qt::SmoothTransformation)
                           .convertToFormat(QImage::Format_ARGB32_Premultiplied);
    QImage tinted(alphaMask.size(), QImage::Format_ARGB32_Premultiplied);
    tinted.fill(Qt::transparent);

    for (int y = 0; y < alphaMask.height(); ++y) {
        const QRgb* alphaRow = reinterpret_cast<const QRgb*>(alphaMask.constScanLine(y));
        QRgb* tintedRow = reinterpret_cast<QRgb*>(tinted.scanLine(y));
        for (int x = 0; x < alphaMask.width(); ++x) {
            QColor pixelColor = color;
            pixelColor.setAlpha((qAlpha(alphaRow[x]) * color.alpha()) / 255);
            tintedRow[x] = pixelColor.rgba();
        }
    }

    return tinted;
}

}  // namespace aitoolkit::core
