#pragma once

#include <QJsonObject>
#include <QString>

namespace aitoolkit::core {

QJsonObject readJsonObject(const QString& filePath);
void writeJsonObject(const QString& filePath, const QJsonObject& object);

}  // namespace aitoolkit::core
