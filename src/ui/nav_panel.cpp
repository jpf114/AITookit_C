#include "ui/nav_panel.h"

#include <QPushButton>
#include <QVBoxLayout>

namespace aitoolkit::ui {

NavPanel::NavPanel(QWidget* parent)
    : QFrame(parent) {
    setObjectName(QStringLiteral("NavPanel"));

    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(16, 16, 16, 16);
    layout->setSpacing(8);

    struct PageButtonSpec {
        const char* text;
        int pageId;
    };

    const PageButtonSpec buttons[] = {
        {"首页", HomePageId},
        {"模型", ModelsPageId},
        {"推理", InferencePageId},
        {"结果", ResultsPageId},
        {"设置", SettingsPageId},
    };

    for (const PageButtonSpec& spec : buttons) {
        auto* button = new QPushButton(QString::fromUtf8(spec.text), this);
        button->setMinimumHeight(36);
        connect(button, &QPushButton::clicked, this, [this, spec]() {
            emit pageRequested(spec.pageId);
        });
        layout->addWidget(button);
    }

    layout->addStretch(1);
}

}  // namespace aitoolkit::ui
