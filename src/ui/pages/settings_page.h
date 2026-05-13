#pragma once

#include <QWidget>

namespace aitoolkit::ui {

class SettingsPage : public QWidget {
    Q_OBJECT

public:
    explicit SettingsPage(QWidget* parent = nullptr);
};

}  // namespace aitoolkit::ui
