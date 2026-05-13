#include "ui/pages/settings_page.h"

#include <QFileDialog>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QPushButton>
#include <QVBoxLayout>

namespace aitoolkit::ui {

SettingsPage::SettingsPage(QWidget* parent)
    : QWidget(parent) {
    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(24, 24, 24, 24);
    layout->setSpacing(12);

    auto* title = new QLabel(QStringLiteral("Settings"), this);
    title->setStyleSheet(QStringLiteral("font-size: 20px; font-weight: 600;"));

    auto* exportLabel = new QLabel(QStringLiteral("Default export directory"), this);
    auto* exportRow = new QHBoxLayout();
    exportDirectoryEdit_ = new QLineEdit(this);
    exportDirectoryEdit_->setPlaceholderText(QStringLiteral("Falls back to the current image folder"));
    auto* browseButton = new QPushButton(QStringLiteral("Browse"), this);
    exportRow->addWidget(exportDirectoryEdit_, 1);
    exportRow->addWidget(browseButton);

    auto* recentModelsLabel = new QLabel(QStringLiteral("Recent models"), this);
    recentModelsList_ = new QListWidget(this);
    recentModelsList_->setObjectName(QStringLiteral("RecentModelsList"));
    recentModelsList_->setMinimumHeight(120);

    auto* recentInputsLabel = new QLabel(QStringLiteral("Recent images"), this);
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
            QStringLiteral("Choose default export directory"),
            exportDirectoryEdit_->text().trimmed());
        if (!directoryPath.isEmpty()) {
            exportDirectoryEdit_->setText(directoryPath);
            emit defaultExportDirectoryChanged(directoryPath);
        }
    });

    layout->addWidget(title);
    layout->addWidget(exportLabel);
    layout->addLayout(exportRow);
    layout->addWidget(recentModelsLabel);
    layout->addWidget(recentModelsList_);
    layout->addWidget(recentInputsLabel);
    layout->addWidget(recentInputsList_, 1);
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

}  // namespace aitoolkit::ui
