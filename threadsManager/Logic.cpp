#include "Logic.h"

void randomize(std::vector<int>& vec)
{
    std::srand(std::time(0));

    for (int i = 0; i < vec.size(); ++i)
    {
        vec[i] = std::rand() % 100;
    }
}

bool IsThreadSuspended(HANDLE thread)
{
    DWORD previousSuspendCount = SuspendThread(thread);
    if (previousSuspendCount == -1)
    {
        return false;
    }

    bool isSuspended = previousSuspendCount != 0;

    ResumeThread(thread);

    return isSuspended;
}


bool IsThreadRunning(HANDLE thread)
{
    DWORD exitCode;
    if (GetExitCodeThread(thread, &exitCode))
    {
        if (exitCode == STILL_ACTIVE)
        {
            return true;
        }
        else
        {
            return false;
        }
    }
    else
    {
        return false;
    }
}

QString getStatusQString(HANDLE thread)
{
    return IsThreadSuspended(thread) ? "Running :)" : "Suspended Zzz";
}

QString GetThreadCpuTime(HANDLE threadHandle)
{
    FILETIME creationTime, exitTime, kernelTime, userTime;

    if (!GetThreadTimes(threadHandle, &creationTime, &exitTime, &kernelTime, &userTime))
    {
        return "-";
    }

    ULONGLONG kernelTimeMs = ((ULONGLONG)kernelTime.dwHighDateTime << 32) | kernelTime.dwLowDateTime;
    ULONGLONG userTimeMs = ((ULONGLONG)userTime.dwHighDateTime << 32) | userTime.dwLowDateTime;

    ULONGLONG totalTimeMs = kernelTimeMs + userTimeMs;

    double cpuTime = (totalTimeMs / 10000.0);
    
    return QString::number(cpuTime) + "ms";
}