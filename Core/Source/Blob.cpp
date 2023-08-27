#include <Core/Blob.hpp>

namespace core
{
	Blob::Blob(const void* data, uint32_t size)
	{
		const uint32_t sizeToAllocate = core::math::roundUpToNextHighestPowerOfTwo(size);

		CORE_ASSERT(sizeToAllocate >= size);

		m_data.reserve(sizeToAllocate);
		m_data.resize(size);
		std::memcpy(m_data.data(), data, size);
	}

	Blob::~Blob()
	{}
}
