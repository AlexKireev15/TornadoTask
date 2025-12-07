#pragma once
#include <vector>
#include <map>
#include <iostream>

namespace UnitTest
{
	typedef unsigned int uint;
	struct Test
	{
		std::map<uint, std::vector<uint>> stepsCalls;
		uint maxStep = 0u;
		Test(std::map<uint, std::vector<uint>>&& test) noexcept : stepsCalls(test)
		{
			for (auto it = test.begin(); it != test.end(); ++it)
			{
				const auto& k = it->first;
				if (maxStep < k)
					maxStep = k;
			}
		}
	};

	bool runAllTests(std::ostream& out = std::cout);
}