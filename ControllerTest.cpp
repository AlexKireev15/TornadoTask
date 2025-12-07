#include "ControllerTest.h"
#include "Controller.h"

#include <chrono>
#include <iomanip>

#include <fstream>

#include <cstdlib>

static uint32_t consumerPacketsCount = 0;
static uint64_t consumerBytesCount = 0;

static float lastRecvDuration = 0ll;
static float maxRecvDuration = 0ll;
static float recvDurationSum = 0ll;

constexpr long long producerTiming = 500;
constexpr long long consumerTiming = 100;
constexpr long long consoleTiming = 10;
constexpr long long bytesToRecieve = 8;

#ifdef _WIN32
#include <windows.h>
#include "psapi.h"

static void ControllerTest::printMemInfo(std::ostream& out)
{
	MEMORYSTATUSEX memInfo;
	memInfo.dwLength = sizeof(MEMORYSTATUSEX);
	GlobalMemoryStatusEx(&memInfo);
	DWORDLONG totalVirtualMem = memInfo.ullTotalPageFile;
	DWORDLONG virtualMemUsed = memInfo.ullTotalPageFile - memInfo.ullAvailPageFile;
	DWORDLONG totalPhysMem = memInfo.ullTotalPhys;
	DWORDLONG physMemUsed = memInfo.ullTotalPhys - memInfo.ullAvailPhys;

	PROCESS_MEMORY_COUNTERS_EX pmc;
	GetProcessMemoryInfo(GetCurrentProcess(), (PROCESS_MEMORY_COUNTERS*)&pmc, sizeof(pmc));
	SIZE_T virtualMemUsedByMe = pmc.PrivateUsage;
	SIZE_T physMemUsedByMe = pmc.WorkingSetSize;

	out << std::setw(20) << "Total VMem: " << totalVirtualMem << " bytes" << std::endl;
	out << std::setw(20) << "Used VMem: " << virtualMemUsed << " bytes" << std::endl;
	out << std::setw(20) << "Used by me VMem: " << virtualMemUsedByMe << " bytes" << std::endl;
	out << std::setw(20) << "Total Mem: " << totalPhysMem << " bytes" << std::endl;
	out << std::setw(20) << "Used Mem: " << physMemUsed << " bytes" << std::endl;
	out << std::setw(20) << "Used by me Mem: " << physMemUsedByMe << " bytes" << std::endl;
}

#endif

void ControllerTest::printStats(Controller::Stats st, std::ostream& out)
{
	if (&out == &std::cout)
	{
		out << "\u001B[12A" << std::flush;
#ifdef _WIN32
		out << "\u001B[6A" << std::flush;
#endif
	}
	out << std::setiosflags(std::ios::left);
	out << std::setw(27) << "Controller " << std::setw(15) << std::endl;
	out << std::setw(20) << "Controller Packets Received: " << std::setw(15) << st.packetsReceived << std::endl;;
	out << std::setw(20) << "Controller Bytes Received: " << std::setw(15) << st.bytesReceived << std::endl;;
	out << std::setw(20) << "Controller Packets Dropped: " << std::setw(15) << st.packetsDropped << std::endl;;
	out << std::setw(20) << "Controller Bytes Dropped: " << std::setw(15) << st.bytesDropped << std::endl;;
	out << std::setw(20) << "Controller Last Error: " << std::setw(15) << getErrorString(st.lastError) << std::endl;;
	out << std::setw(20) << "Controller Last Recv Duration: " << lastRecvDuration << std::setw(9) << " ms" << std::endl;;
	out << std::setw(20) << "Controller Max Recv Duration: " << maxRecvDuration << std::setw(9) << " ms" << std::endl;;
	out << std::setw(20) << "Controller Avg Recv Duration: " << ((st.packetsReceived + st.packetsDropped) == 0 ? 0.f : (float)recvDurationSum / (float)(st.packetsReceived + st.packetsDropped)) << std::setw(5) << " ms" << std::endl;;


	out << std::setw(27) << "Consumer " << std::endl;;
	out << std::setw(20) << "Consumer Packets Received: " << std::setw(15) << consumerPacketsCount << std::endl;;
	out << std::setw(20) << "Consumer Bytes Received: " << std::setw(15) << consumerBytesCount << std::endl;;

#ifdef _WIN32
	printMemInfo(out);
#endif
}

int ControllerTest::runTest()
{
	Controller::Controller c;
	std::mutex quitMutex;
	std::condition_variable quitCond;
	bool isQuit = false;
	std::thread dummyProducer([&c, &quitMutex, &quitCond, &isQuit]
		{
			std::array<uint8_t, (1 << 8)> arr;
			for (int i = 0; i < arr.size(); i++)
			{
				arr[i] = i;
			}

			while (!isQuit)
			{
				std::unique_lock<std::mutex> ul(quitMutex);
				quitCond.wait_for(ul, std::chrono::high_resolution_clock::duration(std::chrono::milliseconds(producerTiming)));
				ul.unlock();
				if (isQuit)
				{
					return;
				}


				auto before = std::chrono::high_resolution_clock::now();
				c.recv(arr.data(), 10 + (std::rand() % (arr.size())));
				auto after = std::chrono::high_resolution_clock::now();
				lastRecvDuration = (float)(after - before).count() * 1e-6f;
				recvDurationSum += lastRecvDuration;
				if (lastRecvDuration > maxRecvDuration)
					maxRecvDuration = lastRecvDuration;
			}
		});

	std::thread dummyConsumer([&c, &quitMutex, &quitCond, &isQuit]
		{

			while (!isQuit)
			{
				std::unique_lock<std::mutex> ul(quitMutex);
				quitCond.wait_for(ul, std::chrono::high_resolution_clock::duration(std::chrono::milliseconds(consumerTiming)));
				ul.unlock();
				if (isQuit)
				{
					return;
				}
				auto buff = c.get(bytesToRecieve);

				consumerPacketsCount++;
				consumerBytesCount += buff.size();
			}
		});

	std::thread dummyConsole([&c, &quitMutex, &quitCond, &isQuit]
		{

			while (!isQuit)
			{
				std::unique_lock<std::mutex> ul(quitMutex);
				quitCond.wait_for(ul, std::chrono::high_resolution_clock::duration(std::chrono::milliseconds(consoleTiming)));
				ul.unlock();
				if (isQuit)
				{
					return;
				}

				auto st = c.getStats();
				printStats(st);

			}
		});

	while (!isQuit)
	{
		char ch;
		std::cin >> ch;
		if (ch == 'q')
		{
			isQuit = true;
			quitCond.notify_all();
			dummyProducer.join();
			dummyConsumer.join();
			dummyConsole.join();
			printMemInfo();


			return 0;
		}
		std::cout << std::endl;
	}
	return 0;
}