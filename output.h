#include <iostream>
#include <fstream>
#include <string>
#include <thread>
#include <mutex>
#include <vector>
using namespace std;

//ͬ���ļ������ֻ࣬��һ�����������������
class SynchronizedFile {
public:
	SynchronizedFile(const string& path) : _path(path) {
		//���ļ�
		outfile.open(_path);
	}

	void write(const string& dataToWrite) {
		//ʹ��lock_guard����ȷ��ͬһʱ��ֻ��һ���߳���д�룬��д��������Զ�����
		std::lock_guard<std::mutex> lock(_writerMutex);

		//��ʼд���ļ�
		outfile << dataToWrite;
		outfile << endl;
	}

private:
	string _path;
	ofstream outfile;
	std::mutex _writerMutex;
};

//����࣬ÿ���߳��а���һ�����������ݴ���͵��ù����෽������д��
class Writer {
public:
	//����ʹ������ָ��������ʹ�����������ָ�������
	Writer(std::shared_ptr<SynchronizedFile> sf) : _sf(sf) {}

	void DataProcessing(std::vector<std::vector<unsigned>>& PMR_copy) {
		//����������ݴ��������˴�����Ϊÿ������������
		string record_line = "";
		for (unsigned i = 0; i < PMR_copy.size(); ++i) {
			record_line += 'P' + to_string(i) + ": ";
			for (auto j : PMR_copy[i]) {
				record_line += to_string(j) + " ";
			}
		}
		_sf->write(record_line);
	}
private:
	std::shared_ptr<SynchronizedFile> _sf;
};
