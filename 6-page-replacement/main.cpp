#include <iostream>
#include <fstream>
#include <vector>
#include <deque>
#include <string>
#include <algorithm>
#include <iomanip>

using namespace std;

// This is for inverted page table
// A single row consists of {page_number, ref_bit, timeStamp, dirty_bit}
struct Frame {
	int page_number; // -1 for empty
	int ref_bit;     // For second chance and enhanced second chance
	int timeStamp;   // For LRU
	int dirty_bit;   // For enhanced second chance
};

// Convert the hexadecimal string part of size 8 (index : 0 - 7) to long integer
unsigned int addressParser(string line, int pageSize) {
	string hexAddr = line.substr(0, 8); // fixed input from file
	unsigned int address = stoul(hexAddr, nullptr, 16);

	int pageSizeInBytes = pageSize * 1024;
	unsigned int pageNumber = address / pageSizeInBytes;

	return pageNumber;
}

// Extract the operation(read/write) from the line, assuming both operations are in capital letters
int operationParser(string line) {
	// If line = "190a7c20 R" then index 9 indicates whether its a read or write operation
	char op = line[9];
	return (op == 'W') ? 1 : 0; // If its a write operation then dirty bit = 1 else 0
}

void secondChanceAlgo(vector<string> &input_lines, int memorySize, int pageSize, int &pageFaults) {
	int numberOfFrames = memorySize / pageSize;
	vector<Frame> table(numberOfFrames, {-1, 0, 0, 0});
	unsigned int pageNum = 0;

	int next = 0;
	for(int i = 0; i < input_lines.size(); i++) {
		pageNum = addressParser(input_lines[i], pageSize);		

		//search for the frame
		int foundFrame = -1;
		for(int j = 0; j < numberOfFrames; j++) {
			if(table[j].page_number == pageNum) {
				foundFrame = j;
				break;
			}
		}

		// if found., set ref bit = 1 & if not found then find the free frame and load it
		if(foundFrame != -1) {
			table[foundFrame].ref_bit = 1;
		} else {
			pageFaults++;
			
			int freeFrameIndex = -1;

			// find the free frame
			for(int j = 0; j < numberOfFrames; j++) {
				if(table[j].page_number == -1) {
					freeFrameIndex = j;
					break;
				}
			}

			// if found., set ref bit = 1 and update the page_number which is equivalent to replacement
			if(freeFrameIndex != -1) {
				table[freeFrameIndex].page_number = pageNum;
				table[freeFrameIndex].ref_bit = 1;
			} else {
				// if there is no free frame then keep looping and changing ref bit on every cycle
				while(true) {
					if(table[next].ref_bit == 0) {
						table[next].page_number = pageNum;
						table[next].ref_bit = 1;
						next = (next + 1) % numberOfFrames;
						break;
					}

					table[next].ref_bit = 0;
					next = (next + 1) % numberOfFrames;
				}
			}
		}			
	}
}

void lruAlgo(vector<string> &input_lines, int memorySize, int pageSize, int &pageFaults) {
	int timer = 0;
	int numberOfFrames = memorySize / pageSize;
	unsigned int pageNum = 0;

	vector<Frame> table(numberOfFrames, {-1, 0, 0, 0});

	for(int i = 0; i < input_lines.size(); i++) {
		timer++;
		pageNum = addressParser(input_lines[i], pageSize);

		int foundFrame = -1;
		
		for(int j = 0; j < numberOfFrames; j++) {			
			if(table[j].page_number == pageNum) {
				foundFrame = j;
				break;
			}
		}

		if(foundFrame != -1) {
			// Page exists in the frame so update the timer bcoz we are accessing it now	
			table[foundFrame].timeStamp = timer;
		} else {
			// If the page doesn't exist in the frame then increase the page fault and look for free frame
			pageFaults++;
						
			int freeFrameIndex = -1;
			for(int j = 0; j < numberOfFrames; j++) {			
				if(table[j].page_number == -1) {
					freeFrameIndex = j;
					break;
				}
			}
			
			// Look for a free frame, if it exists then update page number and time stamp
			if(freeFrameIndex != -1) {
				table[freeFrameIndex].page_number = pageNum;
				table[freeFrameIndex].timeStamp = timer;
			} else {				
				//No free frame found so use LRU to evict the least recently used
				int smallestTimeStamp = timer;
				int lruIndex = -1;

				//Do a table traversal and find out which one has the smallest timestamp value
				//Then that becomes the least recently used frame & we can update it
				for(int j = 0; j < numberOfFrames; j++) {
					if(table[j].timeStamp <= smallestTimeStamp) {
						lruIndex = j;
						smallestTimeStamp = table[j].timeStamp;
					}
				}

				table[lruIndex].page_number = pageNum;
				table[lruIndex].timeStamp = timer;
			}			
		}
	}
}

