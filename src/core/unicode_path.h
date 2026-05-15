#pragma once

#include <QString>

#include <string>

namespace aitoolkit::core {

std::string toNativePath(const QString& filePath);

#ifdef Q_OS_WIN
std::wstring toWidePath(const QString& filePath);
#endif

}  // namespace aitoolkit::core
