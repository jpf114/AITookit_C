#pragma once

#include <QImage>
#include <QJsonObject>
#include <QString>
#include <QVector>

#include "core/types.h"

namespace aitoolkit::services {

class ExportService {
public:
    void exportJson(const QString& filePath, const core::InferenceSummary& summary) const;
    void exportRenderedImage(const QString& filePath, const QImage& image, const core::InferenceSummary& summary) const;
    bool exportBatchJson(const QVector<core::InferenceSummary>& results, const QString& outputPath) const;

private:
    QJsonObject summaryToJson(const core::InferenceSummary& summary) const;
};

}  // namespace aitoolkit::services
