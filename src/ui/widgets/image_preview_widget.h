#pragma once

#include <QImage>
#include <QWidget>

#include "core/types.h"

namespace aitoolkit::ui {

class ImagePreviewWidget : public QWidget {
    Q_OBJECT

public:
    explicit ImagePreviewWidget(QWidget* parent = nullptr);

    void setImage(const QImage& image);
    void setSummary(const core::InferenceSummary& summary);

protected:
    void paintEvent(QPaintEvent* event) override;

private:
    QRectF imageTargetRect() const;

    QImage image_;
    core::InferenceSummary summary_;
};

}  // namespace aitoolkit::ui
