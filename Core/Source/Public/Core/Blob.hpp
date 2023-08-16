#pragma once

#include <Core/Config.hpp>

namespace core
{
	//Just a buffer of plain binary data.
	//Doesn't take ownership of the pointer, allocates new blocks of memory for the data.
	class CORE_API Blob
	{
	public:
		Blob() = default;
		Blob(const void* data, size_t size);
		~Blob();

		size_t		size() const { return m_size; }
		const void* data() const { return m_data; }

	private:
		size_t		m_size = 0;
		void*		m_data = nullptr;
	};
}
