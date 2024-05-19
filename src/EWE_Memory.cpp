#include "EWEngine/Data/EWE_Memory.h"


#ifdef _DEBUG
#include <string>
#include <fstream>
#include <unordered_map>

static constexpr const char* memoryLogPath{ "memoryLog.log" };
bool notFirstMalloc = false;


#endif
#include <cstdlib>
#include <cstring>

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
		std::memcpy(file, f, eof);

		eof = 0;
		while (sourceFunction[eof] != '\0' && eof < 256) {
			eof++;
		}
		eof++;
		std::memcpy(this->sourceFunction, sourceFunction, eof);
	}
};
#ifdef _DEBUG
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
#endif


#if USING_MALLOC
void* ewe_alloc_internal(std::size_t element_size, std::size_t element_count, const char* file, int line, const char* sourceFunction) {
	void* ptr = malloc(element_count * element_size);
#ifdef _DEBUG
	mallocMap.try_emplace(reinterpret_cast<uint64_t>(ptr), file, line, sourceFunction);

	updateMemoryLogFile();
#endif
	return ptr;
}
#else
void ewe_alloc_internal(void* ptr, const char* file, int line, const char* sourceFunction){
#ifdef _DEBUG
	mallocMap.try_emplace(reinterpret_cast<uint64_t>(ptr), file, line, sourceFunction);
	updateMemoryLogFile();
#endif
}
#endif

void ewe_free_internal(void* ptr) {
#if USING_MALLOC
	free(ptr);
#endif
#ifdef _DEBUG
	auto found = mallocMap.find(reinterpret_cast<uint64_t>(ptr));
	assert((found != mallocMap.end()) && "freeing memory that wasn't allocated");
	mallocMap.erase(found);

	updateMemoryLogFile();
#endif
}