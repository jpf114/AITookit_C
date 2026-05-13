#include "ui/pages/settings_page.h"

#include <QLabel>
#include <QVBoxLayout>

namespace aitoolkit::ui {

SettingsPage::SettingsPage(QWidget* parent)
    : QWidget(parent) {
    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(24, 24, 24, 24);
    layout->setSpacing(12);

    auto* title = new QLabel(QStringLiteral("设置"), this);
    title->setStyleSheet(QStringLiteral("font-size: 20px; font-weight: 600;"));

    auto* body = new QLabel(QStringLiteral("后续将在这里补充最近模型、默认导出目录等应用设置。"), this);
    body->setWordWrap(true);

    layout->addWidget(title);
    layout->addWidget(body);
    layout->addStretch(1);
}

}  // namespace aitoolkit::ui
