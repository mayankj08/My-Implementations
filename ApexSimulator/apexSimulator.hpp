#ifndef __MY_APEX_SIMULATOR_HPP__
#define __MY_APEX_SIMULATOR_HPP__

#include "instruction.hpp"
#include <fstream> // for file handling operations
#include <errno.h> // for strerrno
#include <string.h> // String operations
#include <vector> //vector operations
#include <map>
#include <sstream>
#include <algorithm>

using std::fstream;
using std::cout;
using std::endl;
using std::vector;
using std::map;
using std::stringstream;

namespace cs520
{

#define BASE_INST_ADDR 4000
#define ARCH_REGISTERS 16
#define APPEND(a,b) a##b
#define FETCH 0
#define DRF 1
#define ALU1 2
#define ALU2 3
#define BRANCH 4
#define DELAY 5
#define MEM 6
#define WB 7

class apexSimulator
{
	private:
		string instructionFile;
		fstream instFilePointer; 
		int currLine=-4;
		map<string,string> registers;
		string regX;
		bool zeroFlag=false;
		int programCounter=BASE_INST_ADDR;
		int time=0;
		vector<vector<cs520::instruction>> pipeline;
		long memoryLoc[1000];
		int completed=0; //Time when the processing completed
		map<int,cs520::instruction> instMap;
		bool lastCycleBranchStatus= false;

	private:
		instruction getNextInstruction();
		bool simulate(int);
		void split(const string &s, const char* delim, vector<string> & v);
		bool processInstruction(map<int,cs520::instruction>,bool&,bool&);
		bool fetchStage(map<int,cs520::instruction>,bool,vector<cs520::instruction>,vector<cs520::instruction>&);
		bool drfStage(bool,vector<cs520::instruction>,vector<cs520::instruction>&,bool);
		bool alu1Stage(bool,vector<cs520::instruction>,vector<cs520::instruction>&,bool);
		bool alu2Stage(vector<cs520::instruction>,vector<cs520::instruction>&);
		vector<cs520::instruction> getPipelineRowInstance();
		bool checkTrueDependency(cs520::instruction,cs520::instruction,vector<string>&);
		bool checkControlDependency(cs520::instruction,cs520::instruction);
		bool wbStage(vector<cs520::instruction>,vector<cs520::instruction>&);
		bool branchStage(vector<cs520::instruction>,vector<cs520::instruction>&,bool&,int&,bool,bool);
		bool delayStage(vector<cs520::instruction>,vector<cs520::instruction>&);
		bool memStage(vector<cs520::instruction>,vector<cs520::instruction>&);
		string computeResult(cs520::instruction,vector<cs520::instruction>);
		bool getForwadedValue(const cs520::instruction&,const cs520::instruction&,int&);
		int getRegisterVal(string);
		string getRegisterValue(string);
	public:
		apexSimulator(std::string);
		int getNumberOfCyclesRan();
		bool run(int count);
		void display();
		bool initialize();
		int getCompleted();
		void show(int);
};

}

#endif
