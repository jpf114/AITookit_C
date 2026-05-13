#pragma once

#include <QString>

class QApplication;

namespace aitoolkit::app {

class AppBootstrap {
public:
    static void initialize(QApplication& app);
    static QString applicationStylePath();
};

}  // namespace aitoolkit::app
