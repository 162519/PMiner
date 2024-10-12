#include <iostream>
#include <fstream>
#include <string>
#include <thread>
#include <mutex>
using namespace std;


class SynchronizedFile {
public:
	SynchronizedFile(const string& path) : _path(path) {
		
		outfile.open(_path);
	}

	void write(const string& dataToWrite) {
		
		std::lock_guard<std::mutex> lock(_writerMutex);

	
		outfile << dataToWrite;
		outfile << endl;
	}

private:
	string _path;
	ofstream outfile;
	std::mutex _writerMutex;
};


class Writer {
public:
    
	Writer(std::shared_ptr<SynchronizedFile> sf) : _sf(sf) {}

	void DataProcessing(std::vector<std::vector<unsigned>>& PMR_copy) {
		
		string record_line = "";
        for (unsigned i = 0; i < PMR_copy.size(); ++i) {
            record_line += 'P' + to_string(i) + ": ";
            for (auto j : PMR_copy[i]) {
                record_line += to_string(j) + " ";
            }
        }
		_sf->write(record_line);
	}
	void DataProcessing(std::vector<unsigned>& PMR_copy){
		string record_line = "";
		for (unsigned i = 0; i < PMR_copy.size(); ++i){
			record_line += 'P' + to_string(i) + ": ";
			record_line += to_string(PMR_copy[i]) + " ";
		}
		_sf->write(record_line);
	}
private:
	std::shared_ptr<SynchronizedFile> _sf;
};
