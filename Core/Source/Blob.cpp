#include <Core/Blob.hpp>

namespace core
{
	constexpr const uint32_t C_BLOB_BLOCK_SIZE = 1024;

	Blob::Blob(const void* data, size_t size) : m_size(size)
	{
		size_t sizeToAllocate = size / C_BLOB_BLOCK_SIZE;

		if (sizeToAllocate == 0)
		{
			sizeToAllocate = C_BLOB_BLOCK_SIZE;
		}
		else
		{
			sizeToAllocate = sizeToAllocate * C_BLOB_BLOCK_SIZE;
		}

		CORE_ASSERT(sizeToAllocate >= size);

		m_data = ::operator new(sizeToAllocate);
		std::memcpy(m_data, data, size);
	}

	Blob::~Blob()
	{
		::operator delete(m_data);
	}
}
