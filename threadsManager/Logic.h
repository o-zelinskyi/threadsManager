#include <qstring.h>
#include <Windows.h>

void randomize(std::vector<int>& vec);
bool IsThreadRunning(HANDLE thread);
bool IsThreadSuspended(HANDLE thread);
QString getStatusQString(HANDLE thread);
QString GetThreadCpuTime(HANDLE threadHandle);
