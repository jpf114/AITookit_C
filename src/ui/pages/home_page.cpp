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

    auto* title = new QLabel(QStringLiteral("AI \u68c0\u6d4b\u5de5\u5177 v%1").arg(QCoreApplication::applicationVersion()), this);
    title->setStyleSheet(QStringLiteral("font-size: 24px; font-weight: 600;"));

    auto* lead = new QLabel(
        QStringLiteral("\u4ece\u5de6\u4fa7\u5f00\u59cb\u52a0\u8f7d\u6a21\u578b\u6e05\u5355\uff0c\u9009\u62e9\u5f85\u63a8\u7406\u56fe\u50cf\u540e\u5373\u53ef\u6267\u884c\u4e00\u6b21\u76ee\u6807\u68c0\u6d4b\uff0c\u5e76\u5728\u7ed3\u679c\u9875\u67e5\u770b\u53e0\u52a0\u9884\u89c8\u4e0e\u660e\u7ec6\u3002"),
        this);
    lead->setWordWrap(true);

    auto* actionsSection = new QWidget(this);
    actionsSection->setObjectName(QStringLiteral("HomeActionsSection"));
    auto* actionsLayout = new QHBoxLayout(actionsSection);
    actionsLayout->setContentsMargins(0, 0, 0, 0);
    actionsLayout->setSpacing(12);

    auto* loadModelBtn = new QPushButton(QStringLiteral("\u52a0\u8f7d\u6a21\u578b"), this);
    loadModelBtn->setObjectName(QStringLiteral("PrimaryButton"));

    auto* selectImageBtn = new QPushButton(QStringLiteral("\u9009\u62e9\u56fe\u50cf"), this);
    selectImageBtn->setObjectName(QStringLiteral("SecondaryButton"));

    auto* downloadBtn = new QPushButton(QStringLiteral("\u4e0b\u8f7d\u793a\u4f8b\u6a21\u578b"), this);
    downloadBtn->setObjectName(QStringLiteral("SecondaryButton"));
    downloadBtn->setToolTip(QStringLiteral("\u4e0b\u8f7d YOLOv8n COCO \u793a\u4f8b\u6a21\u578b\uff08\u7ea6 6MB\uff09"));

    auto* catalogButton = new QPushButton(QStringLiteral("\u6a21\u578b\u76ee\u5f55"), this);
    catalogButton->setObjectName(QStringLiteral("SecondaryButton"));
    catalogButton->setToolTip(QStringLiteral("\u6d4f\u89c8\u548c\u4e0b\u8f7d\u66f4\u591a\u6a21\u578b"));

    quickStartBtn_ = new QPushButton(QStringLiteral("\u5feb\u901f\u4f53\u9a8c"), this);
    quickStartBtn_->setObjectName(QStringLiteral("PrimaryButton"));
    quickStartBtn_->setToolTip(QStringLiteral("\u4f7f\u7528\u793a\u4f8b\u56fe\u50cf\u6267\u884c\u4e00\u6b21\u76ee\u6807\u68c0\u6d4b"));
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

    auto* modelsTitle = new QLabel(QStringLiteral("\u6700\u8fd1\u6a21\u578b"), this);
    modelsTitle->setStyleSheet(QStringLiteral("font-size: 14px; font-weight: 600;"));

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

    auto* inputsTitle = new QLabel(QStringLiteral("\u6700\u8fd1\u56fe\u50cf"), this);
    inputsTitle->setStyleSheet(QStringLiteral("font-size: 14px; font-weight: 600;"));

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
