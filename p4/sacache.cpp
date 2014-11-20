#include <iostream>
#include <fstream>
#include <string>
#include <cstring>
#include <cstdlib>
#include <iomanip>

using namespace std;

class CacheBlock
{
  public:
    CacheBlock () : data(), tag(0), valid(false), dirty(false) {}

    unsigned char readData(int pos) { return data[pos]; }
    void writeData(unsigned char value, int pos) { data[pos] = value; }
    unsigned short getTag() { return tag; }
    void setTag(unsigned short newTag) { tag = newTag; }
    bool isValid() { return (valid ? true : false); }
    void setValid(bool value) { valid = value; }
    bool isDirty() { return (dirty ? true : false); }
    void setDirty(bool value) { dirty = value; }

    void memToCache(unsigned char memory[], unsigned short address)
    {
      unsigned short startAddress = address - (address % 4);
      for (int i = 0; i < 4; i++)
        data[i] = memory[startAddress + i];
    }
    void cacheToMem(unsigned char memory[])
    {
      unsigned short startAddress = tag << 2;
      for (int i = 0; i < 4; i++)
        memory[startAddress + i] = data[i];
    }

  private:
    unsigned char data[4];
    unsigned short tag;
    bool valid;
    bool dirty;
};

#define NOT_FOUND 6

unsigned char foundCacheBlock(CacheBlock cacheLine[], unsigned short tag)
{
  for (unsigned char blockNum = 0; blockNum < 6; blockNum++)
    if (cacheLine[blockNum].getTag() == tag && cacheLine[blockNum].isValid())
      return blockNum;

  return NOT_FOUND;
}

void updateLRU(unsigned char lruList[], unsigned char blockNum)
{
  for (int i = 0; i < 6; i++)
    if (i != blockNum)
      lruList[i]++;
  lruList[blockNum] = 0;
}

unsigned char findLRU(unsigned char lruList[])
{
  unsigned char lru = 0;
  for (int i = 1; i < 6; i++)
    if (lruList[lru] < lruList[i])
      lru = i;

  return lru;
}

int main(int argc, char *argv[])
{
  ifstream infile(argv[1]);
  ofstream outfile("sa-out.txt");
  string line;
  unsigned char memory[0x10000] = {0};
  unsigned char lruList[10][6] = {0};
  CacheBlock cache[10][6];
  unsigned char value, byteOffset, setNum, blockNum;
  unsigned short address, tag;

  if (!infile.is_open())
  {
    cout << "Failed to open input file.\n";
    if (outfile.is_open())
      outfile.close();
    return 1;
  }
  else if (!outfile.is_open())
  {
    cout << "Failed to open output file.\n";
    infile.close();
    return 2;
  }

  while(getline(infile, line))
  {
    address = strtoul(line.substr(0,4).c_str(), NULL, 16);
    value = strtoul(line.substr(8,10).c_str(), NULL, 16);
    tag = address >> 2;
    byteOffset = address % 4;
    setNum = tag % 10;
    blockNum = foundCacheBlock(cache[setNum], tag);
   
    if (line[5] == 'F') // write operation
    {
      if (blockNum != NOT_FOUND) // cache hit
      {
        cache[setNum][blockNum].writeData(value, byteOffset);
        cache[setNum][blockNum].setDirty(true);
        updateLRU(lruList[setNum], blockNum);
      }
      else // cache miss
      {
        blockNum = findLRU(lruList[setNum]);

        if (cache[setNum][blockNum].isDirty())
          cache[setNum][blockNum].cacheToMem(memory);

        cache[setNum][blockNum].memToCache(memory, address);
        cache[setNum][blockNum].writeData(value, byteOffset);
        cache[setNum][blockNum].setTag(tag);
        cache[setNum][blockNum].setValid(true);
        cache[setNum][blockNum].setDirty(true);
        updateLRU(lruList[setNum], blockNum);
      }
    }
    else // read operation
    {
      if (blockNum != NOT_FOUND) // cache hit
      {
        outfile << hex << uppercase << setfill('0') << setw(2) 
          << (int) cache[setNum][blockNum].readData(byteOffset) << " " 
          << 1 << " " << cache[setNum][blockNum].isDirty() << endl;
       
        updateLRU(lruList[setNum], blockNum);
      }
      else // cache miss
      {
        blockNum = findLRU(lruList[setNum]);

        if (cache[setNum][blockNum].isDirty())
          cache[setNum][blockNum].cacheToMem(memory);

        cache[setNum][blockNum].memToCache(memory, address);

        outfile << hex << uppercase << setfill('0') << setw(2)
          << (int) cache[setNum][blockNum].readData(byteOffset) << " "
          << 0 << " " << cache[setNum][blockNum].isDirty() << endl;

        cache[setNum][blockNum].setTag(tag);
        cache[setNum][blockNum].setValid(true);
        cache[setNum][blockNum].setDirty(false);
        updateLRU(lruList[setNum], blockNum);
      }
    }
  }

  infile.close();
  outfile.close();
  return 0;
}
