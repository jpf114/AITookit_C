#include "ui/pages/settings_page.h"

#include <QCheckBox>
#include <QComboBox>
#include <QCoreApplication>
#include <QFileDialog>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QFileInfo>
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

    auto* title = new QLabel(tr("设置"), this);
    title->setStyleSheet(QStringLiteral("font-size: 20px; font-weight: 600;"));

    auto* exportLabel = new QLabel(tr("默认导出目录"), this);
    auto* exportRow = new QHBoxLayout();
    exportDirectoryEdit_ = new QLineEdit(this);
    exportDirectoryEdit_->setPlaceholderText(tr("未设置时默认使用当前图像所在目录"));
    auto* browseButton = new QPushButton(tr("浏览"), this);
    browseButton->setObjectName(QStringLiteral("SecondaryButton"));
    exportRow->addWidget(exportDirectoryEdit_, 1);
    exportRow->addWidget(browseButton);

    auto* threadLabel = new QLabel(tr("推理线程数"), this);
    threadCountSpin_ = new QSpinBox(this);
    threadCountSpin_->setRange(1, 16);
    threadCountSpin_->setValue(1);
    threadCountSpin_->setToolTip(tr("ONNX Runtime 推理使用的 CPU 线程数。增加线程数可利用多核 CPU 加速推理。"));
    auto* threadRow = new QHBoxLayout();
    threadRow->addWidget(threadCountSpin_);
    threadRow->addStretch(1);

    gpuCheckBox_ = new QCheckBox(tr("使用 GPU 加速推理（CUDA）"), this);
    gpuCheckBox_->setToolTip(tr("需要 NVIDIA GPU 和 CUDA 驱动。如果 GPU 不可用，将自动回退到 CPU 推理。"));
    connect(gpuCheckBox_, &QCheckBox::toggled, this, &SettingsPage::useGPUChanged);

    auto* langLabel = new QLabel(tr("界面语言"), this);
    languageCombo_ = new QComboBox(this);
    languageCombo_->addItem(tr("跟随系统"), QStringLiteral(""));
    languageCombo_->addItem(QStringLiteral("中文"), QStringLiteral("zh_CN"));
    languageCombo_->addItem(QStringLiteral("English"), QStringLiteral("en"));
    connect(languageCombo_, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this]() {
        emit languageChanged(languageCombo_->currentData().toString());
    });

    auto* recentModelsLabel = new QLabel(tr("最近模型"), this);
    recentModelsList_ = new QListWidget(this);
    recentModelsList_->setObjectName(QStringLiteral("RecentModelsList"));
    recentModelsList_->setMinimumHeight(120);

    auto* recentInputsLabel = new QLabel(tr("最近图像"), this);
    recentInputsList_ = new QListWidget(this);
    recentInputsList_->setObjectName(QStringLiteral("RecentInputsList"));
    recentInputsList_->setMinimumHeight(120);

    connect(exportDirectoryEdit_, &QLineEdit::editingFinished, this, [this]() {
        emit defaultExportDirectoryChanged(exportDirectoryEdit_->text().trimmed());
    });
    connect(recentModelsList_, &QListWidget::itemClicked, this, [this](QListWidgetItem* item) {
        if (item != nullptr) {
            const QString path = item->data(Qt::UserRole).toString();
            if (!path.isEmpty()) {
                emit recentModelActivated(path);
            }
        }
    });
    connect(recentInputsList_, &QListWidget::itemClicked, this, [this](QListWidgetItem* item) {
        if (item != nullptr) {
            const QString path = item->data(Qt::UserRole).toString();
            if (!path.isEmpty()) {
                emit recentInputActivated(path);
            }
        }
    });
    connect(browseButton, &QPushButton::clicked, this, [this]() {
        const QString directoryPath = QFileDialog::getExistingDirectory(
            this,
            tr("选择默认导出目录"),
            exportDirectoryEdit_->text().trimmed());
        if (!directoryPath.isEmpty()) {
            exportDirectoryEdit_->setText(directoryPath);
            emit defaultExportDirectoryChanged(directoryPath);
        }
    });
    connect(threadCountSpin_, QOverload<int>::of(&QSpinBox::valueChanged),
            this, &SettingsPage::inferenceThreadCountChanged);

    auto* aboutButton = new QPushButton(tr("关于"), this);
    aboutButton->setObjectName(QStringLiteral("SecondaryButton"));
    connect(aboutButton, &QPushButton::clicked, this, [this]() {
        QMessageBox::about(
            this,
            tr("关于 AI 检测工具"),
            tr("<h3>AI 检测工具 v%1</h3>"
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
    layout->addWidget(gpuCheckBox_);
    layout->addWidget(langLabel);
    layout->addWidget(languageCombo_);
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
    for (const QString& path : recentModels) {
        auto* item = new QListWidgetItem(QFileInfo(path).fileName(), recentModelsList_);
        item->setData(Qt::UserRole, path);
        item->setToolTip(path);
    }
}

void SettingsPage::setRecentInputs(const QStringList& recentInputs) {
    recentInputsList_->clear();
    for (const QString& path : recentInputs) {
        auto* item = new QListWidgetItem(QFileInfo(path).fileName(), recentInputsList_);
        item->setData(Qt::UserRole, path);
        item->setToolTip(path);
    }
}

void SettingsPage::setInferenceThreadCount(const int count) {
    threadCountSpin_->blockSignals(true);
    threadCountSpin_->setValue(count);
    threadCountSpin_->blockSignals(false);
}

void SettingsPage::setUseGPU(const bool useGPU) {
    gpuCheckBox_->blockSignals(true);
    gpuCheckBox_->setChecked(useGPU);
    gpuCheckBox_->blockSignals(false);
}

void SettingsPage::setLanguage(const QString& langCode) {
    languageCombo_->blockSignals(true);
    for (int i = 0; i < languageCombo_->count(); ++i) {
        if (languageCombo_->itemData(i).toString() == langCode) {
            languageCombo_->setCurrentIndex(i);
            break;
        }
    }
    languageCombo_->blockSignals(false);
}

}  // namespace aitoolkit::ui
