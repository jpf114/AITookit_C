#pragma once

#include <QColor>
#include <QRectF>
#include <QString>
#include <QVector>

namespace aitoolkit::core {

struct DetectionItem {
    int classId = -1;
    QString label;
    float confidence = 0.0f;
    QRectF boundingBox;
    QColor renderColor;
};

struct InferenceSummary {
    QString modelName;
    QString inputPath;
    int detectionCount = 0;
    double elapsedMs = 0.0;
    int imageWidth = 0;
    int imageHeight = 0;
    QVector<DetectionItem> detections;
};

}  // namespace aitoolkit::core
