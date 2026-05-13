#include "ui/pages/home_page.h"

#include <QLabel>
#include <QVBoxLayout>

namespace aitoolkit::ui {

HomePage::HomePage(QWidget* parent)
    : QWidget(parent) {
    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(24, 24, 24, 24);
    layout->setSpacing(12);

    auto* title = new QLabel(QStringLiteral("AI Toolkit C"), this);
    title->setStyleSheet(QStringLiteral("font-size: 24px; font-weight: 600;"));

    auto* body = new QLabel(QStringLiteral("从左侧开始加载模型清单，选择图片后即可运行一次目标检测。"), this);
    body->setWordWrap(true);

    layout->addWidget(title);
    layout->addWidget(body);
    layout->addStretch(1);
}

}  // namespace aitoolkit::ui
