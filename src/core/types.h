#pragma once

#include <QColor>
#include <QImage>
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

struct ClassificationItem {
    int classId = -1;
    QString label;
    float confidence = 0.0f;
};

struct SegmentationItem {
    int classId = -1;
    QString label;
    float confidence = 0.0f;
    QRectF boundingBox;
    QImage mask;
    QColor renderColor;
};

struct InferenceSummary {
    QString modelName;
    QString inputPath;
    QString taskType;
    int detectionCount = 0;
    double elapsedMs = 0.0;
    int imageWidth = 0;
    int imageHeight = 0;
    bool success = true;
    QString errorMessage;
    QVector<DetectionItem> detections;
    QVector<ClassificationItem> classifications;
    QVector<SegmentationItem> segmentations;
};

}  // namespace aitoolkit::core
