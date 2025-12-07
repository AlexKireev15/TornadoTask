#include "Controller.h"

void Controller::DataCollector::collect(FastBuffer<>& buffer)
{
	auto b = buffer.get();
	if (b.empty())
		return;
	m_data.push_back(b);
	accumulatedSize += b.size();
}

std::vector<uint8_t> cutFirst(uint64_t count, std::vector<uint8_t>&& v)
{
	std::vector<uint8_t> res(v.begin() + count, v.end());
	return res;
}

std::vector<uint8_t> Controller::DataCollector::get(uint64_t size)
{
	uint64_t reqSize = size > accumulatedSize ? accumulatedSize : size;
	uint64_t capturedSize = 0u;
	std::vector<uint8_t> res; res.resize(reqSize);
	std::vector<std::list<std::vector<uint8_t>>::iterator> deleteVec; deleteVec.reserve(m_data.size());
	for (auto it = m_data.begin(); it != m_data.end(); ++it)
	{
		auto& block = *it;
		uint64_t remainingSize = reqSize - capturedSize;
		uint64_t size = std::min(block.size(), remainingSize);
		std::memcpy(res.data() + capturedSize, block.data(), size);
		capturedSize += size;
		if (size == remainingSize)
		{
			block = cutFirst(size, std::move(block));
			break;
		}
		deleteVec.push_back(it);
	}
	for (auto& it : deleteVec)
	{
		m_data.erase(it);
	}
	accumulatedSize = accumulatedSize - reqSize;
	return res;
}

void Controller::Controller::recv(uint8_t* buf, uint64_t len)
{
	m_stats += m_buffer.push(buf, len);
	if (m_buffer.getBufferFullness() >= 0.5f && m_collectorWasNotified.load(std::memory_order_acquire) == false)
	{
		m_dataCondition.notify_one();
	}
}

std::vector<uint8_t> Controller::Controller::get(uint64_t len)
{
	if (!m_collector.hasData())
	{
		m_collector.collect(m_buffer);
		if (!m_collector.hasData())
			return {};
	}
	return m_collector.get(len);

}

void Controller::Controller::collect()
{
	while (!m_stop)
	{
		m_collectorWasNotified.store(false, std::memory_order_release);
		std::unique_lock<std::mutex> ul(m_dataMutex);
		m_dataCondition.wait(ul, [this]() { return m_buffer.getBufferFullness() >= 0.5f || m_stop; });
		if (m_collectorWasNotified.load(std::memory_order_relaxed))
			m_collectorWasNotified.store(true, std::memory_order_release);
		ul.unlock();
		if (m_stop)
			return;
		m_collector.collect(m_buffer);
	}
}
