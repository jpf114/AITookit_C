#pragma once

#include <QColor>
#include <QImage>
#include <QSize>

namespace aitoolkit::core {

QImage colorizeMask(const QImage& mask, const QSize& targetSize, QColor color);

}  // namespace aitoolkit::core
