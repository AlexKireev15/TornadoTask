#pragma once
#include <iostream>
namespace Controller
{
	struct Stats;
}
namespace ControllerTest
{
#ifdef _WIN32
	static void printMemInfo(std::ostream& out = std::cout);

#endif
	static void printStats(Controller::Stats st, std::ostream& out = std::cout);
	int runTest();
};