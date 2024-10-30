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

    bool isSuspended = previousSuspendCount == 0;

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
    
    return QString::number(cpuTime) + "s";
}

QString GetThreadCpuLoad(HANDLE thread)
{
    int intervalMs = 1000;
    FILETIME creationTime, exitTime, kernelTime1, userTime1;
    FILETIME kernelTime2, userTime2;

    if (!GetThreadTimes(thread, &creationTime, &exitTime, &kernelTime1, &userTime1))
    {
        return "-";
    }

    Sleep(intervalMs);

    if (!GetThreadTimes(thread, &creationTime, &exitTime, &kernelTime2, &userTime2))
    {
        return "-";
    }

    ULONGLONG kernelTimeMs1 = ((ULONGLONG)kernelTime1.dwHighDateTime << 32) | kernelTime1.dwLowDateTime;
    ULONGLONG userTimeMs1 = ((ULONGLONG)userTime1.dwHighDateTime << 32) | userTime1.dwLowDateTime;
    ULONGLONG kernelTimeMs2 = ((ULONGLONG)kernelTime2.dwHighDateTime << 32) | kernelTime2.dwLowDateTime;
    ULONGLONG userTimeMs2 = ((ULONGLONG)userTime2.dwHighDateTime << 32) | userTime2.dwLowDateTime;

    ULONGLONG kernelDeltaMs = kernelTimeMs2 - kernelTimeMs1;
    ULONGLONG userDeltaMs = userTimeMs2 - userTimeMs1;
    ULONGLONG totalDeltaMs = kernelDeltaMs + userDeltaMs;

    double cpuLoad = (totalDeltaMs / (double)(intervalMs * 10000)) * 100.0;


    return QString::number(cpuLoad) + "%";
}
