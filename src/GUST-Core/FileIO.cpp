#include <fstream>
#include "Debugging.hpp"
#include "FileIO.hpp"

namespace gust
{
	std::string readText(const std::string& path)
	{
		std::string contents = "";
		std::string line = "";

		// Open stream
		std::ifstream stream(path);
		gAssert(stream.is_open());

		// Read contents and dump them into the contents stream
		while (std::getline(stream, line))
			contents += line + '\n';

		return contents;

	}

	void writeText(const std::string& path, const std::string& str)
	{
		// Open stream
		std::ofstream stream(path);
		gAssert(stream.is_open());

		// Write
		stream << str;
	}

	std::vector<char> readBinary(const std::string& path)
	{
		// Open stream
		std::ifstream stream(path, std::ios::binary | std::ios::in | std::ios::ate);
		gAssert(stream.is_open());

		// Get ammount of data in the stream
		size_t size = stream.tellg();

		// Go back to the beginning
		stream.seekg(0);

		// Read data
		std::vector<char> vectorData(size);
		stream.read(vectorData.data(), size);

		return vectorData;
	}

	void writeBinary(const std::string& path, const std::vector<char>& bytes)
	{
		// Open stream
		std::ofstream stream(path, std::ios::binary | std::ios::out);
		gAssert(stream.is_open());

		// Write data
		stream.write(bytes.data(), bytes.size());
	}
}