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

    auto* eyebrow = new QLabel(QStringLiteral("AITOOLKIT"), sidebar);
    eyebrow->setObjectName(QStringLiteral("SidebarEyebrow"));
    auto* title = new QLabel(tr("AI 检测工具"), sidebar);
    title->setObjectName(QStringLiteral("SidebarTitle"));
    auto* desc = new QLabel(tr("基于 ONNX Runtime 的轻量级 AI 推理桌面工具，支持目标检测、图像分类和实例分割。"), sidebar);
    desc->setObjectName(QStringLiteral("SidebarDesc"));
    desc->setWordWrap(true);

    sidebarLayout->addWidget(eyebrow);
    sidebarLayout->addWidget(title);
    sidebarLayout->addWidget(desc);

    auto* section = new QLabel(tr("功能导航"), sidebar);
    section->setObjectName(QStringLiteral("SidebarSection"));
    sidebarLayout->addWidget(section);

    struct PageButtonSpec {
        QString text;
        int pageId;
    };

    const PageButtonSpec buttons[] = {
        {tr("首页"), HomePageId},
        {tr("模型"), ModelsPageId},
        {tr("推理"), InferencePageId},
        {tr("结果"), ResultsPageId},
        {tr("设置"), SettingsPageId},
    };

    for (const PageButtonSpec& spec : buttons) {
        auto* button = new QPushButton(spec.text, sidebar);
        button->setObjectName(QStringLiteral("NavItem"));
        button->setCheckable(true);
        button->setMinimumHeight(38);
        button->setAccessibleName(spec.text);
        connect(button, &QPushButton::clicked, this, [this, spec]() {
            setCurrentPage(spec.pageId);
            emit pageRequested(spec.pageId);
        });
        pageButtons_.append(button);
        sidebarLayout->addWidget(button);
    }

    sidebarLayout->addStretch(1);

    auto* footer = new QLabel(tr("支持 YOLOv5/v8/v11 系列模型，更多功能持续更新中。"), sidebar);
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
