#include "ElevatorAlgo.h"

std::vector<ElevatorAlgo::uint> ElevatorAlgo::Elevator::getCalls() const noexcept
{
	std::vector<uint> res;
	res.reserve(calls.size());
	for (uint idx = 0; idx < calls.size(); ++idx)
	{
		if (calls[idx])
			res.push_back(idx + 1);
	}
	return res;
}

ElevatorAlgo::Elevator::Direction ElevatorAlgo::Elevator::doStep()
{
	switch (direction)
	{
	case STOPPED:
		break;
	case UP:
		if (currentFloor < floorCount)
			currentFloor++;
		break;
	case DOWN:
		if (currentFloor > 1)
			currentFloor--;
		break;
	}
	assert(currentFloor > 0 && currentFloor <= floorCount);
	calls[currentFloor - 1] = false;
	calcDirection();
	return direction;
}

bool ElevatorAlgo::Elevator::hasCallsInDirection() const noexcept
{
	for (uint idx = 0u; idx < calls.size(); ++idx)
	{
		if (!calls[idx])
			continue;
		const uint callFloor = idx + 1;
		const Direction callFloorDirection = callFloor > currentFloor ? UP : DOWN;
		if (callFloorDirection == direction)
			return true;
	}
	return false;
}

void ElevatorAlgo::Elevator::calcDirection()
{
	if (!hasCalls())
	{
		direction = STOPPED;
		return;
	}
	if (hasCallsInDirection())
		return;

	for (uint idx = 0u; idx < calls.size(); ++idx)
	{
		if (!calls[idx])
			continue;
		const uint callFloor = idx + 1;
		const Direction callFloorDirection = callFloor > currentFloor ? UP : DOWN;

		direction = callFloorDirection;
		break;
	}
	return;
}
