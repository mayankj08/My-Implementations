#include "apexSimulator.hpp"
#include "logger.hpp"
#include<climits>
#include "assert.h"

INIT_SIMPLE_LOG("")

// Copy constructor
// initializes fileName
cs520::apexSimulator::apexSimulator(std::string fileName)
{
	BEGIN();

	this->instructionFile = fileName;

	END();

}

// All major pipeline initializes are done here
// 1. ARCH Register file
// 2. Memory locations
bool cs520::apexSimulator::initialize()
{
	BEGIN();

	this->instFilePointer.open(this->instructionFile,fstream::in);	

	if(instFilePointer.fail())
	{
		cout << "Error:" <<strerror(errno)<<endl;
		return false;
	}
	// initialize registers with 0
	for(int i=0;i<ARCH_REGISTERS;i++)
	{
		stringstream ss;
		ss << "R" << i;
		registers[ss.str()] = "";
	}

	while(1)
	{
		cs520::instruction inst = getNextInstruction();

		// In case there are no more instruction to load
		if(inst.instName == "") 
			break;

		instMap[inst.instAddr]= inst;
		
	}

	for(int i=0;i<1000;i++)
		memoryLoc[i] = LONG_MAX;

	return true;
	END();
}

// Entry function to start simulation
bool cs520::apexSimulator::run(int count)
{
	BEGIN();
	bool status;
	status = simulate(count);

	return status;
	END();
}

// Reading instruction from files and storing the them
// in appropiate data structures.
cs520::instruction cs520::apexSimulator::getNextInstruction()
{
	BEGIN();
	cs520::instruction inst;
	string instStr="";

	getline(this->instFilePointer,instStr);

	if(!instFilePointer)
		return inst;
	
	vector<string> tokens;
	char delims[] = " ,";

	split(instStr,delims,tokens);

	size_t sizeToken = tokens.size();

	inst.instName = tokens[0];
	
	if(sizeToken >= 2)
		inst.op1 = tokens[1];
	else
		inst.op1 = "";
	
	if(sizeToken >= 3)
		inst.op2 = tokens[2];
	else
		inst.op2 = "";

	if(sizeToken >=4)
		inst.op3 = tokens[3];
	else
		inst.op3 = "";

	currLine = currLine +4;
	
	inst.instAddr = BASE_INST_ADDR + currLine;
	inst.fullInst = instStr;
	DEBUG("inst.instAddr:"<<inst.instAddr);

	return inst;
	END();
}

void cs520::apexSimulator::split(const string &s, const char* delim, vector<string> & v)
{
	BEGIN();
	DEBUG("Input string to split:"<<s);
	// to avoid modifying original string
	// first duplicate the original string and return a char pointer then free the memory
	char * dup = strdup(s.c_str());
	DEBUG("dup is:"<<dup);
	char * token = strtok(dup, delim);

	while(token != NULL)
	{
		DEBUG("token is:"<<string(token));
		v.push_back(string(token));
		// the call is treated as a subsequent calls to strtok:
		// the function continues from where it left in previous invocation
		token = strtok(NULL,delim);
	}
	free(dup);
	END();
}

bool cs520::apexSimulator::simulate(int count)
{
	bool status = true;

	
	// We have to keep calling above till we dont have more insruction left in pipeline.
	
	int countLeft = count;
	while(1)
	{
		bool continueExecution;
		bool isRunning = processInstruction(instMap,lastCycleBranchStatus,continueExecution);	 
		
		if(isRunning == false && completed == 0)
			completed = time;

		if(continueExecution == false)
		{
			completed = time;
			cout<<"Execution Halted due to Halt!!\n";
			break;
		}

		countLeft --;
		if(countLeft ==0)
			break;
	}
	
	return status;
}

