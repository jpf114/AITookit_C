#include "ui/pages/inference_page.h"

#include <QFileDialog>
#include <QHBoxLayout>
#include <QImage>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>
#include <QWidget>

#include "ui/widgets/image_preview_widget.h"

namespace aitoolkit::ui {
namespace {

QString readinessText(const bool modelReady, const QString& imagePath) {
    if (!modelReady) {
        return QStringLiteral("\u8bf7\u5148\u52a0\u8f7d\u6a21\u578b\u6e05\u5355\u3002");
    }
    if (imagePath.isEmpty()) {
        return QStringLiteral("\u6a21\u578b\u5df2\u5c31\u7eea\uff0c\u8bf7\u9009\u62e9\u4e00\u5f20\u5f85\u63a8\u7406\u56fe\u50cf\u3002");
    }
    return QStringLiteral("\u6a21\u578b\u548c\u56fe\u50cf\u5df2\u5c31\u7eea\uff0c\u53ef\u4ee5\u5f00\u59cb\u68c0\u6d4b\u3002");
}

}  // namespace

InferencePage::InferencePage(QWidget* parent)
    : QWidget(parent) {
    auto* shellLayout = new QVBoxLayout(this);
    shellLayout->setContentsMargins(24, 24, 24, 24);
    shellLayout->setSpacing(12);

    auto* title = new QLabel(QStringLiteral("\u63a8\u7406"), this);
    title->setStyleSheet(QStringLiteral("font-size: 20px; font-weight: 600;"));

    auto* lead = new QLabel(
        QStringLiteral("\u9009\u62e9\u4e00\u5f20\u56fe\u50cf\uff0c\u5728\u9884\u89c8\u65c1\u5b8c\u6210\u68c0\u6d4b\u64cd\u4f5c\u3002"),
        this);
    lead->setObjectName(QStringLiteral("PageLead"));
    lead->setWordWrap(true);

    previewWidget_ = new ImagePreviewWidget(this);
    previewWidget_->setMinimumSize(560, 360);

    auto* openButton = new QPushButton(QStringLiteral("\u9009\u62e9\u56fe\u50cf"), this);
    openButton->setObjectName(QStringLiteral("SecondaryButton"));

    runButton_ = new QPushButton(QStringLiteral("\u5f00\u59cb\u68c0\u6d4b"), this);
    runButton_->setObjectName(QStringLiteral("PrimaryButton"));
    runButton_->setEnabled(false);

    imagePathLabel_ = new QLabel(QStringLiteral("\u5f53\u524d\u672a\u9009\u62e9\u56fe\u50cf"), this);
    imagePathLabel_->setObjectName(QStringLiteral("InferenceImagePathLabel"));
    imagePathLabel_->setWordWrap(true);

    readinessLabel_ = new QLabel(QStringLiteral("\u8bf7\u5148\u52a0\u8f7d\u6a21\u578b\u6e05\u5355\u3002"), this);
    readinessLabel_->setObjectName(QStringLiteral("InferenceReadinessLabel"));
    readinessLabel_->setWordWrap(true);

    auto* actionPanel = new QWidget(this);
    actionPanel->setObjectName(QStringLiteral("InferenceActionPanel"));

    auto* actionLayout = new QVBoxLayout(actionPanel);
    actionLayout->setContentsMargins(16, 16, 16, 16);
    actionLayout->setSpacing(10);
    actionLayout->addWidget(openButton);
    actionLayout->addWidget(runButton_);
    actionLayout->addWidget(imagePathLabel_);
    actionLayout->addWidget(readinessLabel_);
    actionLayout->addStretch(1);

    auto* workRow = new QHBoxLayout();
    workRow->setSpacing(16);
    workRow->addWidget(previewWidget_, 1);
    workRow->addWidget(actionPanel, 0);

    connect(openButton, &QPushButton::clicked, this, [this]() {
        const QString path = QFileDialog::getOpenFileName(
            this,
            QStringLiteral("\u9009\u62e9\u56fe\u50cf"),
            QString(),
            QStringLiteral("Images (*.png *.jpg *.jpeg *.bmp)"));
        if (!path.isEmpty()) {
            emit imageSelected(path);
        }
    });
    connect(runButton_, &QPushButton::clicked, this, &InferencePage::runRequested);

    shellLayout->addWidget(title);
    shellLayout->addWidget(lead);
    shellLayout->addLayout(workRow, 1);
}

void InferencePage::setCurrentImagePath(const QString& imagePath) {
    setProperty("currentImagePath", imagePath);

    if (imagePath.isEmpty()) {
        imagePathLabel_->setText(QStringLiteral("\u5f53\u524d\u672a\u9009\u62e9\u56fe\u50cf"));
        previewWidget_->setImage(QImage());
    } else {
        imagePathLabel_->setText(QStringLiteral("\u5f53\u524d\u56fe\u50cf\uff1a%1").arg(imagePath));
        previewWidget_->setImage(QImage(imagePath));
    }

    readinessLabel_->setText(readinessText(modelReady_, imagePath));
    runButton_->setEnabled(modelReady_ && !imagePath.isEmpty());
}

void InferencePage::setModelReady(const bool ready) {
    modelReady_ = ready;
    const QString imagePath = property("currentImagePath").toString();
    readinessLabel_->setText(readinessText(modelReady_, imagePath));
    runButton_->setEnabled(modelReady_ && !imagePath.isEmpty());
}

}  // namespace aitoolkit::ui
