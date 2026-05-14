#include "ui/pages/models_page.h"

#include <QFileDialog>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>

namespace aitoolkit::ui {

namespace {

QString manifestSummaryText(const core::ModelManifest& manifest) {
    if (manifest.manifestPath.isEmpty()) {
        return QStringLiteral("Load a manifest to see model details.");
    }

    return QStringLiteral(
               "Name: %1\nTask: %2\nBackend: %3\nInput: %4 x %5\nLabels: %6\nModel: %7")
        .arg(manifest.name)
        .arg(manifest.taskType)
        .arg(manifest.backendType)
        .arg(manifest.inputWidth)
        .arg(manifest.inputHeight)
        .arg(manifest.labels.size())
        .arg(manifest.modelPath);
}

}  // namespace

ModelsPage::ModelsPage(QWidget* parent)
    : QWidget(parent) {
    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(24, 24, 24, 24);
    layout->setSpacing(12);

    auto* title = new QLabel(QStringLiteral("Models"), this);
    title->setStyleSheet(QStringLiteral("font-size: 20px; font-weight: 600;"));

    auto* loadButton = new QPushButton(QStringLiteral("Load manifest"), this);
    manifestPathLabel_ = new QLabel(QStringLiteral("No manifest selected"), this);
    manifestPathLabel_->setObjectName(QStringLiteral("ManifestPathLabel"));
    manifestPathLabel_->setWordWrap(true);

    manifestSummaryLabel_ = new QLabel(QStringLiteral("Load a manifest to see model details."), this);
    manifestSummaryLabel_->setObjectName(QStringLiteral("ManifestSummaryLabel"));
    manifestSummaryLabel_->setWordWrap(true);

    connect(loadButton, &QPushButton::clicked, this, [this]() {
        const QString path = QFileDialog::getOpenFileName(
            this,
            QStringLiteral("Choose model manifest"),
            QString(),
            QStringLiteral("JSON Files (*.json)"));
        if (!path.isEmpty()) {
            emit modelManifestSelected(path);
        }
    });

    layout->addWidget(title);
    layout->addWidget(loadButton);
    layout->addWidget(manifestPathLabel_);
    layout->addWidget(manifestSummaryLabel_);
    layout->addStretch(1);
}

void ModelsPage::setCurrentManifest(const core::ModelManifest& manifest) {
    setCurrentManifestPath(manifest.manifestPath);
    manifestSummaryLabel_->setText(manifestSummaryText(manifest));
}

void ModelsPage::setCurrentManifestPath(const QString& manifestPath) {
    if (manifestPath.isEmpty()) {
        manifestPathLabel_->setText(QStringLiteral("No manifest selected"));
        if (manifestSummaryLabel_ != nullptr) {
            manifestSummaryLabel_->setText(QStringLiteral("Load a manifest to see model details."));
        }
        return;
    }

    manifestPathLabel_->setText(QStringLiteral("Current manifest: %1").arg(manifestPath));
}

}  // namespace aitoolkit::ui