bool cs520::apexSimulator::processInstruction(map<int,cs520::instruction> instMap, 
					      bool& isBranchTakenInLastCycle, 
					      bool& stopExecutionDueToHalt)
{
	
	BEGIN();
	bool isStalledinDRF=false;
	auto oldPipeline = getPipelineRowInstance();

	// Analyze last clock cycle. This analysis will tell us how can we
	// move ahead in this clock cycle
	// 1. Check if there is any dependency in last clock cycle in D/RF and ALU1.
	//    If, there is any such dependency then we cant move instruction which was
	//    in DRF in last cycle, ahead. But in this case in current cycle we can move
	//    result of INST2 to INST1 when INT2 needs the data. 
	//
	if(pipeline.begin() != pipeline.end()) //For case when t=0;
	{
		auto iter = pipeline.end();
		--iter; //Move to the state in last cycle
  	
		oldPipeline = *iter;

		auto instFetch = (*iter)[FETCH];
		auto instDRF = (*iter)[DRF];
		auto instALU1 = (*iter)[ALU1];		
		auto instALU2 = (*iter)[ALU2];
		auto instBranch = (*iter)[BRANCH];	

		
		DEBUG("Instruction in cycle " <<pipeline.size() 
			<<": [FETCH] "<<instFetch.fullInst
			<<"\n[DRF] "<<instDRF.fullInst
			<<"\n[ALU1] "<<instALU1.fullInst
			<<"\n[ALU2] "<<instALU2.fullInst
			<<" \n[Branch] "<<instBranch.fullInst
			<<" \n[DELAY] "<<((*iter)[DELAY]).fullInst
			<<" \n[MEM] "<<((*iter)[MEM]).fullInst
			<<" \n[WB] "<<((*iter)[WB]).fullInst);
	
		vector<string> dependentRegisters;
		
		// instruction would be stalled in DRF iff
		// 1. In last cycle instruction in ALU1 and DRF has true dependency. - 
		//		Would be of importance in case both the instruction are ALU instruction
		// 2. In last cycle instruction in ALU1 and DRF is control dependent. - 
		//		Would be of importance only if DRF instruction is BZ or BNZ
		isStalledinDRF = checkTrueDependency(instALU1,instDRF,dependentRegisters);

		if(isStalledinDRF == false && 
		   	instALU1.instName == "LOAD" && 
			checkTrueDependency(instALU1,instDRF,dependentRegisters))
		{
			isStalledinDRF = true;
		}

		if(isStalledinDRF == false && 
		   	instALU2.instName == "LOAD" && 
			checkTrueDependency(instALU2,instDRF,dependentRegisters))
		{
			isStalledinDRF = true;
		}
		
		// If below conditions then don't stall
		// 	1. If isStalledinDRF is True.
		//	2. instruction in DRF in old pipeline is STORE.
		//	3. Dependency in old DRF amd old ALUL1 is over operand 1 of STORE.
		//	4. If dependency in old DRF in old ALU1 is only over this operand.
		// Other way of saying (3) and (4) is if dependencyRegister contain register 
		// of operand 2. The we will need to stall else stall not required.	
		// Code : MOVC R0,#4
		//	  MOVC R6,#2
		//        STORE R6,R0,#0
		if(isStalledinDRF == true && instDRF.instName == "STORE")
		{
			auto source2OldDrf = instDRF.getSourceOperands()[1];
			if(find(dependentRegisters.begin(),dependentRegisters.end(),source2OldDrf) != dependentRegisters.end() )
			{
				isStalledinDRF = true;
			}
			else
			{
				DEBUG("Converting stall to don't stall");
				isStalledinDRF = false;
			}	
		}
	
		if(isStalledinDRF == false)
			isStalledinDRF = checkControlDependency(instALU1,instDRF);
		if(isStalledinDRF == false)
			isStalledinDRF = checkControlDependency(instBranch,instDRF);  //Register X dependency

		DEBUG("Value of isStalledinDRF:"<<isStalledinDRF);
		
	}	

	
	vector<cs520::instruction> currPipeline = getPipelineRowInstance();
	
	bool isRunning=true;
	bool isBranchTaken = false;
	int nextInstPcAddress = 0;
	
	// Reason of why WBstage is executed before all other stages are:
	// 1. We need Write in wb to be before read in DRF stage
	// 2. So that write of WB happens before READ of DRF
	
	stopExecutionDueToHalt = wbStage(oldPipeline,currPipeline);
	memStage(oldPipeline,currPipeline);
	alu2Stage(oldPipeline,currPipeline);
	alu1Stage(isStalledinDRF,oldPipeline,currPipeline,isBranchTakenInLastCycle);
	delayStage(oldPipeline,currPipeline);
	branchStage(oldPipeline,currPipeline,isBranchTaken,nextInstPcAddress,isBranchTakenInLastCycle,isStalledinDRF);
	drfStage(isStalledinDRF,oldPipeline,currPipeline,isBranchTakenInLastCycle);
	fetchStage(instMap,isStalledinDRF,oldPipeline,currPipeline);


	pipeline.push_back(currPipeline);

	// in case branch was taken
	// We will need to make PC point to newAddress and also to flush the instructions.

	DEBUG("Value of isBranchTaken is:"<<isBranchTaken);	
	if(isBranchTaken == true)
	{
		programCounter = nextInstPcAddress;
	
		// Now logic to flush DRF and (ALU1 or Branch) 
		// in case branch is taken
		isBranchTakenInLastCycle = true;
		
	}
	else
	{
		isBranchTakenInLastCycle = false;
	}

	DEBUG("programCounter:"<<programCounter);

	if(isRunning == true &&
		currPipeline[FETCH].instName == "" &&
		currPipeline[DRF].instName == "" &&
		currPipeline[ALU1].instName == "" &&
		currPipeline[ALU2].instName == "" &&
		currPipeline[BRANCH].instName == "" &&
		currPipeline[DELAY].instName == "" &&
		currPipeline[MEM].instName == "" &&
		currPipeline[WB].instName == "")
	{
		isRunning = false;
	}
	DEBUG("isRunning:"<<isRunning);
		time++;
	END();	
	return isRunning;
}

// Check True dependency of instructions I1 and I2 
// when I2 > I1
// and invoked as checkTrueDependency(I1,I2)
// Flag : this check is first operand of store is to be considered in dependecy check.
//	True : Yes we need to consider
//	False : We don't need to consider
bool cs520::apexSimulator::checkTrueDependency(cs520::instruction nextInst,
					       cs520::instruction prevInst,
					       vector<string>& dependent)
{
	BEGIN();
	bool isDependent = false;
	
	// In case any of previous of next instruction is not a real instrction then
	// we can't have dependency
	if(nextInst.instName == "" || prevInst.instName == "")
	{
		DEBUG("Either previous or next instruction name is empty");
		return isDependent;
	}
	
	vector<string> prevInstsource = prevInst.getSourceOperands();
	vector<string> nextInstDest = nextInst.getDestinationOperands();

	for (auto source : prevInstsource)
	{
		auto iter = find(nextInstDest.begin(),nextInstDest.end(),source);
		
		if(iter != nextInstDest.end())
		{
			DEBUG("Matched a dependent register:"<<*iter);
			dependent.push_back(*iter);
		}
	}

	if(dependent.size() !=0)
		isDependent = true;


	DEBUG("Returning from true dependeny with value:"<<isDependent);
	END();
	return isDependent;

}

// This function checks for control dependencies between two instructions
bool cs520::apexSimulator::checkControlDependency(cs520::instruction nextInst,cs520::instruction prevInst)
{
	BEGIN();
	if(nextInst.isArithmeticInstruction() && (prevInst.instName == "BZ" || prevInst.instName == "BNZ"))
	{
		DEBUG("Control dependency:true");
		return true;
	}
	else if(nextInst.instName == "BAL" && prevInst.instName == "JUMP")
	{
		DEBUG("Control dependency:true");
		return true;
	}

	DEBUG("Control dependency:false");
	END();

	return false;
}

// Creates a empty pipeline stage
vector<cs520::instruction> cs520::apexSimulator::getPipelineRowInstance()
{
	vector<cs520::instruction> inst;
	cs520::instruction temp;
	
	for(int i=0;i<8;i++)
		inst.push_back(temp);
	return inst;
}

