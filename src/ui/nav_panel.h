#pragma once

#include <QFrame>
#include <QList>

class QPushButton;

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

    void setCurrentPage(int pageId);

signals:
    void pageRequested(int pageId);

private:
    QList<QPushButton*> pageButtons_;
};

}  // namespace aitoolkit::ui
