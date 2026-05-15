#include "ui/image_utils.h"

namespace aitoolkit::ui {

QImage loadUsableImage(const QString& imagePath) {
    if (imagePath.isEmpty()) {
        return QImage();
    }
    const QImage image(imagePath);
    return image.isNull() ? QImage() : image;
}

}  // namespace aitoolkit::ui
