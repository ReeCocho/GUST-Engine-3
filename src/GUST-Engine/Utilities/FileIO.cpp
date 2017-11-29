#include <fstream>
#include <assert.h>
#include "FileIO.hpp"

namespace gust
{
	std::string readText(const std::string& path)
	{
		std::string contents = "";
		std::string line = "";

		// Open stream
		std::ifstream stream(path);
		assert(stream.is_open());

		// Read contents and dump them into the contents stream
		while (std::getline(stream, line))
			contents += line + '\n';

		return contents;

	}

	void writeText(const std::string& path, const std::string& str)
	{
		// Open stream
		std::ofstream stream(path);
		assert(stream.is_open());

		// Write
		stream << str;
	}

	std::vector<unsigned char> readBinary(const std::string& path)
	{
		// Open stream
		std::ifstream stream(path, std::ios::binary | std::ios::in | std::ios::ate);
		assert(stream.is_open());

		// Get ammount of data in the stream
		size_t size = stream.tellg();

		// Go back to the beginning
		stream.seekg(0, std::ios::beg);

		// Read data
		char* data = nullptr;
		stream.read(data, size);

		// Put the data into a vector
		std::vector<unsigned char> vectorData(size);
		for (size_t i = 0; i < size; i++)
			vectorData[i] = static_cast<unsigned char>(data[i]);

		return vectorData;
	}

	void writeBinary(const std::string& path, const std::vector<unsigned char>& bytes)
	{
		// Open stream
		std::ofstream stream(path, std::ios::binary | std::ios::out);
		assert(stream.is_open());

		// Bytes to char
		std::vector<char> chars(bytes.size());
		for (size_t i = 0; i < bytes.size(); i++)
			chars[i] = static_cast<char>(bytes[i]);

		// Write data
		stream.write(chars.data(), chars.size());
	}
}