bool cs520::apexSimulator::fetchStage(map<int,cs520::instruction> instMap,
							    bool isStalledinDRF,
							    vector<cs520::instruction> oldPipeline,
							    vector<cs520::instruction>& inst)
{
	BEGIN();
	//check if currently there is any instruction
 	// in fetch stage in last stage. If not we will need a new instruction

	if(time == 0)
	{
		auto iter = instMap.find(programCounter);
		if(iter == instMap.end())
		{
			DEBUG("No more instructions found to feed");
		}
		else
		{
			DEBUG("A new instruction "<<iter->second.instName<<" fetched");
			inst[FETCH] = iter->second;
			programCounter =programCounter+4;
		}
	}
	else
	{
		if(isStalledinDRF) //if something is stalled in DRF then nothing is to be done in this stage
		{
			inst[FETCH] = oldPipeline[FETCH];
		}
		else
		{
			auto iter = instMap.find(programCounter);
			if(iter == instMap.end())
			{
				; //No more instruction to feed
			}
			else
			{
				DEBUG("A new instruction "<<iter->second.instName<<" fetched");
				inst[FETCH] = iter->second;
				programCounter=programCounter+4;
			}
		}
	}
	END();
	return true;
}

bool cs520::apexSimulator::drfStage(bool isStalledinDRF,
			            vector<cs520::instruction> oldPipeline,
			            vector<cs520::instruction>& currPipeline,
				    bool isBranchTakenInLastCycle)
			       