void enhancedSecondChanceAlgo(vector<string> &input_lines, int memorySize, int pageSize, int &pageFaults) {
	int numberOfFrames = memorySize / pageSize;
	unsigned int pageNum = 0;
	int dirtyBit = 0;
	
	vector<Frame> table(numberOfFrames, {-1, 0, 0, 0});
	int next = 0;

	for(int i = 0; i < input_lines.size(); i++) {
		pageNum = addressParser(input_lines[i], pageSize);
		dirtyBit = operationParser(input_lines[i]);

		// search for the frame
		int foundFrame = -1;
		for(int j = 0; j < numberOfFrames; j++) {
			if(table[j].page_number == pageNum) {
				foundFrame = j;
				break;
			}
		}

		//  if found set ref bit = 1, dirty_bit = 1 & if not found then find the free frame and load it
		if(foundFrame != -1) {
			table[foundFrame].ref_bit = 1;
			if(dirtyBit == 1) table[foundFrame].dirty_bit = 1;
		} else {
			pageFaults++;
			
			// find the free frame
			int freeFrameIndex = -1;
			for(int j = 0; j < numberOfFrames; j++) {
				if(table[j].page_number == -1) {
					freeFrameIndex = j;
					break;
				}
			}

			// if found., set ref bit = 1, dirty bit = 1 and update the page_number which is equivalent to replacement
			if(freeFrameIndex != -1) {
				table[freeFrameIndex].page_number = pageNum;
				table[freeFrameIndex].ref_bit = 1;
				table[freeFrameIndex].dirty_bit = dirtyBit;
			} else {
				//if not found., then start from next pointer pointing to a frame
				//Complete first pass
				//Advance to second pass if first pass didn't yield any results
				bool found = false;
				while(!found) {
					// 1st pass - (0, 0) => (ref_bit, dirty_bit)
					// If a victim is found then simply replace
					for(int j = 0; j < numberOfFrames && !found; j++) {
						if(table[next].ref_bit == 0 && table[next].dirty_bit == 0) {
							table[next].page_number = pageNum;
							table[next].ref_bit = 1;
							table[next].dirty_bit = dirtyBit;
							found = true;
						}
						next = (next + 1) % numberOfFrames;
					}

					// 2nd pass - (0, 1) => (ref_bit, dirty_bit)
					// If a victim is not found in 1st pass, advance to 2nd pass
					// In 2nd pass, we account for an expensive write_to_disk operation before replacing that happens in background
					for(int j = 0; j < numberOfFrames && !found; j++) {
						if(table[next].ref_bit == 0 && table[next].dirty_bit == 1) {
							table[next].page_number = pageNum;
							table[next].ref_bit = 1;
							table[next].dirty_bit = dirtyBit;
							found = true;
						} else {
							table[next].ref_bit = 0;							
						}
						next = (next + 1) % numberOfFrames;
					}
				}
			}
		}
	}
}

// To handle Spaces in the input string I am using this function
string trim(const string& str) {
    int first = str.find_first_not_of(" \r\n\t");
    if (string::npos == first) return "";
    int last = str.find_last_not_of(" \r\n\t");
    return str.substr(first, (last - first + 1));
}

int main(int argc, char* argv[]) {

	// Handle the inputs
	string filename = "";
	char algorithm = '\0';	
	int memorySize = 0;
	int pageSize = 0;

	if (argc == 5) {
        filename = argv[1];
        algorithm = argv[2][0];
        memorySize = stoi(argv[3]);
        pageSize = stoi(argv[4]);
    } else {
    	cout << "Enter the file name:";
    	cin >> filename;
    	cout << "Enter the algorithm:";
    	cin >> algorithm;
    	cout << "Enter the memory size:";
    	cin >> memorySize;
    	cout << "Enter the page size:";
    	cin >> pageSize;
    }

    // Handle file input
    vector<string> input_lines;
    ifstream infile(filename);
    if (!infile.is_open()) {
        cerr << "Error: Could not open file " << filename << endl;
        return 1;
    }
    
    // Read the file line by line using getline() function -> trim it -> put it inside a container
    string line;
    while(getline(infile, line)) {
        string trimmed = trim(line);
        if(!trimmed.empty()) {
            input_lines.push_back(trimmed);
        }
    }
    infile.close();

    // Call the relevant algorithm
    int pageFaults = 0;
    if(algorithm == 'S' || algorithm == 's') {
    	secondChanceAlgo(input_lines, memorySize, pageSize, pageFaults);
    } else if(algorithm == 'L' || algorithm == 'l') {
    	lruAlgo(input_lines, memorySize, pageSize, pageFaults);
    } else if(algorithm == 'E' || algorithm == 'e') {
    	enhancedSecondChanceAlgo(input_lines, memorySize, pageSize, pageFaults);
    } else {
    	cout << "Please enter a relevant page replacement algorithm";
    }

    // Display results    
    cout << left << "Memory size: " << memorySize << ", " << "Page size: " 
    	 << pageSize << ", Page Faults: " << pageFaults << endl;

	return 0;
}