#include<iostream>
#include "apexSimulator.hpp"

using std::cin;


int main(int argc, char** argv)
{
	bool status = true;
	if(argc < 2)
	{
		cout<<"Input file name missing"<<endl;
		return false;
	}
	
	if(argc == 3)
	{
		if(argv[2][0] == '1')
		{
			//Running in test mode
			// setting bad bit so that cout doesn't print anything in debug mode
			std::cout.setstate(std::ios_base::badbit);
		}
	}

	cs520::apexSimulator simulatorObj(argv[1]);

	cout<<"APEx Simulator!!"<<std::endl;
	std::string inp;
	bool isInitialized = false;
	while(1)
	{
		cout << ":";
		cin >> inp;
		if(inp == "Exit" || inp == "exit")
		{
			break;
		}
		else if(inp == "initialize")
		{
			simulatorObj.initialize();
			isInitialized = true;
		} 
		else if(inp == "simulate" || inp == "s")
		{
			if(isInitialized == false)
			{
				cout<<"Not initialized yet!. Ran initialize command by itself\n";
				cout<<":initialize\n";
				simulatorObj.initialize();
				isInitialized = true;
			}
			int count;
			cin >> count;
			status = simulatorObj.run(count);
		}
		else if(inp == "display")
		{
			simulatorObj.display();
		}
		else if(inp =="show")
		{
			int clockCycle;
			cin >> clockCycle;
			simulatorObj.show(clockCycle);
		}
		else
		{
			cout<<"Incorrect command!\n";
		}
	}
	
	return status;
}