{
	BEGIN();

	if(isBranchTakenInLastCycle == true)
	{
		DEBUG("Flushing DRF");
		currPipeline[DRF] = getPipelineRowInstance()[DRF];
		return true;
	}

	bool performDecoding = true;
	vector<string> dependentSources;

	if(time == 0)
	{
		return true;
	}
	else if(isStalledinDRF)
	{
		DEBUG("Instruction currently in DRF is:"<<oldPipeline[DRF].instName<<" and is stalled");
		currPipeline[DRF] = oldPipeline[DRF];
	}
	else if(!isStalledinDRF)
	{
		DEBUG("Instruction currently in DRF is:"<<oldPipeline[FETCH].instName);
		currPipeline[DRF] = oldPipeline[FETCH];
	}

	if( (!currPipeline[DRF].isBranchInstruction() || currPipeline[DRF].instName == "BAL")
		&& checkTrueDependency(currPipeline[ALU1],currPipeline[DRF],dependentSources))
	{
		if(currPipeline[DRF].instName == "STORE")
		{
			auto source2OldDrf = currPipeline[DRF].getSourceOperands()[1];
			if(find(dependentSources.begin(),dependentSources.end(),source2OldDrf) != dependentSources.end() )
			{
				performDecoding = false;		 
			}
			else
			{
				DEBUG("Converting performDecoding");
				performDecoding = true;
			}
		}	
		else
		{
			performDecoding = false;
		}
	}
	else if(!currPipeline[DRF].isBranchInstruction()
			&& checkTrueDependency(currPipeline[ALU2],currPipeline[DRF],dependentSources)
			&& currPipeline[ALU2].instName == "LOAD")
	{
		performDecoding = false;
	}

	if( currPipeline[DRF].isBranchInstruction() && checkControlDependency(currPipeline[ALU1],currPipeline[DRF]))
	{
		performDecoding = false;
	}
	else if(currPipeline[DRF].isBranchInstruction() &&
		checkTrueDependency(currPipeline[ALU1],currPipeline[DRF],dependentSources))
	{
		performDecoding = false;
	}
	if(currPipeline[DRF].instName == "JUMP" && currPipeline[BRANCH].instName == "BAL")
	{
		performDecoding = false;
	}

	DEBUG("Value of perform decoding is:"<<performDecoding);
	
	if(!performDecoding)
	{
		DEBUG("Not to perform decoding");
	}
	else if(performDecoding && !currPipeline[DRF].isBranchInstruction())
	{
		if(currPipeline[DRF].instName == "")
			return true;
			
		
		/* Only ALU instructions (not considering load and store instn.
		+-----------------------------------------------------------------------+
		|Scenario |Dependency between  	           |Stall in DRF|Forwarding? 	|
		|1.	  |DRF and ALU1			   |Y		|N	     	|
		|2.	  |DRF and ALU2			   |N		|N	     	|
		|3.       |DRF and MEM			   |N		|N	     	|
		|4.       |DRF and WB			   |N	        |N	     	|
		|5.	  |ALU1 and ALU2		   |N		|Y	     	|
		|6. 	  |ALU1 and MEM			   |N		|Y	     	|
		|7.	  |ALU1 and WB		           |N	        |Y	     	|			
		|8.	  |ALU2 and MEM			   |NA		|NA	     	|	
		|9.       |ALU2 and WB			   |NA		|NA	     	|
		|10.	  |MEM and WB			   |NA		|NA		|
		+-----------------------------------------------------------------------+*/
		
		// Checking if source register of current registers is destination
		// of any instruction previously in ALU1,ALU2
		// In case instruction which is currently in DRF is dependent on instn which
		// was in DRF in last cycle then it won't ever come in this condtion. As such instruction
		// would be stalled in DRF only.
		vector<string> dependentSource;

		DEBUG("About to check dependecy with old ALU1");
		checkTrueDependency(oldPipeline[ALU1],currPipeline[DRF],dependentSource);

		DEBUG("About to check dependeny with old ALU2");
		DEBUG("oldPipeline[ALU1]:"<<oldPipeline[ALU1].fullInst);
		DEBUG("oldPipeline[ALU2]:"<<oldPipeline[ALU2].fullInst);
		checkTrueDependency(oldPipeline[ALU2],currPipeline[DRF],dependentSource);
			
		// Below for is for debugging purpose
		for(auto temp:dependentSource)
			DEBUG("Dependent Source"<<temp<<" ");		
	
		auto source = currPipeline[DRF].getSourceOperands();
		
		int counter = 0;
		for (auto src:	source)
		{
			// if source is a literal as in case of MOVC instruction
			if(src[0] == '#')
			{
				DEBUG("src is:"<<src);
				DEBUG("src.substr(1) is:"<<src.substr(1));
				auto temp =std::make_pair(stoi(src.substr(1)),true);
				currPipeline[DRF].decodedValue.push_back(temp);
				counter++;
				continue;
			}
			
			// For store :
			// For first operand 
			if(counter ==0 && currPipeline[DRF].instName == "STORE")
			{
				bool needForwarding = false;
				auto memDestOp = currPipeline[MEM].getDestinationOperands();
				auto alu1DestOp = currPipeline[ALU1].getDestinationOperands();
				auto alu2DestOp = currPipeline[ALU2].getDestinationOperands();
				counter++;

				if( find(memDestOp.begin(),memDestOp.end(),src) != memDestOp.end())
				{
					auto temp =std::make_pair(0,false);
					currPipeline[DRF].decodedValue.push_back(temp);	
					continue;
				}
				else if(find(alu1DestOp.begin(),alu1DestOp.end(),src) != alu1DestOp.end())
				{
					auto temp =std::make_pair(0,false);
					currPipeline[DRF].decodedValue.push_back(temp);	
					continue;
				}
				else if(find(alu2DestOp.begin(),alu2DestOp.end(),src) != alu2DestOp.end())
				{
					auto temp =std::make_pair(0,false);
					currPipeline[DRF].decodedValue.push_back(temp);	
					continue;
				}
				else
				{
					auto temp =std::make_pair(getRegisterVal(src),true);
					currPipeline[DRF].decodedValue.push_back(temp);	
					continue;
				}
			}
			
			auto iter = find(dependentSource.begin(),dependentSource.end(),src);
			if(iter == dependentSource.end()) //Not dependent source
			{
				DEBUG("About to insert:"<<getRegisterVal(src));
				auto temp =std::make_pair(getRegisterVal(src),true);
				currPipeline[DRF].decodedValue.push_back(temp);	
			}
			else
			{
				auto temp =std::make_pair(0,false);
				currPipeline[DRF].decodedValue.push_back(temp);
			}	
			counter++;
		}
		
	}
	else if(performDecoding && currPipeline[DRF].isBranchInstruction())
	{
		
		// Here comes another twist:
		// in case of BZ or BNZ we should have two decoded values:
		// 1. The value of literal which is provided along with BZ or BNZ like BZ,#20
		// 2. The value of zero flag.
		// So in this case the zero'th element of decodedValue would store literal value 
		// and first element of decodedValue vector would contain zero flag.

		if(currPipeline[DRF].instName == "BZ" || currPipeline[DRF].instName == "BNZ")
		{
			auto source = currPipeline[DRF].getSourceOperands();
			// in this case we are just expecting a literal value
			auto src = source[0];

			if(src.substr(1) == "")
			{
				FATAL("Invalid value");
				assert(false);
				
			}
			auto temp = std::make_pair(stoi(src.substr(1)),true);
			currPipeline[DRF].decodedValue.push_back(temp);

			// Getting literal value was easy. But getting zero flag is not so easy
			// "Life is hard and it gets worse and worse and worse" and so it this code
			// anyway let's kill it.
			
			// Below few lines of code is for below logic.
			// Zero flag either can be take from apex register of from particualr instruction.
			// zeroFlag is readily avialable when:
			// There is no Arithmetic instruction in MEM,ALU2,ALU1,DRF in last clock cycle. 
			
			if(oldPipeline[DRF].isArithmeticInstruction()  || 
			   oldPipeline[ALU1].isArithmeticInstruction() ||
			   oldPipeline[ALU2].isArithmeticInstruction() ||
			   oldPipeline[MEM].isArithmeticInstruction())
			{
				auto temp = std::make_pair(1,false);
				currPipeline[DRF].decodedValue.push_back(temp);
			}
			else
			{
				auto temp = std::make_pair(zeroFlag,true);
				currPipeline[DRF].decodedValue.push_back(temp);
			
			}
			
		}
		else if(currPipeline[DRF].instName == "BAL")
		{
			DEBUG("Got a BAL instruction");
			auto source = currPipeline[DRF].getSourceOperands();
			vector<string> dependentSource;

			DEBUG("About to check dependecy with old ALU1");
			checkTrueDependency(oldPipeline[ALU1],currPipeline[DRF],dependentSource);

			DEBUG("About to check dependeny with old ALU2");
			checkTrueDependency(oldPipeline[ALU2],currPipeline[DRF],dependentSource);

			auto src = source[0];
			auto iter = find(dependentSource.begin(),dependentSource.end(),src);
			if(iter == dependentSource.end()) //Not dependent source
			{
				DEBUG("About to insert:"<<getRegisterVal(src));
				auto temp =std::make_pair(getRegisterVal(src),true);
				currPipeline[DRF].decodedValue.push_back(temp);	
			}
			else
			{
				DEBUG("Found a dependency");
				auto temp =std::make_pair(0,false);
				currPipeline[DRF].decodedValue.push_back(temp);
			}	

			//Saving the literal value
			auto src2 = source[1];
			auto temp2 = std::make_pair(stoi(src2.substr(1)),true);
			currPipeline[DRF].decodedValue.push_back(temp2);
		}
		else if(currPipeline[DRF].instName == "JUMP")
		{
			auto source = currPipeline[DRF].getSourceOperands();
			vector<string> dependentSource;

			if(source[0] == "X")
			{
				DEBUG("Obtained register X");	
				// check if there is any BAL instruction currently in 
				// DELAY or MEM.
				bool isValPresent = false;
				isValPresent = checkControlDependency(currPipeline[DELAY],currPipeline[DRF]);
				if(isValPresent == false)
					checkControlDependency(currPipeline[MEM],currPipeline[DRF]);
			
				if(isValPresent == true)
				{
					auto temp =std::make_pair(0,false);
					currPipeline[DRF].decodedValue.push_back(temp);
				}
				else
				{
					auto temp = std::make_pair(stoi(regX),true);
					currPipeline[DRF].decodedValue.push_back(temp);
				}
			}
			else
			{
					
				DEBUG("About to check dependecy with old ALU1");
				checkTrueDependency(oldPipeline[ALU1],currPipeline[DRF],dependentSource);

				DEBUG("About to check dependeny with old ALU2");
				checkTrueDependency(oldPipeline[ALU2],currPipeline[DRF],dependentSource);

				auto src = source[0];
				auto iter = find(dependentSource.begin(),dependentSource.end(),src);
				if(iter == dependentSource.end()) //Not dependent source
				{
					DEBUG("About to insert:"<<getRegisterVal(src));
					auto temp =std::make_pair(getRegisterVal(src),true);
					currPipeline[DRF].decodedValue.push_back(temp);	
				}
				else
				{
					DEBUG("Found a dependency");
					auto temp =std::make_pair(0,false);
					currPipeline[DRF].decodedValue.push_back(temp);
				}	

			}

			//Saving the literal value
                        auto src2 = source[1];
                        auto temp2 = std::make_pair(stoi(src2.substr(1)),true);
                        currPipeline[DRF].decodedValue.push_back(temp2);
		}
		
	}

	// Just for debugging purpose
	for(auto dec : currPipeline[DRF].decodedValue)
	{
		DEBUG("Decoded info:dec.first"<<dec.first<<" dec.second:"<<dec.second);	
	}	
	

	END();
	return true;
}

