#include "UnitTest.h"

#include "ElevatorAlgo.h"

#include <chrono>
#include <functional>
#include <sstream>

#define SILENCE

static void printElevatorState(const ElevatorAlgo::Elevator& e, std::ostream& out = std::cout, const std::string& stateDelimeter = std::string(16, '-'))
{
#ifdef SILENCE
	return;
#else
	out << "Floor: " << e.getFloor() << " | Direction: " << e.getDirectionStr() << " | Calls: " << std::endl;
	auto calls = e.getCalls();
	for (const auto& c : calls)
	{
		out << c << " ";
	}
	out << std::endl << stateDelimeter << stateDelimeter << std::endl;
#endif // SILENCE
}

static bool test(ElevatorAlgo::Elevator& e, const UnitTest::Test& test, std::ostream& out, std::ostream& debugInfoOut = std::cout)
{
	try
	{
		auto startTime = std::chrono::high_resolution_clock::now();
		std::chrono::seconds timeout(1);
		unsigned int step = 0u;
		while (test.maxStep >= step || e.hasCalls())
		{
#if __cplusplus >= 201703L
			if (auto t = test.stepsCalls.find(step); t != test.stepsCalls.end())
			{
				e.addCalls(t->second.begin(), t->second.end());
			}
#else
			{
				auto t = test.stepsCalls.find(step);
				if (t != test.stepsCalls.end())
				{
					e.addCalls(t->second.begin(), t->second.end());
				}
			}
#endif
			printElevatorState(e, debugInfoOut);
			e.doStep();
			step++;
			auto time = std::chrono::high_resolution_clock::now();
			auto dummy = time - startTime;
			if (time - startTime > timeout)
			{
				out << "Test failed due to timeout" << std::endl;
				return false;
			}
		}
		printElevatorState(e, debugInfoOut);
	
		if (e.hasCalls())
		{
			out << "Test failed! Unprocessed calls left." << std::endl;
			return false;
		}
	}
	catch (const std::exception& e) 
	{
		out << "Test failed due to exception raised: " << e.what() << std::endl;
		return false;
	}
	return true;
}

static bool t1(std::ostream& out)
{
	ElevatorAlgo::Elevator e;
	UnitTest::Test t =
	{
		{
			{0, {9}},
			{5, {1}},
			{9, {9}},
		}
	};
	return test(e, t, out);
}

static bool t2(std::ostream& out)
{
	ElevatorAlgo::Elevator e;
	UnitTest::Test t =
	{
		{
			{0, {9}},
			{5, {1}},
			{9, {9}},
		}
	};
	return test(e, t, out);
}

static bool t3(std::ostream& out)
{
	ElevatorAlgo::Elevator e;
	UnitTest::Test t =
	{
		{
			{0, {1, 2, 3, 4, 5, 6, 7, 8, 9}},
			{1, {1, 2, 3, 4, 5, 6, 7, 8, 9}},
			{2, {1, 2, 3, 4, 5, 6, 7, 8, 9}},
			{3, {1, 2, 3, 4, 5, 6, 7, 8, 9}},
			{4, {1, 2, 3, 4, 5, 6, 7, 8, 9}},
			{5, {1, 2, 3, 4, 5, 6, 7, 8, 9}},
			{6, {1, 2, 3, 4, 5, 6, 7, 8, 9}},
			{7, {1, 2, 3, 4, 5, 6, 7, 8, 9}},
			{9, {1, 2, 3, 4, 5, 6, 7, 8, 9}},
			{10, {1, 2, 3, 4, 5, 6, 7, 8, 9}},
			{11, {1, 2, 3, 4, 5, 6, 7, 8, 9}},
		}
	};
	return test(e, t, out);
}

static bool t4(std::ostream& out)
{
	ElevatorAlgo::Elevator e;
	UnitTest::Test t =
	{
		{
			
		}
	};
	return test(e, t, out);
}

static bool t5(std::ostream& out)
{
	ElevatorAlgo::Elevator e;
	UnitTest::Test t =
	{
		{
			{0, {1}},
			{1, {2}},
			{2, {3}},
			{3, {4}},
			{4, {5}},
			{5, {6}},
			{6, {7}},
			{7, {8}},
			{8, {9}},
		}
	};
	return test(e, t, out);
}

bool UnitTest::runAllTests(std::ostream& out)
{
	out << "Running all tests: " << std::endl;
	std::vector<std::function<bool(std::ostream&)>> tests {t1, t2, t3, t4, t5};
	std::vector<bool> testRes(tests.size(), false);
	for (uint idx = 0u; idx < tests.size(); ++idx)
	{
		std::stringstream ss;
		testRes[idx] = tests[idx](ss);
		out << "Test " << idx + 1 << " : " << (testRes[idx] ? "Success" : "Failed") << std::endl;
		if (!testRes[idx])
		{
			out << '\t' << "Cause: " << ss.str() << std::endl << std::endl;
		}
	}
	bool res = true;
	for (const auto& r : testRes)
	{
		res = res && r;
	}
	return res;
}
