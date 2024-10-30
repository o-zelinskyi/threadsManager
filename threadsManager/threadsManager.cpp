#include "threadsManager.h"
#include <algorithm>

using std::vector;

std::vector<int> vec(Size);
GlobalMin globalMin{ vec[0], 0 };
vector<HANDLE> threads;
vector<ThreadData> threadData;

threadsManager::threadsManager(QWidget *parent)
    : QMainWindow(parent)
    , timer(new QTimer(this))
    , countdownTimer(new QTimer(this))
{
    ui.setupUi(this);

    randomize(vec);

    connect(timer, &QTimer::timeout, this, &threadsManager::updateTable);
    timer->start(10000);

    connect(countdownTimer, &QTimer::timeout, [this]()
        {
            static int counter = 10;
            ui.refreshLabel->setText("Time to update: " + QString::number(counter--) + " seconds");
            if (counter < 0)
            {
                counter = 10;
            }
        });
    countdownTimer->start(1000);

    connect(ui.reloadButton, &QPushButton::clicked, this, &threadsManager::on_reloadButton_clicked);
    connect(ui.runButton, &QPushButton::clicked, this, &threadsManager::on_runButton_clicked);
	connect(ui.suspendButton, &QPushButton::clicked, this, &threadsManager::on_suspendButton_clicked);
	connect(ui.resumeButton, &QPushButton::clicked, this, &threadsManager::on_resumeButton_clicked);
	connect(ui.priorityComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &threadsManager::on_priorityComboBox_currentIndexChanged);
	connect(ui.killButton, &QPushButton::clicked, this, &threadsManager::on_killButton_clicked);
}

threadsManager::~threadsManager()
{
    terminateAllThreads();
}

void threadsManager::terminateAllThreads()
{
    for (const auto& thread : threads)
    {
        if (IsThreadRunning(thread))
        {
            TerminateThread(thread, NULL);
        }
        CloseHandle(thread);
    }
    threads.clear();
    threadData.clear();
}

void threadsManager::on_reloadButton_clicked()
{
    threadsManager::updateTable();
}

void threadsManager::insertProcessToTable(const HANDLE thread)
{
    int rowCount = ui.tableWidget->rowCount();
    ui.tableWidget->insertRow(rowCount);

    QTableWidgetItem* threadIdItem = new QTableWidgetItem(QString::number(GetThreadId(thread)));
    ui.tableWidget->setItem(rowCount, 0, threadIdItem);

    QTableWidgetItem* statusItem = new QTableWidgetItem(getStatusQString(thread));
    ui.tableWidget->setItem(rowCount, 1, statusItem);

    QTableWidgetItem* priorityItem = new QTableWidgetItem(QString::number(GetThreadPriority(thread)));
    ui.tableWidget->setItem(rowCount, 2, priorityItem);

    QTableWidgetItem* cpuItem = new QTableWidgetItem(GetThreadCpuTime(thread));
    ui.tableWidget->setItem(rowCount, 3, cpuItem);

    QTableWidgetItem* cpuLoadItem = new QTableWidgetItem(GetThreadCpuLoad(thread));
    ui.tableWidget->setItem(rowCount, 4, cpuLoadItem);
}

void threadsManager::updateTable()
{
    ui.tableWidget->setRowCount(0);

    for (size_t i = 0; i < threads.size();)
    {
        if (IsThreadRunning(threads[i]))
        {
            insertProcessToTable(threads[i]);
            ++i;
        } else
        {
            CloseHandle(threads[i]);
            threads.erase(threads.begin() + i);
        }
    }
}

DWORD WINAPI linearSearchMin(LPVOID lp)
{
    ThreadData* data = (ThreadData*)lp;
    int minElement = (*data->arr)[data->start];
    int minIndex = data->start;

    for (int i = data->start; i < data->end; ++i)
    {
        if ((*data->arr)[i] < minElement)
        {
            minElement = (*data->arr)[i];
            minIndex = i;
        }
    }

    data->minElement = minElement;
    data->minIndex = minIndex;

    return 0;
}

void updateGlobalMin(int localMinElement, int localMinIndex)
{
    if (localMinElement < globalMin.Element)
    {
        globalMin.Element = localMinElement;
        globalMin.Index = localMinIndex;
    }
}

