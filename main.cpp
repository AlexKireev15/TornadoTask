#include <iostream>

#include "UnitTest.h"
#include "ControllerTest.h"

int main()
{
	UnitTest::runAllTests();
	char ch; std::cin >> ch;
	ControllerTest::runTest();
	return 0;
}