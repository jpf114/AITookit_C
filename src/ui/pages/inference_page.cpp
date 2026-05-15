#include "ui/pages/inference_page.h"

#include <QDoubleSpinBox>
#include <QFileDialog>
#include <QHBoxLayout>
#include <QImage>
#include <QLabel>
#include <QProgressBar>
#include <QPushButton>
#include <QSpinBox>
#include <QVBoxLayout>
#include <QWidget>

#include "ui/widgets/image_preview_widget.h"

namespace aitoolkit::ui {
namespace {

QString readinessText(const bool modelReady, const bool imageReady) {
    if (!modelReady) {
        return QStringLiteral("\u8bf7\u5148\u52a0\u8f7d\u6a21\u578b\u6e05\u5355\u3002");
    }
    if (!imageReady) {
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

    auto* openFolderButton = new QPushButton(QStringLiteral("\u9009\u62e9\u6587\u4ef6\u5939"), this);
    openFolderButton->setObjectName(QStringLiteral("SecondaryButton"));

    auto* openVideoButton = new QPushButton(QStringLiteral("\u9009\u62e9\u89c6\u9891"), this);
    openVideoButton->setObjectName(QStringLiteral("SecondaryButton"));

    auto* maxFramesLabel = new QLabel(QStringLiteral("\u6700\u5927\u5e27\u6570\uff1a"), this);
    maxFramesSpin_ = new QSpinBox(this);
    maxFramesSpin_->setRange(0, 100000);
    maxFramesSpin_->setValue(0);
    maxFramesSpin_->setSpecialValueText(QStringLiteral("\u5168\u90e8"));
    maxFramesSpin_->setToolTip(QStringLiteral("\u8bbe\u7f6e\u4e3a 0 \u8868\u793a\u5904\u7406\u89c6\u9891\u7684\u6240\u6709\u5e27"));

    runButton_ = new QPushButton(QStringLiteral("\u5f00\u59cb\u68c0\u6d4b"), this);
    runButton_->setObjectName(QStringLiteral("PrimaryButton"));
    runButton_->setEnabled(false);

    cancelButton_ = new QPushButton(QStringLiteral("\u53d6\u6d88"), this);
    cancelButton_->setObjectName(QStringLiteral("SecondaryButton"));
    cancelButton_->setVisible(false);

    progressBar_ = new QProgressBar(this);
    progressBar_->setObjectName(QStringLiteral("InferenceProgressBar"));
    progressBar_->setVisible(false);
    progressBar_->setMinimum(0);
    progressBar_->setMaximum(100);
    progressBar_->setTextVisible(true);

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
    actionLayout->addWidget(openFolderButton);
    actionLayout->addWidget(openVideoButton);

    auto* maxFramesRow = new QHBoxLayout();
    maxFramesRow->addWidget(maxFramesLabel);
    maxFramesRow->addWidget(maxFramesSpin_, 1);
    actionLayout->addLayout(maxFramesRow);

    auto* confLabel = new QLabel(QStringLiteral("置信度阈值："), this);
    confSpin_ = new QDoubleSpinBox(this);
    confSpin_->setRange(0.0, 1.0);
    confSpin_->setSingleStep(0.05);
    confSpin_->setDecimals(2);
    confSpin_->setValue(0.25);
    confSpin_->setToolTip(QStringLiteral("低于此置信度的检测结果将被过滤。值越高，保留的结果越少但越准确。"));

    auto* confRow = new QHBoxLayout();
    confRow->addWidget(confLabel);
    confRow->addWidget(confSpin_, 1);
    actionLayout->addLayout(confRow);

    auto* nmsLabel = new QLabel(QStringLiteral("重叠过滤阈值（NMS）："), this);
    nmsSpin_ = new QDoubleSpinBox(this);
    nmsSpin_->setRange(0.0, 1.0);
    nmsSpin_->setSingleStep(0.05);
    nmsSpin_->setDecimals(2);
    nmsSpin_->setValue(0.45);
    nmsSpin_->setToolTip(QStringLiteral("重叠度过高的检测框将被合并。值越低，重叠框越少；值越高，保留更多可能重叠的框。"));

    auto* nmsRow = new QHBoxLayout();
    nmsRow->addWidget(nmsLabel);
    nmsRow->addWidget(nmsSpin_, 1);
    actionLayout->addLayout(nmsRow);

    actionLayout->addWidget(runButton_);
    actionLayout->addWidget(cancelButton_);
    actionLayout->addWidget(progressBar_);
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
    connect(openFolderButton, &QPushButton::clicked, this, [this]() {
        const QString path = QFileDialog::getExistingDirectory(
            this,
            QStringLiteral("\u9009\u62e9\u56fe\u50cf\u6587\u4ef6\u5939"));
        if (!path.isEmpty()) {
            emit folderSelected(path);
        }
    });
    connect(openVideoButton, &QPushButton::clicked, this, [this]() {
        const QString path = QFileDialog::getOpenFileName(
            this,
            QStringLiteral("\u9009\u62e9\u89c6\u9891"),
            QString(),
            QStringLiteral("Videos (*.mp4 *.avi *.mkv *.mov *.wmv)"));
        if (!path.isEmpty()) {
            emit videoSelected(path, maxFramesSpin_->value());
        }
    });
    connect(runButton_, &QPushButton::clicked, this, &InferencePage::runRequested);
    connect(cancelButton_, &QPushButton::clicked, this, &InferencePage::cancelRequested);

    shellLayout->addWidget(title);
    shellLayout->addWidget(lead);
    shellLayout->addLayout(workRow, 1);
}

void InferencePage::setCurrentImagePath(const QString& imagePath) {
    if (imagePath.isEmpty()) {
        hasValidImage_ = false;
        imagePathLabel_->setText(QStringLiteral("\u5f53\u524d\u672a\u9009\u62e9\u56fe\u50cf"));
        previewWidget_->setImage(QImage());
    } else {
        const QImage image(imagePath);
        hasValidImage_ = !image.isNull();

        if (hasValidImage_) {
            static constexpr int kMaxImageDim = 4096;
            static constexpr int kMinImageDim = 32;

            if (image.width() > kMaxImageDim || image.height() > kMaxImageDim) {
                hasValidImage_ = false;
                imagePathLabel_->setText(
                    QStringLiteral("图像尺寸过大（%1×%2），最大支持 %3×%3")
                        .arg(image.width())
                        .arg(image.height())
                        .arg(kMaxImageDim));
            } else if (image.width() < kMinImageDim || image.height() < kMinImageDim) {
                hasValidImage_ = false;
                imagePathLabel_->setText(
                    QStringLiteral("图像尺寸过小（%1×%2），最小支持 %3×%3")
                        .arg(image.width())
                        .arg(image.height())
                        .arg(kMinImageDim));
            } else {
                imagePathLabel_->setText(QStringLiteral("当前图像：%1").arg(imagePath));
            }
        } else {
            imagePathLabel_->setText(QStringLiteral("当前未选择图像"));
        }

        previewWidget_->setImage(hasValidImage_ ? image : QImage());
    }

    readinessLabel_->setText(readinessText(modelReady_, hasValidImage_));
    updateRunButtonState();
}

void InferencePage::setModelReady(const bool ready) {
    modelReady_ = ready;
    readinessLabel_->setText(readinessText(modelReady_, hasValidImage_));
    updateRunButtonState();
}

void InferencePage::setRunning(const bool running) {
    running_ = running;
    runButton_->setVisible(!running);
    cancelButton_->setVisible(running);
    progressBar_->setVisible(running);
    confSpin_->setEnabled(!running);
    nmsSpin_->setEnabled(!running);
    if (running) {
        progressBar_->setValue(0);
    }
    updateRunButtonState();
}

void InferencePage::setProgress(const int current, const int total) {
    if (total > 0) {
        progressBar_->setMaximum(total);
        progressBar_->setValue(current);
    } else {
        progressBar_->setMaximum(0);
        progressBar_->setValue(0);
    }
}

void InferencePage::updateRunButtonState() {
    runButton_->setEnabled(modelReady_ && hasValidImage_ && !running_);
}

void InferencePage::setDefaultThresholds(const double confidence, const double nms) {
    confSpin_->setValue(confidence);
    nmsSpin_->setValue(nms);
}

double InferencePage::confidenceThreshold() const {
    return confSpin_->value();
}

double InferencePage::nmsThreshold() const {
    return nmsSpin_->value();
}

bool InferencePage::isRunning() const {
    return running_;
}

}  // namespace aitoolkit::ui
