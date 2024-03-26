#ifndef _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#endif

#include <iostream>
#include <fstream>
#include <sstream>
#include <cstring>
#include <vector>
#include <set>
#include <queue>

#define ROW_LENGTH 12
#define DEFAULT_SIZE 100
#define DEFAULT_CACHE_SIZE 5

using namespace std;

//-- file directory manager --//

struct DirectoryManager;

struct DirectoryManager {
	string cwd;
	DirectoryManager(string& prDir) : cwd(prDir) {
		cwd.erase(getLastDirPos(), cwd.end());
	}
	DirectoryManager(char* prDir) : cwd(prDir) {
		cwd.erase(getLastDirPos(), cwd.end());
	}
	string::iterator getLastDirPos() {
		string::iterator strIt = prev(cwd.end());
		while (*(strIt) != '\\' && strIt != cwd.begin()) { --strIt; }
		if (strIt == cwd.begin()) { return strIt; }
		return ++strIt;
	}
};

//-- memory subsystems --//

struct BitVector;
struct Cache;
struct CacheEntry;

struct BitVector {
	string fName;
	int size;
	int capacity;
	vector<int> bits;

	static constexpr int BITS = 30;

	BitVector(int size, string fName) : size(size), fName(fName), capacity(), bits() {
		capacity = ((size / BITS) + 1);
		bits = vector<int>(capacity, 0);
		fileIn();
	}
	~BitVector() {
		fileOut();
	}
	inline int groupSelector(int index) {
		return index / BITS;
	}
	inline int bitSelector(int index) {
		return index % BITS;
	}
	void set(int index, bool value) {
		int& group = bits[groupSelector(index)];
		int mask = (int)value << bitSelector(index);
		group = (group & (~mask)) | mask;
	}
	bool get(int index) {
		return (bits[groupSelector(index)] >> bitSelector(index)) & 1;
	}
	void set(int startIndex, int endIndex, bool value) {
		if (groupSelector(startIndex) == groupSelector(endIndex)) {
			for (int indexIt = startIndex; indexIt <= endIndex; ++indexIt) {
				set(indexIt, value);
			}
		}
		else {
			int startGroupEndIndex = ((groupSelector(startIndex) + 1) * BITS);
			int endGroupStartIndex = (groupSelector(endIndex) * BITS);
			for (int indexIt = startIndex; indexIt <= startGroupEndIndex; ++indexIt) {
				set(indexIt, value);
			}
			for (int indexIt = endGroupStartIndex; indexIt <= endIndex; ++indexIt) {
				set(indexIt, value);
			}
			int filler = -((int)value);
			for (int groupIt = groupSelector(startIndex) + 1; groupIt < groupSelector(endIndex); ++groupIt) {
				bits[groupIt] = filler;
			}
		}
	}
	void fileIn() {
		ifstream fi(fName);
		if (!fi.is_open()) { return; }
		int inFileSize;
		fi >> inFileSize;
		if (inFileSize != size) { return; }
		for (int& group : bits) {
			fi >> group;
		}
		fi.close();
	}
	void fileOut() {
		ofstream fi(fName);
		fi << size << '\n';
		for (const int& group : bits) {
			fi << group << '\n';
		}
		fi.close();
	}
};

struct CacheEntry {
	int row;
	char data[ROW_LENGTH];
	CacheEntry() : row(), data() {}
	CacheEntry(int row) : row(row), data() {}
	CacheEntry(int row, char data[]) : row(row), data() {
		strncpy(this->data, data, ROW_LENGTH - 2);
	}
	void update(int newRow) {
		row = newRow;
	}
	void update(char newData[]) {
		strncpy(data, newData, ROW_LENGTH - 2);
	}
	void update(int newRow, char newData[]) {
		update(newRow);
		update(newData);
	}
	bool operator<(const CacheEntry& rhs) const {
		return row < rhs.row;
	}
	static CacheEntry str2entry(ifstream& fi) {
		CacheEntry entry;
		fi >> entry.row >> entry.data;
		return entry;
	}
	static string entry2str(const CacheEntry& entry) {
		ostringstream oss;
		oss << entry.row << ' ' << entry.data << '\n';
		return oss.str();
	}
};

