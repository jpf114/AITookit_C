#include "core/app_logger.h"

#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QLoggingCategory>
#include <QMutex>
#include <QMutexLocker>
#include <QStandardPaths>
#include <QTextStream>

namespace aitoolkit::core {

namespace {

QMutex g_logMutex;
QString g_logFilePath;
constexpr int kMaxLogBytes = 5 * 1024 * 1024;

QString resolveLogFilePath() {
    const QString baseDir = QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation);
    QDir().mkpath(baseDir);
    return QDir(baseDir).filePath(QStringLiteral("aitoolkit.log"));
}

void rotateIfNeeded(QFile& file) {
    if (file.size() <= kMaxLogBytes) {
        return;
    }
    file.close();
    const QString backupPath = file.fileName() + QStringLiteral(".1");
    QFile::remove(backupPath);
    QFile::rename(file.fileName(), backupPath);
    file.setFileName(resolveLogFilePath());
    file.open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text);
}

void appendLine(const QString& level, const QString& message) {
    QMutexLocker locker(&g_logMutex);
    if (g_logFilePath.isEmpty()) {
        g_logFilePath = resolveLogFilePath();
    }

    QFile file(g_logFilePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text)) {
        return;
    }

    rotateIfNeeded(file);

    QTextStream stream(&file);
    stream << QDateTime::currentDateTime().toString(Qt::ISODateWithMs)
           << ' ' << level << ' ' << message << '\n';
}

void messageHandler(QtMsgType type, const QMessageLogContext& context, const QString& message) {
    Q_UNUSED(context)
    const QString line = message;
    switch (type) {
        case QtDebugMsg:
            appendLine(QStringLiteral("DEBUG"), line);
            break;
        case QtInfoMsg:
            appendLine(QStringLiteral("INFO"), line);
            break;
        case QtWarningMsg:
            appendLine(QStringLiteral("WARN"), line);
            break;
        case QtCriticalMsg:
        case QtFatalMsg:
            appendLine(QStringLiteral("ERROR"), line);
            break;
    }

    if (type == QtFatalMsg) {
        abort();
    }
}

}  // namespace

void AppLogger::install() {
    g_logFilePath = resolveLogFilePath();
    qInstallMessageHandler(messageHandler);
    logInfo(QStringLiteral("Application logger initialized"));
}

void AppLogger::logInfo(const QString& message) {
    appendLine(QStringLiteral("INFO"), message);
}

void AppLogger::logWarning(const QString& message) {
    appendLine(QStringLiteral("WARN"), message);
}

void AppLogger::logError(const QString& message) {
    appendLine(QStringLiteral("ERROR"), message);
}

QString AppLogger::logFilePath() {
    if (g_logFilePath.isEmpty()) {
        g_logFilePath = resolveLogFilePath();
    }
    return g_logFilePath;
}

}  // namespace aitoolkit::core
