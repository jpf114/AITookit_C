#include "core/unicode_path.h"

namespace aitoolkit::core {

std::string toNativePath(const QString& filePath) {
#ifdef Q_OS_WIN
    return filePath.toStdString();
#else
    return filePath.toStdString();
#endif
}

#ifdef Q_OS_WIN
std::wstring toWidePath(const QString& filePath) {
    return filePath.toStdWString();
}
#endif

}  // namespace aitoolkit::core
