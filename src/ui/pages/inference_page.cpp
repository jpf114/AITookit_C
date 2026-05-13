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

    auto* title = new QLabel(QStringLiteral("Inference"), this);
    title->setStyleSheet(QStringLiteral("font-size: 20px; font-weight: 600;"));

    auto* openButton = new QPushButton(QStringLiteral("Choose image"), this);
    runButton_ = new QPushButton(QStringLiteral("Run detection"), this);
    runButton_->setEnabled(false);

    imagePathLabel_ = new QLabel(QStringLiteral("No image selected"), this);
    imagePathLabel_->setObjectName(QStringLiteral("InferenceImagePathLabel"));
    imagePathLabel_->setWordWrap(true);

    connect(openButton, &QPushButton::clicked, this, [this]() {
        const QString path = QFileDialog::getOpenFileName(
            this,
            QStringLiteral("Choose image"),
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
        imagePathLabel_->setText(QStringLiteral("No image selected"));
        return;
    }

    imagePathLabel_->setText(QStringLiteral("Current image: %1").arg(imagePath));
}

void InferencePage::setModelReady(const bool ready) {
    runButton_->setEnabled(ready);
}

}  // namespace aitoolkit::ui
