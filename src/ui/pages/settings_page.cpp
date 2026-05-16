#include "ui/pages/settings_page.h"

#include <QCoreApplication>
#include <QFileDialog>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QMessageBox>
#include <QPushButton>
#include <QSpinBox>
#include <QVBoxLayout>

namespace aitoolkit::ui {

SettingsPage::SettingsPage(QWidget* parent)
    : QWidget(parent) {
    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(24, 24, 24, 24);
    layout->setSpacing(12);

    auto* title = new QLabel(QStringLiteral("设置"), this);
    title->setStyleSheet(QStringLiteral("font-size: 20px; font-weight: 600;"));

    auto* exportLabel = new QLabel(QStringLiteral("默认导出目录"), this);
    auto* exportRow = new QHBoxLayout();
    exportDirectoryEdit_ = new QLineEdit(this);
    exportDirectoryEdit_->setPlaceholderText(QStringLiteral("未设置时默认使用当前图像所在目录"));
    auto* browseButton = new QPushButton(QStringLiteral("浏览"), this);
    browseButton->setObjectName(QStringLiteral("SecondaryButton"));
    exportRow->addWidget(exportDirectoryEdit_, 1);
    exportRow->addWidget(browseButton);

    auto* threadLabel = new QLabel(QStringLiteral("推理线程数"), this);
    threadCountSpin_ = new QSpinBox(this);
    threadCountSpin_->setRange(1, 16);
    threadCountSpin_->setValue(1);
    threadCountSpin_->setToolTip(QStringLiteral("ONNX Runtime 推理使用的 CPU 线程数。增加线程数可利用多核 CPU 加速推理。"));
    auto* threadRow = new QHBoxLayout();
    threadRow->addWidget(threadCountSpin_);
    threadRow->addStretch(1);

    auto* recentModelsLabel = new QLabel(QStringLiteral("最近模型"), this);
    recentModelsList_ = new QListWidget(this);
    recentModelsList_->setObjectName(QStringLiteral("RecentModelsList"));
    recentModelsList_->setMinimumHeight(120);

    auto* recentInputsLabel = new QLabel(QStringLiteral("最近图像"), this);
    recentInputsList_ = new QListWidget(this);
    recentInputsList_->setObjectName(QStringLiteral("RecentInputsList"));
    recentInputsList_->setMinimumHeight(120);

    connect(exportDirectoryEdit_, &QLineEdit::editingFinished, this, [this]() {
        emit defaultExportDirectoryChanged(exportDirectoryEdit_->text().trimmed());
    });
    connect(recentModelsList_, &QListWidget::itemClicked, this, [this](QListWidgetItem* item) {
        if (item != nullptr) {
            emit recentModelActivated(item->text());
        }
    });
    connect(recentInputsList_, &QListWidget::itemClicked, this, [this](QListWidgetItem* item) {
        if (item != nullptr) {
            emit recentInputActivated(item->text());
        }
    });
    connect(browseButton, &QPushButton::clicked, this, [this]() {
        const QString directoryPath = QFileDialog::getExistingDirectory(
            this,
            QStringLiteral("选择默认导出目录"),
            exportDirectoryEdit_->text().trimmed());
        if (!directoryPath.isEmpty()) {
            exportDirectoryEdit_->setText(directoryPath);
            emit defaultExportDirectoryChanged(directoryPath);
        }
    });
    connect(threadCountSpin_, QOverload<int>::of(&QSpinBox::valueChanged),
            this, &SettingsPage::inferenceThreadCountChanged);

    auto* aboutButton = new QPushButton(QStringLiteral("关于"), this);
    aboutButton->setObjectName(QStringLiteral("SecondaryButton"));
    connect(aboutButton, &QPushButton::clicked, this, [this]() {
        QMessageBox::about(
            this,
            QStringLiteral("关于 AI 检测工具"),
            QStringLiteral("<h3>AI 检测工具 v%1</h3>"
                           "<p>基于 ONNX Runtime 的轻量级目标检测桌面应用</p>"
                           "<p>支持 YOLOv5/YOLOv8/YOLOX 等模型</p>"
                           "<p>推理后端：%2</p>"
                           "<p>&copy; 2026 MyProject</p>")
                .arg(QCoreApplication::applicationVersion())
                .arg(QStringLiteral("ONNX Runtime")));
    });

    layout->addWidget(title);
    layout->addWidget(exportLabel);
    layout->addLayout(exportRow);
    layout->addWidget(threadLabel);
    layout->addLayout(threadRow);
    layout->addWidget(recentModelsLabel);
    layout->addWidget(recentModelsList_);
    layout->addWidget(recentInputsLabel);
    layout->addWidget(recentInputsList_, 1);
    layout->addWidget(aboutButton);
}

void SettingsPage::setDefaultExportDirectory(const QString& directoryPath) {
    exportDirectoryEdit_->setText(directoryPath);
}

void SettingsPage::setRecentModels(const QStringList& recentModels) {
    recentModelsList_->clear();
    recentModelsList_->addItems(recentModels);
}

void SettingsPage::setRecentInputs(const QStringList& recentInputs) {
    recentInputsList_->clear();
    recentInputsList_->addItems(recentInputs);
}

void SettingsPage::setInferenceThreadCount(const int count) {
    threadCountSpin_->blockSignals(true);
    threadCountSpin_->setValue(count);
    threadCountSpin_->blockSignals(false);
}

}  // namespace aitoolkit::ui
