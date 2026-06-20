#include <iostream>
#include <fstream>
#include <vector>
#include <deque>
#include <string>
#include <algorithm>
using namespace std;

// A structure for Process that contains the 
// necessary info's related to a process.
// Contains fields and a constructor
struct Process {
    int id;
    int burstTime;
    int remainingTime;
    int arrivalTime;
    int waitTime;
    int turnsInA;
    char queueSource; 
    int completionTime;
    int queueBEntryTime;
    
    Process(int i, int burst, int _arrivalTime) {
        id = i;
        completionTime = -1;
        queueSource = 'A'; // New Process will start from queue A
        waitTime = 0;
        burstTime = burst;
        remainingTime = burst; // The burst time will be the remaining time the process has to execute
        arrivalTime = _arrivalTime;
        turnsInA = 0;
        queueBEntryTime = -1;
    }
};

// To handle Spaces in the input string I am using this function
string trim(const string& str) {
    int first = str.find_first_not_of(" \r\n\t");
    if (string::npos == first) return "";
    int last = str.find_last_not_of(" \r\n\t");
    return str.substr(first, (last - first + 1));
}

int main(int argc, char* argv[]) {

    // 1. Handle the inputs
    string filename;
    int demotionCriteria;
    int dispatchRatio;
    
    if (argc == 4) {
        filename = argv[1];
        demotionCriteria = stoi(argv[2]);
        dispatchRatio = stoi(argv[3]);
    } else {
        cout << "Enter the name of the input file:";
        cin >> filename;
        cout << "Enter the value of demotion criteria:";
        cin >> demotionCriteria;
        cout << "Enter the value of dispatch ratio:";
        cin >> dispatchRatio;    
    }
    
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

    // Up until here I have all the inputs set., filename, dispatchRatio, demotionCriteria.
    // Opened the file mentioned in the command line argument -> read line by line -> load all the lines to a vector.

    // Initialize 2 queues & a vector of type Process*
    deque<Process*> QueueA;
    deque<Process*> QueueB;
    vector<Process*> completedProcesses;
    
    Process* cpuProcess = nullptr;
    int currentTime = 0;
    int timeInCpu = 0; // Tracks how long the current process has been running
    int currentQuantum = 0;
    int dispatchCountA = 0;
    
    int processIDCounter = 0;
    int totalExecutionTime = 0;
    int idleTime = 0;
    
    //2. Main Simulation
    // In every loop we evaluate the process., Whether it has finished - which queue should get the cpu next -
    // Based on dispatch ratio and demotion criteria we decide the queue. We demote long CPU bound jobs to the other queue.    
    while(true) {
        Process* returningProcess = nullptr;
        
        //a. Check status of current process in execution 
        if(cpuProcess != nullptr) {
            if(cpuProcess->remainingTime == 0) {
                // Process has finished executing so update the completion time and put it in the vector
                cpuProcess->completionTime = currentTime;
                completedProcesses.push_back(cpuProcess);
                cpuProcess = nullptr;
            } else if(timeInCpu >= currentQuantum) {
                returningProcess = cpuProcess;
                cpuProcess = nullptr;
            }
        }
        
        //b. Dispatch process from other Queue if CPU is not busy
        if(cpuProcess == nullptr) {
            bool pickA = false;
            bool bothHaveJobs = !QueueA.empty() && !QueueB.empty();
            
            // Dispatch Ratio Logic
            if (!QueueA.empty() && QueueB.empty()) {
                // Only queue A has jobs
                pickA = true;
                dispatchCountA = 0;
            } else if (QueueA.empty() && !QueueB.empty()) {
                // Only queue B has jobs
                pickA = false;
                dispatchCountA = 0;
            } else if (bothHaveJobs) {
                if (dispatchRatio == 0) {
                    pickA = true; // because Queue A has absolute priority
                } else if (dispatchCountA < dispatchRatio) {
                    pickA = true;
                } else {
                    pickA = false;
                }
            }
            
            if(pickA && !QueueA.empty()) {
                // Process FCFS queue
                cpuProcess = QueueA.front();
                QueueA.pop_front();
                currentQuantum = 5;
                timeInCpu = 0;
                cpuProcess->queueSource = 'A';
                
                if(bothHaveJobs && dispatchRatio > 0) {
                    dispatchCountA++;
                }
            } else if(!pickA && !QueueB.empty()) {
                // Stable sort B by remaining time because queue B is Shortest Job First
                // Just using a lambda function to use stable sort
                stable_sort(QueueB.begin(), QueueB.end(), 
                    [](Process* a, Process* b) {
                        if(a->remainingTime != b->remainingTime) {
                            return a->remainingTime < b->remainingTime;
                        }
                        return a->queueBEntryTime < b->queueBEntryTime;
                    });
                
                cpuProcess = QueueB.front();
                QueueB.pop_front();
                currentQuantum = 40; // Queue B Quantum value
                timeInCpu = 0;
                cpuProcess->queueSource = 'B';
                
                if(bothHaveJobs && dispatchRatio > 0) {
                    dispatchCountA = 0; // Reset after a dispatch from Queue B
                }
            }
        }
        
        // c. Queue/Fetch Job
        // Returning process has priority over newly fetched processes
        // If a process was kicked off the CPU in previous Step, put it back in a queue
        if(returningProcess != nullptr) {
            if (returningProcess->queueSource == 'A') {
                returningProcess->turnsInA++;
                // Demotion Check
                if (demotionCriteria > 0 && 
                    returningProcess->turnsInA >= demotionCriteria) {
                    returningProcess->queueBEntryTime = currentTime;
                    QueueB.push_back(returningProcess);
                } else {
                    QueueA.push_back(returningProcess);                    
                }
            } else {
                returningProcess->queueBEntryTime = currentTime;
                QueueB.push_back(returningProcess);
            }
        }
        
        // d. Fetch new job if available at current clock tick
        if(currentTime < (int)input_lines.size() && input_lines[currentTime] != "idle") {
            int burst = stoi(input_lines[currentTime]);
            Process* newProcess = new Process(++processIDCounter, burst, currentTime);
            QueueA.push_back(newProcess); // New processes always enter Queue A
        }
        
        
        // e.Termination check
        bool moreInput = false;
        for(int i = currentTime; i < (int)input_lines.size(); ++i) {
            if(input_lines[i] != "idle") { 
                moreInput = true;
                break;
            }
        }


        // We can break out of the main loop if.,
        // No more process are going to arrive &
        // Queue A is empty &
        // Queue B is empty &
        // CPU is currently idle
        if(!moreInput && QueueA.empty() && QueueB.empty() && cpuProcess == nullptr) {
            break;
        }
        
        // f. Update times related to counters
        if(cpuProcess != nullptr) {
            // here CPU is busy
            cpuProcess->remainingTime--;
            timeInCpu++;
            totalExecutionTime++;
        } else {
            // here CPU is idel
            if(currentTime > 0) {
                idleTime++;
            }
        }
        
        // Wait times increment if sitting in a queue
        for(Process* p : QueueA) p->waitTime++;
        for(Process* p : QueueB) p->waitTime++;
        
        currentTime++;
    }
    
    //Calcuate end Time, longest wait time and total wait time
    int endTime = totalExecutionTime + idleTime;
    int longestWait = 0;
    int totalWait = 0;
    
    for(Process* p : completedProcesses) {
        totalWait += p->waitTime;
        if(p->waitTime > longestWait) {
            longestWait = p->waitTime;
        }
    }
    
    // Calculate AVG wait time
    int completedCount = completedProcesses.size();
    int avgWait = completedCount > 0 ? (totalWait/completedCount) : 0;
    
    cout << left << setw(25) << "End Time:" << endTime << "\n";
    cout << left << setw(25) << "Processes Completed:" << completedCount << "\n";
    cout << left << setw(25) << "Total execution time:" << totalExecutionTime << "\n";
    cout << left << setw(25) << "Idle time:" << idleTime << "\n";
    cout << left << setw(25) << "Longest Wait Time:" << longestWait << "\n";
    cout << left << setw(25) << "Average Wait Time:" << avgWait << "\n";
    cout << left << setw(25) << "Total Wait Time:" << totalWait << "\n";
    
    // Clean up memory
    for(Process* p : completedProcesses) delete p;
    for(Process* p : QueueA) delete p;
    for(Process* p : QueueB) delete p;
    
    return 0;
}