bool cs520::apexSimulator::alu1Stage(bool isStalledinDRF,
					vector<cs520::instruction> oldPipeline,
			            	vector<cs520::instruction>& currPipeline,
					bool isBranchTakenInLastCycle)
{
	if(isBranchTakenInLastCycle == true)
	{
		DEBUG("Flushing ALU1 stage");
		currPipeline[ALU1] = getPipelineRowInstance()[ALU1];
		return true;
	}

	if(time == 0)
	{
		return true;
	}
	else if(isStalledinDRF)
	{
		;
	}
	else if(!oldPipeline[DRF].isBranchInstruction())
	{
		currPipeline[ALU1] = oldPipeline[DRF];

		if(currPipeline[ALU1].instName == "STORE")
		{
			// In case current instruction is store 
			// we will check if first operand of store has decoded value or not.
			if(currPipeline[ALU1].decodedValue[0].second == false)
			{
				DEBUG("ALU1 and need to decode first operand of STORE");
				//check if inst in WB is dependent on current inst.
				vector<string> dep;
				auto wbDest = currPipeline[WB].getDestinationOperands();
				auto memDest = currPipeline[MEM].getDestinationOperands();
				auto alu2Dest = currPipeline[ALU2].getDestinationOperands();
				auto alu1Source1 = (currPipeline[ALU1].getSourceOperands())[0];
				// Below if checks if both of below conditions are met or not. If met then	
				// performForwarding is set to true.
				// Condition 1 : First source of store (STORE R1,R2,#10 : R1 is first source)
				//              matches destination of WB.
				// Condition 2 : First source of store doesn't matches destination of ALU2,MEM,WB
				if((find(wbDest.begin(),wbDest.end(),alu1Source1) != wbDest.end()) 
					&& (find(memDest.begin(),memDest.end(),alu1Source1) == memDest.end())
					&& (find(alu2Dest.begin(),alu2Dest.end(),alu1Source1) == alu2Dest.end()))
				{
					currPipeline[ALU1].decodedValue[0].second = true;
					currPipeline[ALU1].decodedValue[0].first = stoi(currPipeline[WB].result);
				}
			}
		}


		int operandNumber =-1;
		auto sourceOps = currPipeline[ALU1].getSourceOperands();
		// Here we need to need to check if there is any operand value for which
		// was not available in DRF in case there is one we need to have forwaded 
		// value for same
		for(auto &src : currPipeline[ALU1].decodedValue)
		{
			operandNumber++;

			if(currPipeline[ALU1].instName == "STORE" && operandNumber == 0)
			{
				continue;
			}

			if(src.second == false)
			{
				DEBUG("src.second is false");
				// We need to get this value via forwarding from somewhere.	
				// checking if current insturction has dependency on one 
				// currently in ALU2 stage	
				vector<string> dependent;
				bool isDependent = checkTrueDependency(currPipeline[ALU2],currPipeline[ALU1],dependent);
				
				if(isDependent && (find(dependent.begin(),dependent.end(),sourceOps[operandNumber]) != dependent.end()))
				{
					DEBUG("Forwarding from ALU2");
					// this means the value we are searching for is present in ALU2
					src.first = stoi(currPipeline[ALU2].result);
					continue;
				}
				
				dependent.clear();
				isDependent = checkTrueDependency(currPipeline[MEM],currPipeline[ALU1],dependent);
				
				if(isDependent && (find(dependent.begin(),dependent.end(),sourceOps[operandNumber]) != dependent.end()))
				{
					DEBUG("Forwarding from MEM");
					// this means the value we are searching for is present in ALU2
					src.first = stoi(currPipeline[MEM].result);
					continue;
				}

				dependent.clear();
				isDependent = checkTrueDependency(currPipeline[WB],currPipeline[ALU1],dependent);
				
				if(isDependent && (find(dependent.begin(),dependent.end(),sourceOps[operandNumber]) != dependent.end()))
				{
					DEBUG("Forwarding from WB");
					// this means the value we are searching for is present in ALU2
					src.first = stoi(currPipeline[WB].result);
					continue;
				}
			
			}
		}
		
	}
	DEBUG("Current instruction in ALU1 is:"<<currPipeline[ALU1].instName);
	return true;
}

bool cs520::apexSimulator::alu2Stage(vector<cs520::instruction> oldPipeline,
			            vector<cs520::instruction>& currPipeline)
{
	BEGIN();
	if(time == 0)
	{
		return true;
	}
	else
	{
		currPipeline[ALU2] = oldPipeline[ALU1];
		DEBUG("Current instruction in ALU2 is"<<currPipeline[ALU2].instName);

		if(currPipeline[ALU2].instName == "STORE")
		{
			// In case current instruction is store 
			// we will check if first operand of store has decoded value or not.
			if(currPipeline[ALU2].decodedValue[0].second == false)
			{
				//check if inst in WB is dependent on current inst.
				vector<string> dep;
				bool performForwarding=false;
				auto wbDest = currPipeline[WB].getDestinationOperands();
				auto memDest = currPipeline[MEM].getDestinationOperands();
				auto alu2Source1 = (currPipeline[ALU2].getSourceOperands())[0];
				// Below if checks if both of below conditions are met or not. If met then	
				// performForwarding is set to true.
				// Condition 1 : First source of store (STORE R1,R2,#10 : R1 is first source)
				//              matches destination of WB.
				// Condition 2 : First source of store doesn't matched destination of MEM
				if((find(wbDest.begin(),wbDest.end(),alu2Source1) != wbDest.end()) 
					&& (find(memDest.begin(),memDest.end(),alu2Source1) == memDest.end()))
				{
					currPipeline[ALU2].decodedValue[0].second = true;
					currPipeline[ALU2].decodedValue[0].first = stoi(currPipeline[WB].result);
				}
			}
		}
	
		// Checking if there is any need of data forwarding
		if(currPipeline[ALU2].instName != "" || currPipeline[ALU2].instName != "HALT")
		{	
			currPipeline[ALU2].result = computeResult(currPipeline[ALU2],oldPipeline);

			// This is to be done only in case of ALU instructions
			// We get here only in case of all arithmetic instructions
			
			if(currPipeline[ALU2].isArithmeticInstruction() && currPipeline[ALU2].result == "0")
			{
				currPipeline[ALU2].zeroFlag=true;
				DEBUG("Setting zeroFlag to true");
			}
		}
	}
	END();
	return true;
}

