// LOG LEVELS
// 1 -> FATAL
// 2 -> ERROR
// 3 -> WARN
// 4 -> DEBUG
// 5 -> BEGIN

#include <fstream>
#include<iostream>
using namespace std;

#define INIT_SIMPLE_LOG(level)	  						\
	ofstream logFile("log/simulatorLogs.txt");

#define LOG_LEVEL 6

#if (LOG_LEVEL >=1)
	#define FATAL(x)												\
		do {													\
		logFile<<"[FATAL] "<<__FILE__<<" ("<<__LINE__<<")  " <<__DATE__<<__TIME__<<endl;			\
		logFile<<x<<endl;											\
		logFile<<"---------------------------------------------------------------------------"<<endl;		\
		}while(0)
#else
	#define FATAL(x)
#endif 

#if (LOG_LEVEL >=2)
	#define ERROR(x)												\
		logFile<<"[ERROR] "<<__FILE__<<" ("<<__LINE__<<")  " <<__DATE__<<__TIME__<<endl;			\
		logFile<<x<<endl;											\
		logFile<<"---------------------------------------------------------------------------"<<endl;
#else
	#define ERROR(x)
#endif 

#if (LOG_LEVEL >=3)
	#define WARN(x)												\
		logFile<<"[WARN] "<<__FILE__<<" ("<<__LINE__<<")  " <<__DATE__<<__TIME__<<endl;			\
		logFile<<x<<endl;											\
		logFile<<"---------------------------------------------------------------------------"<<endl;
#else
	#define WARN(x)
#endif 

#if (LOG_LEVEL >=4)
	#define DEBUG(x)												\
		do {													\
		logFile<<"[DEBUG] "<<__FILE__<<" ("<<__LINE__<<")  " <<__DATE__<<__TIME__<<endl;			\
		logFile<<x<<endl;											\
		logFile<<"---------------------------------------------------------------------------"<<endl;		\
		}while(0)
#else
	#define DEBUG(x)
#endif

#if (LOG_LEVEL >=5)
	#define BEGIN()												\
		logFile<<"[BEGIN] "<<__FILE__<<" ("<<__LINE__<<")  " <<__DATE__<<__TIME__<<endl;				\
		logFile<<__PRETTY_FUNCTION__<<endl;									\
		logFile<<"---------------------------------------------------------------------------"<<endl;
	
	#define END()												\
		logFile<<"[END] "<<__FILE__<<" ("<<__LINE__<<")  " <<__DATE__<<__TIME__<<endl;				\
		logFile<<__PRETTY_FUNCTION__<<endl;									\
		logFile<<"---------------------------------------------------------------------------"<<endl;
#else
	#define BEGIN()
	#define END()
#endif

	
