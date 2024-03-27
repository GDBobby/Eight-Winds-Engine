#include "EWEngine/Data/EWE_Memory.h"


#ifdef _DEBUG
#include <string>
#include <fstream>
#include <unordered_map>

static constexpr const char* memoryLogPath{ "memoryLog.log" };
bool notFirstMalloc = false;


#endif

struct MallocTracker {
	char file[256];
	int line;
	char sourceFunction[256];
	MallocTracker(const char* f, int l, const char* sourceFunction) : line(l)
	
	{
		uint64_t eof = 0;

		while (f[eof] != '\0' && eof < 256) {
			eof++;
		}
		eof++;
		memcpy(file, f, eof);

		eof = 0;
		while (sourceFunction[eof] != '\0' && eof < 256) {
			eof++;
		}
		eof++;
		memcpy(this->sourceFunction, sourceFunction, eof);
	}
};
std::unordered_map<uint64_t, MallocTracker> mallocMap{};

void updateMemoryLogFile() {
	std::ofstream memoryLogFile{};
	if (mallocMap.size() == 0) {
		memoryLogFile.open(memoryLogPath, std::ofstream::out | std::ofstream::trunc);
		memoryLogFile.seekp(std::ios::beg); //i dont think this is necessary
		memoryLogFile << "empty, no mem leaks";
		memoryLogFile.close();
	}
	else {

		std::vector<std::string> memoryLogCopy(mallocMap.size());

		std::string temp;
		for (auto& memPiece : mallocMap) {
			temp.clear();
			temp = "\nfile:{line} - ";
			temp += memPiece.second.file;
			temp += ":{";
			temp += std::to_string(memPiece.second.line);
			temp += "}\n\t";
			temp += memPiece.second.sourceFunction;
			temp += '\n';
			memoryLogCopy.push_back(temp);
		}
		memoryLogFile.open(memoryLogPath, std::ofstream::out | std::ofstream::trunc);
		memoryLogFile.seekp(std::ios::beg); //i dont think this is necessary
		for (auto const& logCopy : memoryLogCopy) {
			memoryLogFile << logCopy;
		}
		memoryLogFile.close();
	}

}

void* ewe_alloc_internal(size_t element_size, size_t element_count, const char* file, int line, const char* sourceFunction) {
	void* ptr = malloc(element_count * element_size);

	mallocMap.try_emplace(reinterpret_cast<uint64_t>(ptr), file, line, sourceFunction);

	updateMemoryLogFile();
	return ptr;
}

void ewe_free_internal(void* ptr) {
	auto found = mallocMap.find(reinterpret_cast<uint64_t>(ptr));
	assert((found != mallocMap.end()) && "freeing memory that wasn't allocated");
	mallocMap.erase(found);

	updateMemoryLogFile();
}