bool cs520::apexSimulator::branchStage(vector<cs520::instruction> oldPipeline,
				       vector<cs520::instruction>& currPipeline,
				       bool& isBranchTaken,
				       int& nextInstPcAddress,
				       bool isBranchTakenInLastCycle,
				       bool isStalledinDRF)
{

	if(isBranchTakenInLastCycle == true)
	{
		currPipeline[BRANCH].instName = "";
		return true;
	}

	if(time == 0)
	{
		return true;
	}
	else if(isStalledinDRF == true)
	{
		;
	}
	else if(oldPipeline[DRF].instName=="BZ" || oldPipeline[DRF].instName=="BNZ")
	{
		currPipeline[BRANCH] = oldPipeline[DRF];

		int offset = (currPipeline[BRANCH].decodedValue[0]).first;
		bool isZeroFlagAvailable = (currPipeline[BRANCH].decodedValue[1]).second;
		bool localZeroFlag;
		
		if(isZeroFlagAvailable == true)
		{
			localZeroFlag = (currPipeline[BRANCH].decodedValue[1]).first;
		}
		else
		{	
			localZeroFlag = false;
		
			if(currPipeline[ALU2].isArithmeticInstruction())
			{
				DEBUG("Forwarding from curr ALU2");
				if(currPipeline[ALU2].result == "0")
					localZeroFlag = true;
			}
			if(!localZeroFlag && oldPipeline[ALU2].isArithmeticInstruction())
			{
				DEBUG("Forwarding from old ALU2.. Result is"<<oldPipeline[ALU2].result);
				if(oldPipeline[ALU2].result == "0")
					localZeroFlag = true;		
			}
			if(!localZeroFlag && oldPipeline[MEM].isArithmeticInstruction())
			{
				DEBUG("Forwarding from old MEM");
				if(oldPipeline[MEM].result == "0")
					localZeroFlag = true;		
			}
		}
		DEBUG("Value of localZeroFlag is:"<<localZeroFlag);	

		// If instruction is BZ or BNZ then we need to check the value of zeroFlag
		if(currPipeline[BRANCH].instName == "BZ" && localZeroFlag == true)
		{
			isBranchTaken = true;
			nextInstPcAddress = currPipeline[BRANCH].instAddr + offset;
			nextInstPcAddress = nextInstPcAddress - (nextInstPcAddress%4);
		}
		else if(currPipeline[BRANCH].instName == "BNZ" && localZeroFlag == false)
		{
			isBranchTaken = true;
			nextInstPcAddress = currPipeline[BRANCH].instAddr + offset;
			nextInstPcAddress = nextInstPcAddress - (nextInstPcAddress%4);
		}
		else
		{
			isBranchTaken = false;
		}
	}
	else if(oldPipeline[DRF].instName == "BAL")
	{
		currPipeline[BRANCH] = oldPipeline[DRF];

		// First decode value is Register value in this case
		// Check if the value of register is readily available
		bool isRegValAvailable = (currPipeline[BRANCH].decodedValue[0]).second;
		
		int val;
		if(isRegValAvailable == true)
		{
			val = (currPipeline[BRANCH].decodedValue[0]).first;	
			DEBUG("Value of register reterived for BAL is:"<<val);
		}
		else
		{
			bool valFound = false;

			vector<string> dependent;
			auto source = currPipeline[BRANCH].getSourceOperands();
			bool isDependent = checkTrueDependency(currPipeline[ALU2],currPipeline[BRANCH],dependent);
				
			if(isDependent && (find(dependent.begin(),dependent.end(),source[0]) != dependent.end()))
			{
				// this means the value we are searching for is present in ALU2
				val = stoi(currPipeline[ALU2].result);
				valFound = true;
			}
				
			dependent.clear();
			isDependent = checkTrueDependency(currPipeline[MEM],currPipeline[BRANCH],dependent);
				
			if(valFound == false && isDependent && (find(dependent.begin(),dependent.end(),source[0]) != dependent.end()))
			{
				// this means the value we are searching for is present in ALU2
				val = stoi(currPipeline[MEM].result);
				DEBUG("Forwaded value is:"<<val);
				valFound = true;
			}

			dependent.clear();
			isDependent = checkTrueDependency(currPipeline[WB],currPipeline[BRANCH],dependent);
				
			if(valFound == false && isDependent && (find(dependent.begin(),dependent.end(),source[0]) != dependent.end()))
			{
				// this means the value we are searching for is present in ALU2
				val = stoi(currPipeline[WB].result);
				valFound = true;
			}

			if(valFound == false)
				assert(false);
		}

		// Second decode value is literal
		int offset = (currPipeline[BRANCH].decodedValue[1]).first;

		isBranchTaken = true;
		nextInstPcAddress = val + offset;
		nextInstPcAddress = nextInstPcAddress - (nextInstPcAddress%4);
		DEBUG("val:"<<val<<" offset:"<<offset);
		DEBUG("nextInstPcAddress:"<<nextInstPcAddress); 
		stringstream ss;
		DEBUG("currPipeline[BRANCH].instAddr:"<<currPipeline[BRANCH].instAddr);
		ss << (currPipeline[BRANCH].instAddr +4);
		currPipeline[BRANCH].result = ss.str();
		DEBUG("Result is:"<<ss.str());
	}
	else if(oldPipeline[DRF].instName == "JUMP")
	{
		currPipeline[BRANCH] = oldPipeline[DRF];
		
		int offset = (currPipeline[BRANCH].decodedValue[1]).first;
		DEBUG("offset:"<<offset);
		
		isBranchTaken = true;
		
		int val;
		if(currPipeline[BRANCH].decodedValue[0].second) //If value is true
		{
			val = currPipeline[BRANCH].decodedValue[0].first;
		}
		else
		{
			// Forward value of register here..

			auto srcs = currPipeline[BRANCH].getSourceOperands();
			if(srcs[0] == "X")
			{
				// Forwarding of X register
				
				if(currPipeline[DELAY].instName == "BAL")
				{
					val = stoi(currPipeline[DELAY].result);
				}
				else if(currPipeline[MEM].instName == "BAL")
				{
					val = stoi(currPipeline[MEM].result);
				}
				else if(currPipeline[WB].instName == "BAL")
				{
					val = stoi(currPipeline[WB].result);
				}
				else
				{
					assert(false);
				}
			}
			else
			{	
				bool isValueObtained = getForwadedValue(currPipeline[MEM],currPipeline[BRANCH],val);
				
				if(isValueObtained == false)
					isValueObtained = getForwadedValue(currPipeline[WB],currPipeline[BRANCH],val);
					
				if(isValueObtained == false)
					assert(false);
			}	
		}
		nextInstPcAddress = offset+val;
		nextInstPcAddress = nextInstPcAddress - (nextInstPcAddress%4);
		DEBUG("nextInstPcAddress:"<<nextInstPcAddress);
	}
	return true;
}

