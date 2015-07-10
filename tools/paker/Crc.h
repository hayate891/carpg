#pragma once

const uint CRC32_NEGL = 0xffffffffL;

#ifdef IS_LITTLE_ENDIAN
#define CRC32_INDEX(c) (c & 0xff)
#define CRC32_SHIFTED(c) (c >> 8)
#else
#define CRC32_INDEX(c) (c >> 24)
#define CRC32_SHIFTED(c) (c << 8)
#endif

class CRC32
{
public:
	inline CRC32()
	{
		m_crc = CRC32_NEGL;
	}
	void Update(const byte *input, size_t length);
	inline uint Get()
	{
		return m_crc;
	}

	template<typename T>
	inline void Update(T& item)
	{
		Update((const byte*)&item, sizeof(item));
	}

	template<>
	inline void Update(string& str)
	{
		if(!str.empty())
			Update((const byte*)str.c_str(), str.length());
	}

private:
	static const uint m_tab[256];
	uint m_crc;
};
