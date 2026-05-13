#pragma once

#include <QString>
#include <QWidget>

class QLabel;
class QPushButton;

namespace aitoolkit::ui {

class InferencePage : public QWidget {
    Q_OBJECT

public:
    explicit InferencePage(QWidget* parent = nullptr);

    void setCurrentImagePath(const QString& imagePath);
    void setModelReady(bool ready);

signals:
    void imageSelected(const QString& imagePath);
    void runRequested();

private:
    QLabel* imagePathLabel_ = nullptr;
    QPushButton* runButton_ = nullptr;
};

}  // namespace aitoolkit::ui
