#include "ui/pages/home_page.h"

#include <QCoreApplication>
#include <QFileInfo>
#include <QLabel>
#include <QListWidget>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>

namespace aitoolkit::ui {

HomePage::HomePage(QWidget* parent)
    : QWidget(parent) {
    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(24, 24, 24, 24);
    layout->setSpacing(16);

    auto* title = new QLabel(tr("AI 检测工具 v%1").arg(QCoreApplication::applicationVersion()), this);
    title->setObjectName(QStringLiteral("HomePageTitle"));

    auto* lead = new QLabel(
        tr("从左侧开始加载模型清单，选择待推理图像后即可执行一次目标检测，并在结果页查看叠加预览与明细。"),
        this);
    lead->setWordWrap(true);

    auto* actionsSection = new QWidget(this);
    actionsSection->setObjectName(QStringLiteral("HomeActionsSection"));
    auto* actionsLayout = new QHBoxLayout(actionsSection);
    actionsLayout->setContentsMargins(0, 0, 0, 0);
    actionsLayout->setSpacing(12);

    auto* loadModelBtn = new QPushButton(tr("加载模型"), this);
    loadModelBtn->setObjectName(QStringLiteral("PrimaryButton"));
    loadModelBtn->setAccessibleName(tr("加载模型"));

    auto* selectImageBtn = new QPushButton(tr("选择图像"), this);
    selectImageBtn->setObjectName(QStringLiteral("SecondaryButton"));
    selectImageBtn->setAccessibleName(tr("选择图像"));

    auto* downloadBtn = new QPushButton(tr("下载示例模型"), this);
    downloadBtn->setObjectName(QStringLiteral("SecondaryButton"));
    downloadBtn->setToolTip(tr("下载 YOLOv8n COCO 示例模型（约 6MB）"));
    downloadBtn->setAccessibleName(tr("下载示例模型"));

    auto* catalogButton = new QPushButton(tr("模型目录"), this);
    catalogButton->setObjectName(QStringLiteral("SecondaryButton"));
    catalogButton->setToolTip(tr("浏览和下载更多模型"));
    catalogButton->setAccessibleName(tr("模型目录"));

    quickStartBtn_ = new QPushButton(tr("快速体验"), this);
    quickStartBtn_->setObjectName(QStringLiteral("PrimaryButton"));
    quickStartBtn_->setToolTip(tr("使用示例图像执行一次目标检测"));
    quickStartBtn_->setAccessibleName(tr("快速体验"));
    quickStartBtn_->hide();

    actionsLayout->addWidget(loadModelBtn);
    actionsLayout->addWidget(selectImageBtn);
    actionsLayout->addWidget(downloadBtn);
    actionsLayout->addWidget(catalogButton);
    actionsLayout->addWidget(quickStartBtn_);
    actionsLayout->addStretch(1);

    auto* recentSection = new QWidget(this);
    recentSection->setObjectName(QStringLiteral("HomeRecentSection"));
    auto* recentLayout = new QHBoxLayout(recentSection);
    recentLayout->setContentsMargins(0, 0, 0, 0);
    recentLayout->setSpacing(16);

    auto* modelsColumn = new QWidget(this);
    auto* modelsColumnLayout = new QVBoxLayout(modelsColumn);
    modelsColumnLayout->setContentsMargins(0, 0, 0, 0);
    modelsColumnLayout->setSpacing(6);

    auto* modelsTitle = new QLabel(tr("最近模型"), this);
    modelsTitle->setObjectName(QStringLiteral("HomeRecentTitle"));

    recentModelsList_ = new QListWidget(this);
    recentModelsList_->setObjectName(QStringLiteral("HomeRecentModelsList"));
    recentModelsList_->setMaximumHeight(160);
    recentModelsList_->setSelectionMode(QAbstractItemView::SingleSelection);

    modelsColumnLayout->addWidget(modelsTitle);
    modelsColumnLayout->addWidget(recentModelsList_);

    auto* inputsColumn = new QWidget(this);
    auto* inputsColumnLayout = new QVBoxLayout(inputsColumn);
    inputsColumnLayout->setContentsMargins(0, 0, 0, 0);
    inputsColumnLayout->setSpacing(6);

    auto* inputsTitle = new QLabel(tr("最近图像"), this);
    inputsTitle->setObjectName(QStringLiteral("HomeRecentTitle"));

    recentInputsList_ = new QListWidget(this);
    recentInputsList_->setObjectName(QStringLiteral("HomeRecentInputsList"));
    recentInputsList_->setMaximumHeight(160);
    recentInputsList_->setSelectionMode(QAbstractItemView::SingleSelection);

    inputsColumnLayout->addWidget(inputsTitle);
    inputsColumnLayout->addWidget(recentInputsList_);

    recentLayout->addWidget(modelsColumn, 1);
    recentLayout->addWidget(inputsColumn, 1);

    connect(loadModelBtn, &QPushButton::clicked, this, &HomePage::loadModelClicked);
    connect(selectImageBtn, &QPushButton::clicked, this, &HomePage::selectImageClicked);
    connect(downloadBtn, &QPushButton::clicked, this, &HomePage::downloadSampleModelClicked);
    connect(catalogButton, &QPushButton::clicked, this, &HomePage::modelCatalogRequested);
    connect(quickStartBtn_, &QPushButton::clicked, this, &HomePage::quickStartClicked);
    connect(recentModelsList_, &QListWidget::itemClicked, this, [this](QListWidgetItem* item) {
        emit recentModelActivated(item->data(Qt::UserRole).toString());
    });
    connect(recentInputsList_, &QListWidget::itemClicked, this, [this](QListWidgetItem* item) {
        emit recentInputActivated(item->data(Qt::UserRole).toString());
    });

    layout->addWidget(title);
    layout->addWidget(lead);
    layout->addWidget(actionsSection);
    layout->addWidget(recentSection);
    layout->addStretch(1);
}

void HomePage::setRecentModels(const QStringList& paths) {
    recentModelsList_->clear();
    for (const QString& path : paths) {
        const QFileInfo info(path);
        auto* item = new QListWidgetItem(info.fileName(), recentModelsList_);
        item->setData(Qt::UserRole, path);
        item->setToolTip(path);
    }
}

void HomePage::setRecentInputs(const QStringList& paths) {
    recentInputsList_->clear();
    for (const QString& path : paths) {
        const QFileInfo info(path);
        auto* item = new QListWidgetItem(info.fileName(), recentInputsList_);
        item->setData(Qt::UserRole, path);
        item->setToolTip(path);
    }
}

void HomePage::setQuickStartVisible(const bool visible) {
    quickStartBtn_->setVisible(visible);
}

}  // namespace aitoolkit::ui
