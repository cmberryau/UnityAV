#include "stdafx.h"
#include "Debug.h"

// exported calls for logging
extern "C" void UNITY_INTERFACE_EXPORT DeregisterAllCallbacks()
{
    Debug::DeregisterAllCallbacks();
}

extern "C" void UNITY_INTERFACE_EXPORT RegisterLogCallback(DebugCallback callback)
{
    if (callback)
    {
        Debug::RegisterLogCallback(callback);
    }
}

extern "C" void UNITY_INTERFACE_EXPORT RegisterWarningCallback(DebugCallback callback)
{
    if (callback)
    {
        Debug::RegisterWarningCallback(callback);
    }
}

extern "C" void UNITY_INTERFACE_EXPORT RegisterErrorCallback(DebugCallback callback)
{
    if (callback)
    {
        Debug::RegisterErrorCallback(callback);
    }
}

const int Debug::kFormatBufferSize = 2048;
const string Debug::kFlushPrefix = "Flush: ";
unique_ptr<Debug> Debug::Instance;

Debug::Debug(bool cacheLogs)
    : _logCallback(nullptr), _warningCallback(nullptr), 
    _errorCallback(nullptr), _cacheLogs(cacheLogs)
{
    DeregisterAllCallbacks();
}

Debug::~Debug()
{
    DeregisterAllCallbacks();
}

void Debug::DeregisterAllCallbacks()
{
    if (Instance)
    {
        Instance->UnregisterAllCallbacksImpl();
    }
}

void Debug::Initialize(bool cacheLogs = true)
{
    if (!Instance)
    {
        Instance = unique_ptr<Debug>(new Debug(cacheLogs));
    }
}

void Debug::Teardown()
{
    if (Instance)
    {
        Instance.reset();
    }
}

void Debug::RegisterLogCallback(DebugCallback callback)
{
    if (!Instance)
    {
        Initialize();
    }

    if (callback)
    {
        Instance->RegisterLogCallbackImpl(callback);
    }
}

void Debug::RegisterWarningCallback(DebugCallback callback)
{
    if (!Instance)
    {
        Initialize();
    }

    if (callback)
    {
        Instance->RegisterWarningCallbackImpl(callback);
    }
}

void Debug::RegisterErrorCallback(DebugCallback callback)
{
    if (!Instance)
    {
        Initialize();
    }

    if (callback)
    {
        Instance->RegisterErrorCallbackImpl(callback);
    }
}

void Debug::UnregisterAllCallbacksImpl()
{
    _logCallback = nullptr;
    _warningCallback = nullptr;
    _errorCallback = nullptr;
}

void Debug::Log(const char * message, ...)
{
    if (message && Instance)
    {
        va_list args;
        va_start(args, message);
        Instance->LogImpl(Format(message, args));
        va_end(args);
    }
}

void Debug::LogWarning(const char * message, ...)
{
    if (message && Instance)
    {
        va_list args;
        va_start(args, message);
        Instance->LogWarningImpl(Format(message, args));
        va_end(args);
    }
}

void Debug::LogError(const char * message, ...)
{
    if (message && Instance)
    {
        va_list args;
        va_start(args, message);
        Instance->LogErrorImpl(Format(message, args));
        va_end(args);
    }
}

void Debug::Log(string & message, ...)
{
    if (message.size() > 0 && Instance)
    {
        va_list args;
        auto * cMessage = message.c_str();
        va_start(args, cMessage);
        Instance->LogImpl(Format(message, args));
        va_end(args);
    }
}

void Debug::LogWarning(string & message, ...)
{
    if (message.size() > 0 && Instance)
    {
        va_list args;
        auto * cMessage = message.c_str();
        va_start(args, cMessage);
        Instance->LogWarningImpl(Format(message, args));
        va_end(args);
    }
}

void Debug::LogError(string & message, ...)
{
    if (message.size() > 0 && Instance)
    {
        va_list args;
        auto * cMessage = message.c_str();
        va_start(args, cMessage);
        Instance->LogErrorImpl(Format(message, args));
        va_end(args);
    }
}

string Debug::Format(const char * message, va_list args)
{
    char buf[kFormatBufferSize];

    vsprintf_s(buf, message, args);

    return string(buf);
}

string Debug::Format(string & message, va_list args)
{
    char buf[kFormatBufferSize];

    vsprintf_s(buf, message.c_str(), args);

    return string(buf);
}

void Debug::RegisterLogCallbackImpl(DebugCallback callback)
{
    _logCallback = callback;

    if (_queuedLogs.size() > 0)
    {
        FlushLogs();
    }
}

void Debug::RegisterWarningCallbackImpl(DebugCallback callback)
{
    _warningCallback = callback;

    if (_queuedWarnings.size() > 0)
    {
        FlushWarnings();
    }
}

void Debug::RegisterErrorCallbackImpl(DebugCallback callback)
{
    _errorCallback = callback;

    if (_queuedErrors.size() > 0)
    {
        FlushErrors();
    }
}

void Debug::LogImpl(string & message)
{
    if (_logCallback)
    {
        _logCallback(message.c_str());
    }
    else if(_cacheLogs)
    {
        _queuedLogs.push(message);
    }

    fprintf(stdout, message.append("\n").c_str());
}

void Debug::LogWarningImpl(string & message)
{
    if (_warningCallback)
    {
        _warningCallback(message.c_str());
    }
    else if (_cacheLogs)
    {
        _queuedWarnings.push(message);
    }

    fprintf(stdout, message.append("\n").c_str());
}

void Debug::LogErrorImpl(string & message)
{
    if (_errorCallback)
    {
        _errorCallback(message.c_str());
    }
    else if (_cacheLogs)
    {
        _queuedErrors.push(message);
    }

    fprintf(stderr, message.append("\n").c_str());
}

void Debug::FlushLogs()
{
    while (_queuedLogs.size() > 0)
    {
        LogImpl(_queuedLogs.front().insert(0, kFlushPrefix));
        _queuedLogs.pop();
    }
}

void Debug::FlushWarnings()
{
    while (_queuedWarnings.size() > 0)
    {
        LogWarningImpl(_queuedWarnings.front().insert(0, kFlushPrefix));
        _queuedWarnings.pop();
    }
}

void Debug::FlushErrors()
{
    while (_queuedErrors.size() > 0)
    {
        LogErrorImpl(_queuedErrors.front().insert(0, kFlushPrefix));
        _queuedErrors.pop();
    }
}