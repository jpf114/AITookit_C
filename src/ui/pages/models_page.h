#pragma once

#include <QString>
#include <QWidget>

class QLabel;

namespace aitoolkit::ui {

class ModelsPage : public QWidget {
    Q_OBJECT

public:
    explicit ModelsPage(QWidget* parent = nullptr);

    void setCurrentManifestPath(const QString& manifestPath);

signals:
    void modelManifestSelected(const QString& manifestPath);

private:
    QLabel* manifestPathLabel_ = nullptr;
};

}  // namespace aitoolkit::ui
