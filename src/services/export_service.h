#pragma once

#include <QImage>
#include <QString>

#include "core/types.h"

namespace aitoolkit::services {

class ExportService {
public:
    void exportJson(const QString& filePath, const core::InferenceSummary& summary) const;
    void exportRenderedImage(const QString& filePath, const QImage& image, const core::InferenceSummary& summary) const;
};

}  // namespace aitoolkit::services