struct Cache {
	string cacheFileName;
	string validFileName;
	int size;
	set<CacheEntry> entries;
	queue<int> requestQueue;
	vector<int> requestQueueCounter;
	BitVector valid;
	char cacheBuffer[ROW_LENGTH];

	static constexpr int NO_EVICT = -1;

	Cache(int size, int memorySize, string cacheFileName, string validFileName) :
		cacheFileName(cacheFileName),
		validFileName(validFileName),
		size(size),
		entries(),
		requestQueue(),
		requestQueueCounter(memorySize),
		valid(memorySize, validFileName)
	{
		strcpy(cacheBuffer, "0x00000000");
		fileIn();
	}
	~Cache() {
		fileOut();
	}
	bool get(int row, char buffer[]) {
		if (valid.get(row)) {
			strncpy(buffer, (entries.find(CacheEntry(row)))->data, ROW_LENGTH - 2);
			return true;
		}
		return false;
	}
	int set(int row, char buffer[]) {
		if (valid.get(row)) {
			// pop
			entries.erase(row);
			// push
			requestQueue.push(row);
			++requestQueueCounter[row];
			entries.insert({ row, buffer });
			return NO_EVICT;
		}
		// pop
		int poped = NO_EVICT;
		if (size <= entries.size()) {
			while (poped == NO_EVICT && !requestQueue.empty()) {
				poped = evict();
			}
		}
		// push
		valid.set(row, true);
		requestQueue.push(row);
		++requestQueueCounter[row];
		entries.insert({ row, buffer });
		if (poped != NO_EVICT) {
			strncpy(buffer, cacheBuffer, ROW_LENGTH - 2);
			return poped;
		}
		return NO_EVICT;
	}
	int evict() {
		bool isValidPop = false;
		int poped = requestQueue.front();
		requestQueue.pop();
		isValidPop = --requestQueueCounter[poped] == 0;
		if (isValidPop) {
			std::set<CacheEntry>::iterator entryIt = entries.find(poped);
			int evictedRow = entryIt->row;
			strncpy(cacheBuffer, entryIt->data, ROW_LENGTH - 2);
			valid.set(poped, false);
			entries.erase(poped);
			return evictedRow;
		}
		return NO_EVICT;
	}
	void fileIn() {
		ifstream fi(cacheFileName);
		if (!fi.is_open()) { return; }
		int inFileSize;
		fi >> inFileSize;
		if (inFileSize != size) { return; }
		int entrySize;
		fi >> entrySize;
		for (int entryIt = 0; entryIt < entrySize; ++entryIt) {
			entries.insert(CacheEntry::str2entry(fi));
		}
		int queueSize;
		fi >> queueSize;
		for (int queueIt = 0; queueIt < queueSize; ++queueIt) {
			int row;
			fi >> row;
			requestQueue.push(row);
		}
		int vectorSize;
		fi >> vectorSize;
		for (int& cnt : requestQueueCounter) {
			fi >> cnt;
		}
	}
	void fileOut() {
		ofstream fi(cacheFileName);
		fi << size << '\n';
		fi << entries.size() << '\n';
		for (const CacheEntry& entry : entries) {
			fi << CacheEntry::entry2str(entry);
		}
		fi << requestQueue.size() << '\n';
		while (!requestQueue.empty()) {
			fi << requestQueue.front() << '\n';
			requestQueue.pop();
		}
		fi << requestQueueCounter.size() << '\n';
		for (int& cnt : requestQueueCounter) {
			fi << cnt << '\n';
		}
	}
};

//-- memeory mainsystem --//

struct Memory;

struct Memory {
	string fNandName;
	string fResultName;
	fstream fNand;
	ofstream fResult;
	Cache cache;
	BitVector valid;
	int size;
	char readBuffer[ROW_LENGTH];
	char writeBuffer[ROW_LENGTH];

