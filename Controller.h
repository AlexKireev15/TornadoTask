#pragma once
#include <thread>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <array>
#include <list>
#include <vector>

namespace Controller
{
	enum class Error
	{
		None,
		OutOfAllocatedMemory,
		InvalidArgument
	};

	inline const char* getErrorString(Error e)
	{
		switch (e)
		{
		case Error::None:
			return "None";
		case Error::OutOfAllocatedMemory:
			return "Out Of Allocated Memory";
		case Error::InvalidArgument:
			return "Invalid Argument";
		}
		return "Unknown";
	}

	struct Stats
	{
		uint64_t bytesReceived = 0;
		uint64_t packetsReceived = 0;
		uint64_t bytesDropped = 0;
		uint64_t packetsDropped = 0;
		Error lastError = Error::None;
		Stats& operator+=(const Stats& o)
		{
			bytesReceived += o.bytesReceived;
			packetsReceived += o.packetsReceived;
			bytesDropped += o.bytesDropped;
			packetsDropped += o.packetsDropped;
			lastError = o.lastError;
			return *this;
		}
	};

	template<uint64_t bufferSize = 1 << 10>
	class alignas(256) FastBuffer final
	{
	public:
		Stats push(uint8_t* buf, uint64_t len)
		{
			Stats stats;
			if (len > bufferSize)
			{
				stats.packetsDropped += 1u;
				stats.bytesDropped += len;
				stats.lastError = Error::OutOfAllocatedMemory;
				return stats;
			}

			uint64_t head = m_head.load(std::memory_order_relaxed);
			uint64_t tail = m_tail.load(std::memory_order_acquire);
			uint64_t used = head - tail;
			uint64_t free = bufferSize - used;

			if (len > free)
			{
				stats.packetsDropped += 1u;
				stats.bytesDropped += len;
				stats.lastError = Error::OutOfAllocatedMemory;
				return stats;
			}

			std::memcpy(m_buffer.data() + used, buf, len);
			{
				stats.packetsReceived += 1u;
				stats.bytesReceived += len;
			}
			m_head.store(head + len, std::memory_order_release);
			return stats;
		}

		std::vector<uint8_t> get()
		{
			uint64_t head = m_head.load(std::memory_order_acquire);
			uint64_t tail = m_tail.load(std::memory_order_relaxed);

			uint64_t available = head - tail;
			if (available == 0u) return {};
			std::vector<uint8_t> res; res.resize(available);
			std::memcpy(res.data(), m_buffer.data(), available);

			m_tail.store(head, std::memory_order_release);
			return res;
		}

		float getBufferFullness() const
		{
			return (float)(m_head.load(std::memory_order_acquire) - m_tail.load(std::memory_order_acquire)) / bufferSize;
		}

	private:
		alignas(128) std::array<uint8_t, bufferSize> m_buffer{};
		struct alignas(64) Atomic64
		{
			std::atomic<uint64_t> v = 0;
			uint64_t load(std::memory_order mo) const
			{
				return v.load(mo);
			}
			void store(uint64_t val, std::memory_order mo)
			{
				return v.store(val, mo);
			}
		};
		Atomic64 m_head;
		uint8_t pad1[64]{};
		Atomic64 m_tail;
		uint8_t pad2[64]{};
	};

	class DataCollector final
	{
	public:
		void collect(FastBuffer<>& buffer);
		bool hasData() const
		{
			return accumulatedSize != 0u;
		}
		std::vector<uint8_t> get(uint64_t size);
	private:
		std::list<std::vector<uint8_t>> m_data;
		uint64_t accumulatedSize = 0u;
	};

	class alignas(256) Controller final
	{
	public:
		Controller()
		{
			m_collectorThread = std::thread(&Controller::collect, this);
		}
		~Controller()
		{
			m_stop.exchange(true);
			m_dataCondition.notify_all();
			m_collectorThread.join();
		}
		void recv(uint8_t* buf, uint64_t len);
		std::vector<uint8_t> get(uint64_t len);
		void collect();
		Stats getStats() const
		{
			return m_stats;
		}
	private:
		Stats m_stats;
		FastBuffer<> m_buffer;
		DataCollector m_collector;

		std::thread m_collectorThread;
		std::mutex m_dataMutex;
		std::condition_variable m_dataCondition;

		std::atomic_bool m_stop = false;
		std::atomic_bool m_collectorWasNotified = false;

	};

}