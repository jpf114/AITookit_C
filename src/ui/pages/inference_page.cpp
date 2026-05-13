#include "ui/pages/inference_page.h"

#include <QFileDialog>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>

namespace aitoolkit::ui {

InferencePage::InferencePage(QWidget* parent)
    : QWidget(parent) {
    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(24, 24, 24, 24);
    layout->setSpacing(12);

    auto* title = new QLabel(QStringLiteral("单图推理"), this);
    title->setStyleSheet(QStringLiteral("font-size: 20px; font-weight: 600;"));

    auto* openButton = new QPushButton(QStringLiteral("选择图片"), this);
    runButton_ = new QPushButton(QStringLiteral("运行检测"), this);
    runButton_->setEnabled(false);

    imagePathLabel_ = new QLabel(QStringLiteral("当前未选择图片"), this);
    imagePathLabel_->setWordWrap(true);

    connect(openButton, &QPushButton::clicked, this, [this]() {
        const QString path = QFileDialog::getOpenFileName(
            this,
            QStringLiteral("选择图片"),
            QString(),
            QStringLiteral("Images (*.png *.jpg *.jpeg *.bmp)"));
        if (!path.isEmpty()) {
            emit imageSelected(path);
        }
    });
    connect(runButton_, &QPushButton::clicked, this, &InferencePage::runRequested);

    layout->addWidget(title);
    layout->addWidget(openButton);
    layout->addWidget(runButton_);
    layout->addWidget(imagePathLabel_);
    layout->addStretch(1);
}

void InferencePage::setCurrentImagePath(const QString& imagePath) {
    if (imagePath.isEmpty()) {
        imagePathLabel_->setText(QStringLiteral("当前未选择图片"));
        return;
    }

    imagePathLabel_->setText(QStringLiteral("当前图片：%1").arg(imagePath));
}

void InferencePage::setModelReady(const bool ready) {
    runButton_->setEnabled(ready);
}

}  // namespace aitoolkit::ui
