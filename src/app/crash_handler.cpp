#include "app/crash_handler.h"

#ifdef Q_OS_WIN

#include <QCoreApplication>
#include <QDateTime>
#include <QDir>
#include <QStandardPaths>

#include <Windows.h>
#include <DbgHelp.h>

#include <cstdio>

namespace aitoolkit::app {

namespace {

LONG WINAPI unhandledExceptionHandler(EXCEPTION_POINTERS* exceptionInfo) {
    const QString dumpDir = crashDumpsDirectory();
    QDir().mkpath(dumpDir);

    const QString timestamp = QDateTime::currentDateTime().toString(QStringLiteral("yyyyMMdd_HHmmss"));
    const QString dumpPath = QDir(dumpDir).filePath(QStringLiteral("crash_%1.dmp").arg(timestamp));

    const HANDLE file = CreateFileW(
        reinterpret_cast<LPCWSTR>(dumpPath.utf16()),
        GENERIC_WRITE,
        0,
        nullptr,
        CREATE_ALWAYS,
        FILE_ATTRIBUTE_NORMAL,
        nullptr);

    if (file != INVALID_HANDLE_VALUE) {
        MINIDUMP_EXCEPTION_INFORMATION mei;
        mei.ThreadId = GetCurrentThreadId();
        mei.ExceptionPointers = exceptionInfo;
        mei.ClientPointers = FALSE;

        MiniDumpWriteDump(
            GetCurrentProcess(),
            GetCurrentProcessId(),
            file,
            MiniDumpWithDataSegs,
            &mei,
            nullptr,
            nullptr);

        CloseHandle(file);
    }

    return EXCEPTION_EXECUTE_HANDLER;
}

}

void installCrashHandler() {
    SetUnhandledExceptionFilter(unhandledExceptionHandler);
}

QString crashDumpsDirectory() {
    return QDir(QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation))
        .filePath(QStringLiteral("crash_dumps"));
}

}  // namespace aitoolkit::app

#else

namespace aitoolkit::app {

void installCrashHandler() {}

QString crashDumpsDirectory() {
    return {};
}

}  // namespace aitoolkit::app

#endif
