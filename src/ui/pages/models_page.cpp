#include "ui/pages/models_page.h"

#include <QFileDialog>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>

namespace aitoolkit::ui {

ModelsPage::ModelsPage(QWidget* parent)
    : QWidget(parent) {
    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(24, 24, 24, 24);
    layout->setSpacing(12);

    auto* title = new QLabel(QStringLiteral("模型清单"), this);
    title->setStyleSheet(QStringLiteral("font-size: 20px; font-weight: 600;"));

    auto* loadButton = new QPushButton(QStringLiteral("加载模型清单"), this);
    manifestPathLabel_ = new QLabel(QStringLiteral("当前未选择模型"), this);
    manifestPathLabel_->setWordWrap(true);

    connect(loadButton, &QPushButton::clicked, this, [this]() {
        const QString path = QFileDialog::getOpenFileName(
            this,
            QStringLiteral("选择模型清单"),
            QString(),
            QStringLiteral("JSON Files (*.json)"));
        if (!path.isEmpty()) {
            emit modelManifestSelected(path);
        }
    });

    layout->addWidget(title);
    layout->addWidget(loadButton);
    layout->addWidget(manifestPathLabel_);
    layout->addStretch(1);
}

void ModelsPage::setCurrentManifestPath(const QString& manifestPath) {
    if (manifestPath.isEmpty()) {
        manifestPathLabel_->setText(QStringLiteral("当前未选择模型"));
        return;
    }

    manifestPathLabel_->setText(QStringLiteral("当前模型清单：%1").arg(manifestPath));
}

}  // namespace aitoolkit::ui
