//Authors: Courtney Laux, Angela(Haowen) Zhou


#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <cstdlib>
using namespace std; 

const int lineSize = 8; 
const int numofLine = 256/lineSize; 
const int memorySize = 65536/lineSize;

class Line{
	private:
		int byte[lineSize]; 
	public:
		Line()
		{
			for(int i = 0; i < lineSize; i++){
				byte[i] = 0;
			 }
		}
		void writeByte(const int offset, int data)
		{
			byte[offset] = data; 
		}

		int readByte(const int offset)
		{
			return this->byte[offset]; 
		}

}; 

class Cache{
	private:
		Line line[numofLine]; 
		int tag[numofLine]; 
		int dirty[numofLine]; 
	public:
		Cache()
		{
			for(int i = 0; i < numofLine; i++)
			{
				tag[i] = 0; 
				dirty[i] = 0; 
			}
		}

		void writeLine(const int set, const int offset, int data)
		{
			line[set].writeByte(offset, data); 
		}
		int returnLine(const int set, const int offset)
		{
			return line[set].readByte(offset); 
		}

		void setTag(const int set, int value)
		{
			tag[set] = value; 
		}
		int getTag(const int set)
		{
			return tag[set]; 
		}
		void setDirty(const int set, int value)
		{
			dirty[set] = value; 
		}
		int getDirty(const int set)
		{
			return dirty[set]; 
		}
}; 

class Memory{
	private:
		Line blocks[memorySize]; 
	public:
		void writeLine(const int set, const int offset, int data)
		{
			blocks[set].writeByte(offset, data); 
		}
		int returnLine(const int set, const int offset)
		{
			return blocks[set].readByte(offset); 
		}
}; 

class CPU{
	private: 
		Cache dmc; 
		Memory ram; 
	public:
		void write(int address, int data)
		{
			int addressMask = 0xff00; 
			int setOffsetMask = 0xff; 
			int offsetMask = 0x07; 
			int cacheAddress = address & addressMask; 
			cacheAddress = cacheAddress >> 8; 
			int setOffset = address & setOffsetMask; 
			int set = setOffset >> 3; 
			int offset = setOffset & offsetMask; 
			int memoryAddress = address/8; 
			
			if(dmc.getTag(set) != cacheAddress)
			{
				 
				if(dmc.getDirty(set) == 0)
				{
					 
					for(int i = 0; i < lineSize;i++)
					{
						dmc.writeLine(set, i, ram.returnLine(memoryAddress, i)); 

					}
					dmc.writeLine(set, offset, data); 
					dmc.setTag(set, cacheAddress); 
					dmc.setDirty(set, 1); 
				}
				else
				{
					
					int tmp = (dmc.getTag(set)<<8) | (set<<3) | offset;
					tmp = tmp/8; 
					
					for(int i = 0; i < lineSize;i++)
					{
						ram.writeLine(tmp, i, dmc.returnLine(set, i)); 

					}
					for(int i = 0; i < lineSize;i++)
					{
						dmc.writeLine(set, i, ram.returnLine(memoryAddress, i)); 

					}
					dmc.writeLine(set, offset, data); 
					dmc.setTag(set, cacheAddress); 
					dmc.setDirty(set, 1); 

				}
			}
			else
			{
				dmc.writeLine(set, offset, data); 
				dmc.setDirty(set, 1); 
			}
			
		}
		void read(int address, int data)
		{
			int addressMask = 0xff00; 
			int setOffsetMask = 0xff; 
			int offsetMask = 0x07; 
			int cacheAddress = address & addressMask; 
			cacheAddress = cacheAddress >> 8; 
			int setOffset = address & setOffsetMask; 
			int set = setOffset >> 3; 
			int offset = setOffset & offsetMask; 
			int memoryAddress = address/8; 

			ofstream outputFile; 
			outputFile.open("dm-out.txt", std::ios_base::app);	
					
			if(dmc.getTag(set) == cacheAddress)
			{
				//Read hit
				if(dmc.returnLine(set, offset) == data)
				{
					outputFile<<hex<<data<<" " << 1 << " " << dmc.getDirty(set)<<endl;
					 
				} 
				else
				{ 
					outputFile<<hex<<data<<" " << 0 << " " << dmc.getDirty(set)<<endl;
				}
				//chec later if need to set dirty
			}
			else 
			{

				//Read miss
				if(dmc.getDirty(set) == 0)
				{
					
					outputFile<<hex<<data<<" " << 0 << " " << dmc.getDirty(set)<<endl; 
					for(int i = 0; i < lineSize;i++)
					{
						dmc.writeLine(set, i, ram.returnLine(memoryAddress, i)); 

					}
					dmc.setTag(set, cacheAddress); 
					dmc.setDirty(set, 0); 
				}
				else
				{
					
					outputFile<<hex<<data<<" " << 0 << " " << dmc.getDirty(set)<<endl; 
					int tmp = (dmc.getTag(set)<<8) | (set<<3) | offset;
					tmp = tmp/8; 
										
					for(int i = 0; i < lineSize;i++)
					{
						ram.writeLine(tmp, i, dmc.returnLine(set, i)); 

					}

					
					 
					for(int i = 0; i < lineSize;i++)
					{
						dmc.writeLine(set, i, ram.returnLine(memoryAddress, i)); 

					}
					dmc.setTag(set, cacheAddress); 
					dmc.setDirty(set, 0); 



				}
			}

						
			outputFile.close();
		}

}; 

int main(int argc, char* argv[])
{
	
	CPU cpu; 	
	long int address;
   	long int option;
  	 long int data;

	 string line;


	ifstream myfile(argv[1]);
   	if(myfile.is_open())
   	{
   	    while(getline(myfile,line))
	    {
	  address = strtol(line.substr(0,4).c_str(),NULL,16);
	  option = strtol(line.substr(5,7).c_str(),NULL,16);
	  data = strtol(line.substr(8,9).c_str(),NULL,16);
	   
		if(option == 0)
		{
			cpu.read(address, data); 
		}
		else
		{
			cpu.write(address, data); 
		}
		
	     }

	}
	
	myfile.close(); 
	return 0;
}