	static constexpr char DEFAULT_DATA[11] = "0x00000000";

	// construct and validate
	Memory(int size, int cacheSize, string& dir) :
		fNandName(dir + "nand.txt"),
		fResultName(dir + "result.txt"),
		fNand(),
		fResult(),
		cache(cacheSize, size, dir + "cache.txt", dir + "cacheValid.txt"),
		valid(size, dir + "nandValid.txt"),
		size(size) {
		setReadBuffer(DEFAULT_DATA);
		readBuffer[ROW_LENGTH - 2] = '\0';
		setWriteBuffer(DEFAULT_DATA);
		writeBuffer[ROW_LENGTH - 2] = '\0';
		if (!validate()) {
			reconstruct();
		}
	}
	bool validate() {
		fNand.open(fNandName);
		fNand.seekg(0, ios::beg);
		for (int colIt = 0; colIt < size; ++colIt) {
			string str;
			fNand >> str;
			if (str.empty()) {
				fNand.close();
				return false;
			}
		}
		fNand.close();
		return true;
	}
	void reconstruct() {
		setWriteBuffer(DEFAULT_DATA);
		for (int it = 0; it < size; ++it) {
			physicalWrite(it);
		}
	}
	constexpr bool isValidRow(int row) { return 0 <= row && row < size; }
	constexpr bool isValidRow(int rowStart, int rowEnd) { return 0 <= rowStart && rowStart <= rowEnd && rowEnd < size; }
	// public
	void write(int row) {
		int cacheEvictedIndex = cache.set(row, writeBuffer);
		valid.set(row, true);
		if (cacheEvictedIndex != Cache::NO_EVICT) {
			if (valid.get(cacheEvictedIndex)) { return; }
			physicalWrite(row);
		}
	}
	void read(int row) {
		if (!valid.get(row)) {
			setReadBuffer(DEFAULT_DATA);
			return;
		}
		if (cache.get(row, readBuffer)) { return; }
		physicalRead(row);
	}
	void erase(int row) {
		valid.set(row, false);
	}
	void erase(int rowStart, int rowEnd) {
		valid.set(rowStart, rowEnd, false);
	}
	void cacheFlush() {
		while (!cache.requestQueue.empty()) {
			int row = cache.evict();
			if (row == Cache::NO_EVICT) { continue; }
			setWriteBuffer(cache.cacheBuffer);
			physicalWrite(row);
		}
	}
	// buffer set
	void setReadBuffer(const char* data) {
		strncpy(readBuffer, data, ROW_LENGTH - 2);
	}
	void setWriteBuffer(const char* data) {
		strncpy(writeBuffer, data, ROW_LENGTH - 2);
	}
	// file io
	void physicalWrite(int row) {
		fNand.open(fNandName);
		fNand.seekp(row * ROW_LENGTH, ios::beg);
		fNand << writeBuffer << '\n';
		fNand.close();
	}
	void physicalRead(int row) {
		fNand.open(fNandName);
		fNand.seekg(row * ROW_LENGTH, ios::beg);
		fNand >> readBuffer;
		fNand.close();
	}
	void physicalErase(int row) {
		setWriteBuffer(DEFAULT_DATA);
		physicalWrite(row);
	}
	void printResult(int row) {
		fResult.open(fResultName);
		fResult << "R " << row << ' ' << readBuffer << '\n';
		fResult.close();
	}
};

//-- command buffer --//
struct BufferEntry {
	bool valid;
	vector<string> args;
	BufferEntry() : valid(false), args() {}
	BufferEntry(int argc, char* argv[]) : valid(true), args() {
		for (int argIt = 1; argIt < argc; ++argIt) {
			args.emplace_back(argv[argIt]);
		}
	}
	static BufferEntry str2entry(ifstream& fi) {
		int argc;
		fi >> argc;
		BufferEntry entry;
		cin >> entry.valid;
		string arg;
		for (int argIt = 1; argIt < argc; ++argIt) {
			fi >> arg;
			entry.args.emplace_back(arg);
		}
		return entry;
	}
	static string entry2str(const BufferEntry& entry) {
		ostringstream oss;
		oss << (entry.args.size() + 1) << ' ' << entry.valid;
		for (const string& arg : entry.args) {
			oss << ' ' << arg;
		}
		oss << '\n';
		return oss.str();
	}
};

