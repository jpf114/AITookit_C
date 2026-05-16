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

    const QString msg = QStringLiteral(
        "AI 检测工具遇到严重错误，需要关闭。\n\n"
        "崩溃转储已保存至：\n%1\n\n"
        "请将此文件发送给开发者以帮助修复问题。"
    ).arg(dumpPath);

    MessageBoxW(
        nullptr,
        reinterpret_cast<LPCWSTR>(msg.utf16()),
        L"AI \u68c0\u6d4b\u5de5\u5177 - \u5d29\u6e83",
        MB_OK | MB_ICONERROR | MB_TOPMOST);

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
