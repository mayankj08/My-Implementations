#ifndef __INSTRUCTION_HPP__
#define __INSTRUCTION_HPP__

#include<iostream>
#include<string>
#include<vector>

using std::cout;
using std::end;
using std::string;
using std::vector;
using std::pair;

namespace cs520
{
class instruction
{
	public:
		int instAddr; //Stores address of current instruction
		string instName = "";  // instruction
		string op1; 
		string op2; 
		string op3; 
		string result;		
		vector<pair<int,bool>> decodedValue;
		bool zeroFlag=false;
		string regX;
		string fullInst;
				
		vector<string> getDestinationOperands();
		vector<string> getSourceOperands();
		bool isBranchInstruction();
		bool isArithmeticInstruction();
};
}

#endif