struct CommandBuffer {
	vector<BufferEntry> buffer;
	string fName;
	int size;
	int memorySize;
	int capacity;
	CommandBuffer(int capacity, string dir) : buffer(), fName(dir + "commandBuffer.txt"), size(0), memorySize(memorySize), capacity(capacity) {
		fileIn();
	}
	~CommandBuffer() {
		fileOut();
	}
	void add(int argc, char* argv[]) {
		buffer.emplace_back(argc, argv);
	}
	bool validate(int row) {
		vector<bool> affected = vector<bool>(memorySize, false);
		vector<bool> valid = vector<bool>(memorySize, true);
		int eraseStart = -1;
		int eraseEnd = -1;
		for (int entryIt = buffer.size() - 1; 0 <= entryIt; --entryIt) {
			int argc = buffer[entryIt].args.size() + 1;
			vector<string>& argv = buffer[entryIt].args;
			if (argc == 4 && !strcmp(argv[1].c_str(), "W")) {
				eraseStart = eraseEnd = -1;
				int rowToWrite = atoi(argv[2].c_str());
				if (affected[rowToWrite]) {
					buffer[entryIt].valid = false;
					continue;
				}
				affected[rowToWrite] = true;
			}
			else if (argc == 3 && !strcmp(argv[1].c_str(), "E")) {
				int rowToErase = atoi(argv[2].c_str());
				affected[rowToErase] = true;
				if (eraseStart <= rowToErase + 1 && rowToErase <= eraseEnd) {
					mergeErase();
				}
				eraseStart = rowToErase;
				eraseEnd = rowToErase + 1;
			}
			else if (argc == 4 && !strcmp(argv[1].c_str(), "E")) {
				int rowToEraseStart = atoi(argv[2].c_str());
				int rowToEraseEnd = rowToEraseStart + atoi(argv[3].c_str()) - 1;
			}
		}
	}
	void mergeErase() {

	}
	void fileIn() {
		ifstream fi(fName);
		if (!fi.is_open()) { return; }
		fi >> size;
		for (int commandIt = 0; commandIt < size; ++commandIt) {
			buffer.push_back(BufferEntry::str2entry(fi));
		}
		fi.close();
	}
	void fileOut() {
		ofstream fi(fName);
		fi << size;
		for (const BufferEntry& entry : buffer) {
			fi << BufferEntry::entry2str(entry);
		}
	}
};

//-- main --//

int main(int argc, char* argv[]) {
	DirectoryManager dm(argv[0]);
	Memory ssd(DEFAULT_SIZE, DEFAULT_CACHE_SIZE, dm.cwd);
	if (argc < 2) { return 0; }
	else if (argc == 3 && !strcmp(argv[1], "R")) {
		int rowToRead = atoi(argv[2]);
		ssd.read(rowToRead);
		ssd.printResult(rowToRead);
	}
	else if (argc == 4 && !strcmp(argv[1], "W")) {
		int rowToWrite = atoi(argv[2]);
		ssd.setWriteBuffer(argv[3]);
		ssd.write(rowToWrite);
	}
	else if (argc == 3 && !strcmp(argv[1], "E")) {
		int rowToErase = atoi(argv[2]);
		ssd.erase(rowToErase);
	}
	else if (argc == 4 && !strcmp(argv[1], "E")) {
		int rowToEraseStart = atoi(argv[2]);
		int rowToEraseEnd = rowToEraseStart + atoi(argv[3]) - 1;
		ssd.erase(rowToEraseStart, rowToEraseEnd);
	}
	else if (argc == 2 && !strcmp(argv[1], "CF")) {
		ssd.cacheFlush();
	}
	return 0;
}
