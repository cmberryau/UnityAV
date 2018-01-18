#pragma once

#include "stdafx.h"

/**
 * \brief The callback type for debug calls on Unity's side
 */
typedef void(__stdcall * DebugCallback) (const char * str);

using namespace std;

/**
* \brief Responsible for logging debug information
*/
class Debug
{
public:
    ~Debug();

    static void Initialize(bool cacheLogs);
    static void Teardown();
    static void RegisterLogCallback(DebugCallback callback);
    static void RegisterWarningCallback(DebugCallback callback);
    static void RegisterErrorCallback(DebugCallback callback);

    static void DeregisterAllCallbacks();

    static void Log(const char * message, ...);
    static void LogWarning(const char * message, ...);
    static void LogError(const char * message, ...);

    static void Log(string & message, ...);
    static void LogWarning(string & message, ...);
    static void LogError(string & message, ...);

private:
    explicit Debug(bool cacheLogs = true);
    static const int kFormatBufferSize;
    static const string kFlushPrefix;
    static unique_ptr<Debug> Instance;

    void UnregisterAllCallbacksImpl();

    static string Format(const char *, va_list args);
    static string Format(string & message, va_list args);

    void RegisterLogCallbackImpl(DebugCallback callback);
    void RegisterWarningCallbackImpl(DebugCallback callback);
    void RegisterErrorCallbackImpl(DebugCallback callback);

    void LogImpl(string & message);
    void LogWarningImpl(string & message);
    void LogErrorImpl(string & message);

    void FlushLogs();
    void FlushWarnings();
    void FlushErrors();

    DebugCallback _logCallback;
    DebugCallback _warningCallback;
    DebugCallback _errorCallback;

    queue<string> _queuedLogs;
    queue<string> _queuedWarnings;
    queue<string> _queuedErrors;

    bool _cacheLogs;
};