bool cs520::apexSimulator::getForwadedValue(const cs520::instruction& inst1,const cs520::instruction& inst2,int& val)
{
	
	vector<string> dependent;
	bool isDependent = checkTrueDependency(inst1,inst2,dependent);
	
	if(isDependent)
	{
		val = stoi(inst1.result);
		return true;
	}
	return false;
} 

bool cs520::apexSimulator::delayStage(vector<cs520::instruction> oldPipeline,
				       vector<cs520::instruction>& currPipeline)
{
	if(time == 0)
	{
		return true;
	}
	else
	{
		currPipeline[DELAY] = oldPipeline[BRANCH];
	}
	return true;
}

bool cs520::apexSimulator::memStage(vector<cs520::instruction> oldPipeline,
				       vector<cs520::instruction>& currPipeline)
{
	BEGIN();
	if(time == 0)
	{
		return true;
	}
	else if(oldPipeline[ALU2].instName != "")
	{
		currPipeline[MEM] = oldPipeline[ALU2];
		
		// Memory operations here
		if(currPipeline[MEM].instName == "LOAD")
		{
			int computedMemoryLocation = stoi(currPipeline[MEM].result);
			DEBUG("Load instn with computedMemoryLocation as"<<computedMemoryLocation);
			int val= memoryLoc[computedMemoryLocation/4];
			
			DEBUG("Value as retervied from memory location is"<<val);

			stringstream ss;
			ss << val;
			currPipeline[MEM].result = ss.str();
		}
		else if(currPipeline[MEM].instName == "STORE")
		{
			int computedMemoryLocation = stoi(currPipeline[MEM].result);
			int valueToStore;

			DEBUG("Number of STORE operands:"<<currPipeline[MEM].decodedValue.size());

			// need forwarding
			if(currPipeline[MEM].decodedValue[0].second == false)
			{
				// There's only one possiblity here.. This particular value should be 
				// available in WB so we will forward from WB.
				DEBUG("Forwarding from WB");
				auto wbResult = currPipeline[WB].result;
				if(wbResult == "")
				{
					FATAL("Something wrong!!...Cycle:"<<time);
					assert(false);
				}				

				valueToStore= stoi(currPipeline[WB].result);
			}
			else
			{
				valueToStore = currPipeline[MEM].decodedValue[0].first;		
			}
			computedMemoryLocation = computedMemoryLocation/4;
			DEBUG("computedMemoryLocation:"<<computedMemoryLocation);
			DEBUG("About to store:"<<valueToStore);
			memoryLoc[computedMemoryLocation] = valueToStore;
			
		}
	}
	else if(oldPipeline[DELAY].instName != "")
	{
		currPipeline[MEM] = oldPipeline[DELAY];
	}
		
	DEBUG("instruction level zero flage here is:"<<currPipeline[MEM].zeroFlag);
	
	END();

	return true;
}


bool cs520::apexSimulator::wbStage(vector<cs520::instruction> oldPipeline,
				   vector<cs520::instruction>& currPipeline)
{
	BEGIN();
	if(time == 0)
	{
		return true;
	}
	else 
	{
		
		if(oldPipeline[MEM].instName == "HALT")
		{
			currPipeline[WB] = oldPipeline[MEM];
			DEBUG("Halt instruction encountered..." );	
			return false;
		}

		currPipeline[WB] = oldPipeline[MEM];
		DEBUG("Copied old MEM instr in WB");
		
		// Write result to correct register
		if(currPipeline[WB].instName == "STORE")
		{
			// Nothing is to be written in this stage
			// incase of STORE instn.
			;
		}
		else if(currPipeline[WB].instName == "BAL")
		{
			//need to write value of regX in RF
			regX = currPipeline[WB].result;
		}
		else if(currPipeline[WB].instName != "")
		{
			DEBUG("Instruction currently in WB is:"<<currPipeline[WB].instName);
			auto dest = currPipeline[WB].getDestinationOperands();
			for(auto d : dest)
				DEBUG("Dest:"<<d);
			
			if(currPipeline[WB].result != "")
				registers[dest[0]] = currPipeline[WB].result;

			// time to write zero flag too
			// but we need to change this flag iff current instruction a Alu instruction
			if(currPipeline[WB].isArithmeticInstruction())
				zeroFlag = currPipeline[WB].zeroFlag;
		}

		
	}
	END();
	return true;
}

