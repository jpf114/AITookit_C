#pragma once

#include <QString>

#include "core/types.h"

namespace aitoolkit::services {

class ExportService {
public:
    void exportJson(const QString& filePath, const core::InferenceSummary& summary) const;
};

}  // namespace aitoolkit::services
