#pragma once

#include <QFrame>

namespace aitoolkit::ui {

class NavPanel : public QFrame {
    Q_OBJECT

public:
    enum PageId {
        HomePageId = 0,
        ModelsPageId,
        InferencePageId,
        ResultsPageId,
        SettingsPageId,
    };

    explicit NavPanel(QWidget* parent = nullptr);

signals:
    void pageRequested(int pageId);
};

}  // namespace aitoolkit::ui