string cs520::apexSimulator::computeResult(cs520::instruction inst,vector<cs520::instruction> oldInst)
{
	BEGIN();
	if( inst.instName == "ADD" || 
            inst.instName == "SUB" || 
            inst.instName == "MUL" ||
            inst.instName == "AND" || 
	    inst.instName == "OR"  || 
	    inst.instName == "EX-OR")
	{
		int result;
		
		auto src1 = inst.decodedValue[0].first;
		auto src2 = inst.decodedValue[1].first;		

		if(inst.instName == "ADD")
		{
			result = src1+src2;
			DEBUG("Cycle:"<<time<<"Add result is:"<<result);
		}
		else if(inst.instName == "SUB")
		{
			result = src1-src2;
		}
		else if(inst.instName == "MUL")
		{
			result = src1*src2;
		}
		else if(inst.instName == "AND")
		{
			result = src1 & src2;
		}
		else if(inst.instName == "OR")
		{
			result = src1 | src2;
		}
		else if(inst.instName == "EX-OR")
		{
			result = src1 ^ src2;
		}


		DEBUG("Setting instruction level zero flag to:"<<inst.zeroFlag);

		stringstream ss;
		ss << result;
		return ss.str(); 	
	}
	else if(inst.instName == "MOVC")
	{
		int result = inst.decodedValue[0].first;

		stringstream ss;
		ss << result;
		return ss.str(); 	

	}
	else if(inst.instName == "LOAD")
	{
		auto src1 = inst.decodedValue[0].first;
		auto src2 = inst.decodedValue[1].first;
		int result = src1+src2;

		stringstream ss;
		ss << result;
		return ss.str();		
	}
	else if(inst.instName == "STORE")
	{

		auto src1 = inst.decodedValue[1].first;
		auto src2 = inst.decodedValue[2].first;
		
		DEBUG("Src1 for store:"<<src1<<" Src2 for store:"<<src2);
		int result = src1+src2;

		stringstream ss;
		ss << result;
		return ss.str();		
	}
	END();
	return "";
}

int cs520::apexSimulator::getRegisterVal(string reg)
{

	auto iter = registers.find(reg);
	
	if(iter == registers.end())
	{
		
		DEBUG("Input to getRegisterVal:"<<reg);	
		return 0;
	}
		
	DEBUG("iter->second:"<<iter->second);	
	if(iter->second == "")
		return 0;	
	return stoi(iter->second);

}


void cs520::apexSimulator::show(int clockCycle)
{
	cout<<"Gantt Chart:\n";
	
	auto row = pipeline[clockCycle-1];
			
	cout<<"[FETCH]";
	if(row[FETCH].instName != "")
		cout<<row[FETCH].fullInst<<"\n";
	else
		cout<<"-\n";
	
	cout<<"[DRF]";
	if(row[DRF].instName != "")
		cout<<row[DRF].fullInst<<"\n";
	else
		cout<<"-\n";

	cout<<"[ALU1]";
	if(row[ALU1].instName != "")
		cout<<row[ALU1].fullInst<<"\n";
	else
		cout<<"-\n";

	cout<<"[ALU2]";
	if(row[ALU2].instName != "")
		cout<<row[ALU2].fullInst<<"\n";
	else
		cout<<"-\n";
	
	cout<<"[BRANCH]";
	if(row[BRANCH].instName != "")
		cout<<row[BRANCH].fullInst<<"\n";
	else	
		cout<<"-\n";

	cout<<"[DELAY]";
	if(row[DELAY].instName != "")
		cout<<row[DELAY].fullInst<<"\n";
	else
		cout<<"-\n";

	cout<<"[MEM]";
	if(row[MEM].instName != "")
		cout<<row[MEM].fullInst<<"\n";
	else	
		cout<<"-\n";

	cout<<"[WB]";
	if(row[WB].instName != "")
		cout<<row[WB].fullInst<<endl;
	else
		cout<<"-\n"<<endl;
}


void cs520::apexSimulator::display()
{
	cout<<"\nPipeline stage:\n";
	
	auto row = pipeline[pipeline.size()-1];
			
	cout<<"[FETCH]: ";
	if(row[FETCH].instName != "")
		cout<<row[FETCH].fullInst<<"\n";
	else
		cout<<"\n";
	
	cout<<"[DRF]: ";
	if(row[DRF].instName != "")
		cout<<row[DRF].fullInst<<"\n";
	else
		cout<<"\n";

	cout<<"[ALU1]: ";
	if(row[ALU1].instName != "")
		cout<<row[ALU1].fullInst<<"\n";
	else
		cout<<"\n";

	cout<<"[ALU2]: ";
	if(row[ALU2].instName != "")
		cout<<row[ALU2].fullInst<<"\n";
	else
		cout<<"\n";
	
	cout<<"[BRANCH]: ";
	if(row[BRANCH].instName != "")
		cout<<row[BRANCH].fullInst<<"\n";
	else	
		cout<<"\n";

	cout<<"[DELAY]: ";
	if(row[DELAY].instName != "")
		cout<<row[DELAY].fullInst<<"\n";
	else
		cout<<"\n";

	cout<<"[MEM]: ";
	if(row[MEM].instName != "")
		cout<<row[MEM].fullInst<<"\n";
	else	
		cout<<"\n";

	cout<<"[WB]: ";
	if(row[WB].instName != "")
		cout<<row[WB].fullInst<<endl;
	else
		cout<<"\n"<<endl;


	cout<<"\nRegisters"<<endl;
	for(auto a:registers)
	{
		cout<<a.first<<" "<<a.second<<endl;
	}
	cout<<"\nZero flag:"<<zeroFlag<<endl;

	cout<<"\nMemory"<<endl;
	for(int i=0;i<1000;i++)
	{
		if(memoryLoc[i] != LONG_MAX)
			cout<<4*i<<"-"<<4*i+3<<":"<<memoryLoc[i]<<endl;
	}
	
	cout<<"\nRegister X:"<<regX<<endl;
	
	cout <<"\nClock to complete:"<<completed<<endl;
}

string cs520::apexSimulator::getRegisterValue(string registerName)
{
	auto iter =  registers.find(registerName);
	return iter->second;		
}

int cs520::apexSimulator::getNumberOfCyclesRan()
{
	return ((pipeline.size())-1); 
}

int cs520::apexSimulator::getCompleted()
{
	return completed;
}