void threadsManager::launchThreads(int method, int numOfThreads)
{
    threads.resize(numOfThreads);
    threadData.resize(numOfThreads);

    int chunkSize = Size / numOfThreads;

    
    globalMin.Element = vec[0];
    globalMin.Index = 0;

    if (method == 0)
    {
        HANDLE mutex = CreateMutex(NULL, FALSE, NULL);

        for (int i = 0; i < numOfThreads; ++i)
        {
            threadData[i].arr = &vec;
            threadData[i].start = i * chunkSize;
            threadData[i].end = (i == numOfThreads - 1) ? Size : (i + 1) * chunkSize;
            threadData[i].minElement = (*threadData[i].arr)[threadData[i].start];
            threadData[i].minIndex = threadData[i].start;

            
            threads[i] = CreateThread(NULL, 0, linearSearchMin, &threadData[i], CREATE_SUSPENDED, 0);
        }

        WaitForMultipleObjects(numOfThreads, threads.data(), TRUE, INFINITE);
        for (int i = 0; i < numOfThreads; ++i)
        {
            WaitForSingleObject(mutex, INFINITE);
            updateGlobalMin(threadData[i].minElement, threadData[i].minIndex);
            ReleaseMutex(mutex);
        }

        threads.clear();
        CloseHandle(mutex);

    } else if (method == 1)
    {
        HANDLE semaphore = CreateSemaphore(NULL, 1, 1, NULL);

        for (int i = 0; i < numOfThreads; ++i)
        {
            threadData[i].arr = &vec;
            threadData[i].start = i * chunkSize;
            threadData[i].end = (i == numOfThreads - 1) ? Size : (i + 1) * chunkSize;
            threadData[i].minElement = (*threadData[i].arr)[threadData[i].start];
            threadData[i].minIndex = threadData[i].start;

            threads[i] = CreateThread(NULL, 0, linearSearchMin, &threadData[i], 0, 0);
        }

        WaitForMultipleObjects(numOfThreads, threads.data(), TRUE, INFINITE);
        for (int i = 0; i < numOfThreads; ++i)
        {
            WaitForSingleObject(semaphore, INFINITE);
            updateGlobalMin(threadData[i].minElement, threadData[i].minIndex);
            ReleaseSemaphore(semaphore, 1, NULL);
        }
        threads.clear();
        CloseHandle(semaphore);

    }
}

DWORD WINAPI threadsManager::launchThreadsWrapper(LPVOID param)
{
    threadsManager* tm = (threadsManager*)param;
    int numOfThreads = 1;
    int index = tm->ui.numOfThreadsComboBox->currentIndex();
    int method = tm->ui.chooseSyncComboBox->currentIndex();

    switch (index)
    {
    case 0:
        numOfThreads = 1;
        break;
    case 1:
        numOfThreads = 2;
        break;
    case 2:
        numOfThreads = 4;
        break;
    case 3:
        numOfThreads = 8;
        break;
    case 4:
        numOfThreads = 16;
        break;
    default:
        numOfThreads = 1;
        break;
    }
    
    tm->launchThreads(method, numOfThreads);
    return 0;
}

void threadsManager::on_runButton_clicked()
{
    HANDLE thread = CreateThread(NULL, 0, launchThreadsWrapper, (LPVOID)this, 0, NULL);

    if (thread == NULL)
    {
        QMessageBox::information(this, "err", "err");
    } else
    {
        CloseHandle(thread);
    }
}

void threadsManager::on_suspendButton_clicked()
{
    QTableWidget* tableWidget = ui.tableWidget;
    int currentRow = tableWidget->currentRow();
    if (currentRow >= 0 && currentRow < threads.size())
    {
        if (IsThreadRunning(threads[currentRow]))
        {
            SuspendThread(threads[currentRow]);
        } else
        {
            QMessageBox::warning(this, "Warning", "The process is no longer running.");
        }
    } else
    {
        QMessageBox::warning(this, "Warning", "Invalid process selection.");
    }
}

void threadsManager::on_resumeButton_clicked()
{
    QTableWidget* tableWidget = ui.tableWidget;
    int currentRow = tableWidget->currentRow();
    if (currentRow >= 0 && currentRow < threads.size())
    {
        if (IsThreadRunning(threads[currentRow]))
        {
            ResumeThread(threads[currentRow]);
        } else
        {
            QMessageBox::warning(this, "Warning", "The process is no longer running.");
        }
    } else
    {
        QMessageBox::warning(this, "Warning", "Invalid process selection.");
    }
}

void threadsManager::on_priorityComboBox_currentIndexChanged(int index)
{
    QTableWidget* tableWidget = ui.tableWidget;
    int currentRow = tableWidget->currentRow();
    if (currentRow >= 0 && currentRow < threads.size())
    {
        if (IsThreadRunning(threads[currentRow]))
        {
            int priority;
            switch (index)
            {
            case 0: priority = THREAD_PRIORITY_IDLE; break;
            case 1: priority = THREAD_PRIORITY_BELOW_NORMAL; break;
            case 2: priority = THREAD_PRIORITY_NORMAL; break;
            case 3: priority = THREAD_PRIORITY_ABOVE_NORMAL; break;
            case 4: priority = THREAD_PRIORITY_HIGHEST; break;
            case 5: priority = THREAD_PRIORITY_TIME_CRITICAL; break;
            default: priority = THREAD_PRIORITY_NORMAL; break;
            }
            if (SetThreadPriority(threads[currentRow], priority))
            {
                QMessageBox::information(this, "Success", "Thread priority set successfully.");
            } else
            {
                QMessageBox::warning(this, "Warning", "Failed to set thread priority.");
            }
        } else
        {
            QMessageBox::warning(this, "Warning", "The thread is no longer running.");
        }
    } else
    {
        QMessageBox::warning(this, "Warning", "Invalid thread selection.");
    }
}


void threadsManager::on_killButton_clicked()
{
    QTableWidget* tableWidget = ui.tableWidget;
    int currentRow = tableWidget->currentRow();
    if (currentRow >= 0 && currentRow < threads.size())
    {
        if (IsThreadRunning(threads[currentRow]))
        {
            TerminateThread(threads[currentRow], NULL);
            CloseHandle(threads[currentRow]); 
            QMessageBox::information(this, "Success", "Process terminated successfully.");
            threads.erase(threads.begin() + currentRow);
            updateTable();
        } else
        {
            QMessageBox::warning(this, "Warning", "The process is no longer running.");
        }
    } else
    {
        QMessageBox::warning(this, "Warning", "Invalid process selection.");
    }
}