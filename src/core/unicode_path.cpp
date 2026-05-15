#include "core/unicode_path.h"

namespace aitoolkit::core {

std::string toNativePath(const QString& filePath) {
    return filePath.toUtf8().toStdString();
}

#ifdef Q_OS_WIN
std::wstring toWidePath(const QString& filePath) {
    return filePath.toStdWString();
}
#endif

}  // namespace aitoolkit::core
