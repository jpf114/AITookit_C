#pragma once

#include <QString>
#include <QStringList>

namespace aitoolkit::core {

struct ModelManifest {
    QString manifestPath;
    QString name;
    QString taskType;
    QString backendType;
    QString modelPath;
    QString labelsPath;
    QString decoder;
    int inputWidth = 0;
    int inputHeight = 0;
    double confidenceThreshold = 0.25;
    double nmsThreshold = 0.45;
    QStringList labels;
    QStringList labelsInline;
};

ModelManifest loadModelManifest(const QString& manifestPath);

}  // namespace aitoolkit::core
