#include "instruction.hpp"

vector<string> cs520::instruction::getDestinationOperands()
{
	vector<string> dest;
	if(instName == "ADD" || 
	   instName == "SUB" || 
	   instName == "MOVC"||
	   instName == "MUL" ||
	   instName == "AND" ||
	   instName == "OR"  ||
           instName == "EX-OR")
	{
		dest.push_back(op1);
	}
	else if(instName == "LOAD")
	{
		dest.push_back(op1);
	}
	
	return dest;
}

vector<string> cs520::instruction::getSourceOperands()
{
	vector<string> source;
	
	if(instName == "ADD" || 
	   instName == "SUB" || 
	   instName == "MUL" ||
	   instName == "AND" ||
	   instName == "OR"  ||
           instName == "EX-OR")
	{
		source.push_back(op2);
		source.push_back(op3);
	}
	else if(instName == "MOVC")
	{
		source.push_back(op2);
	}
	else if(instName == "BZ" || instName == "BNZ")
	{
		source.push_back(op1);
	}
	else if(instName == "BAL")
	{
		source.push_back(op1);
		source.push_back(op2);
	}
	else if(instName == "JUMP")
	{
		source.push_back(op1);
		source.push_back(op2);
	}
	else if(instName == "LOAD")
	{
		source.push_back(op2);
		source.push_back(op3);
	}
	else if(instName == "STORE")
	{
		source.push_back(op1);
		source.push_back(op2);
		source.push_back(op3);
	}
	
	return source;
}

bool cs520::instruction::isBranchInstruction()
{
	if(instName == "BZ" || instName == "BNZ" || instName == "BAL" || instName == "JUMP")
	{
		return true;	
	}
	return false;
}

bool cs520::instruction::isArithmeticInstruction()
{

	if(instName == "ADD" || 
	   instName == "SUB" || 
	   instName == "MUL" ||
	   instName == "AND" ||
	   instName == "OR"  ||
           instName == "EX-OR")
	{
		return true;
	}
	return false;
}

