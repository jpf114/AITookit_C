#include "ui/nav_panel.h"

#include <QFrame>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>

namespace aitoolkit::ui {

NavPanel::NavPanel(QWidget* parent)
    : QFrame(parent) {
    setObjectName(QStringLiteral("NavPanel"));

    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);

    auto* sidebar = new QFrame(this);
    sidebar->setObjectName(QStringLiteral("SidebarFrame"));

    auto* sidebarLayout = new QVBoxLayout(sidebar);
    sidebarLayout->setContentsMargins(14, 12, 14, 12);
    sidebarLayout->setSpacing(8);

    auto* eyebrow = new QLabel(QStringLiteral("AI TOOLKIT"), sidebar);
    eyebrow->setObjectName(QStringLiteral("SidebarEyebrow"));
    auto* title = new QLabel(QStringLiteral("AI 检测工具"), sidebar);
    title->setObjectName(QStringLiteral("SidebarTitle"));
    auto* desc = new QLabel(QStringLiteral("围绕模型清单、图像推理和结果导出组织一个紧凑的桌面工作台。"), sidebar);
    desc->setObjectName(QStringLiteral("SidebarDesc"));
    desc->setWordWrap(true);

    sidebarLayout->addWidget(eyebrow);
    sidebarLayout->addWidget(title);
    sidebarLayout->addWidget(desc);

    auto* section = new QLabel(QStringLiteral("功能导航"), sidebar);
    section->setObjectName(QStringLiteral("SidebarSection"));
    sidebarLayout->addWidget(section);

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
        auto* button = new QPushButton(QString::fromUtf8(spec.text), sidebar);
        button->setObjectName(QStringLiteral("NavItem"));
        button->setCheckable(true);
        button->setMinimumHeight(38);
        connect(button, &QPushButton::clicked, this, [this, spec]() {
            setCurrentPage(spec.pageId);
            emit pageRequested(spec.pageId);
        });
        pageButtons_.append(button);
        sidebarLayout->addWidget(button);
    }

    sidebarLayout->addStretch(1);

    auto* footer = new QLabel(QStringLiteral("当前版本先聚焦单图推理链路，后续可继续补批处理、结果复核和模型管理能力。"), sidebar);
    footer->setObjectName(QStringLiteral("SidebarFooter"));
    footer->setWordWrap(true);
    sidebarLayout->addWidget(footer);

    layout->addWidget(sidebar);
    setCurrentPage(HomePageId);
}

void NavPanel::setCurrentPage(const int pageId) {
    for (int index = 0; index < pageButtons_.size(); ++index) {
        pageButtons_.at(index)->setChecked(index == pageId);
    }
}

}  // namespace aitoolkit::ui
