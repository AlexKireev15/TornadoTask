#pragma once
#include <vector>
#include <cassert>
namespace ElevatorAlgo
{
	typedef unsigned int uint;
	class Elevator final
	{
	public:
		Elevator(uint floorCount = 9u, uint currentFloor = 1u) noexcept :
			floorCount(floorCount), currentFloor(currentFloor),
			calls(floorCount, false)
		{
		}
		~Elevator() noexcept {}
		Elevator(const Elevator&) = delete;
		Elevator(Elevator&&) = delete;
		void operator=(const Elevator&) = delete;
		void operator=(Elevator&&) = delete;


		void addCall(uint floor) noexcept
		{
			assert(floor > 0 && floor <= floorCount);
			if (floor > 0 && floor <= floorCount)
				calls[floor - 1u] = true;
		}

		template<typename I, typename = typename std::enable_if<std::is_same<typename std::iterator_traits<I>::value_type, unsigned int>::value>>
		void addCalls(const I& begin, const I& end)
		{
			for (I it = begin; it < end; ++it)
			{
				assert(*it > 0 && *it <= floorCount);
				if (*it > 0 && *it <= floorCount)
					calls[*it - 1u] = true;
			}
		}

	public:
		enum Direction
		{
			STOPPED,
			UP,
			DOWN
		};

		uint getFloor() const noexcept
		{
			return currentFloor;
		}

		Direction getDirection() const noexcept
		{
			return direction;
		}

		const char* getDirectionStr() const noexcept
		{
			switch (direction)
			{
			case STOPPED:
				return "Stopped";
			case UP:
				return "Up";
			case DOWN:
				return "Down";				
			}
			return "Unknown";
		}

		std::vector<uint> getCalls() const noexcept;

		bool hasCalls() const noexcept
		{
			for (const auto& c : calls)
			{
				if (c)
					return true;
			}
			return false;
		}

		Direction doStep();

	private:
		
		bool hasCallsInDirection() const noexcept;
		void calcDirection();

	private:
		const uint floorCount = 9;
		std::vector<bool> calls;
		uint currentFloor = 1;
		Direction direction = STOPPED;
	};
}