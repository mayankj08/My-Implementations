#include<typeinfo>
#include <type_traits>
#include<cassert>
#include<utility>
#include<string>
#include<iostream>
#include<algorithm>
#include<tuple>
#include<vector>
#include<sstream>
#include <exception>
#include<iomanip>

using std::vector;
using std::cout;
using std::string;
using std::tuple;
using std::stringstream;

#define DEBUG_MODE 0
		
typedef std::ostream & (&omanip_t)( std::ostream & );
#define ffr static_cast<omanip_t>

namespace cs540{
	class WrongNumberOfArgs: public std::exception
	{
  		virtual const char* what()  const throw()
  		{
    			return "Wrong number of Args.";
  		}
	};

	template<typename Arg_Tuple> struct Interpolate_Helper
	{
		std::string fmtstr="";
		Arg_Tuple argTuple;
		vector<std::string>tokens;
		bool isLastCharDelim;

		Interpolate_Helper(string fmt,
				  const Arg_Tuple &args,
				  vector<std::string> tokens,
				  bool isLastCharDelim):
					fmtstr(fmt),
					argTuple(args),
					tokens(tokens),
					isLastCharDelim(isLastCharDelim)
		{
			
		}
	};


	vector<string> split(const string &s, char delim,bool &isLastCharDelim) 
	{
		stringstream ss(s);
		string item;
		vector<string> tokens;
		while (getline(ss, item, delim)) 
		{
			tokens.push_back(item);
			if(ss.eof())
				isLastCharDelim =false;
			else
				isLastCharDelim=true;
		}
		return tokens;
	}


	// This function would bw called when there is no argument except 
	// format string
	template<typename Arg_Tuple>
	std::ostream& outputHelper2(std::ostream& os, Interpolate_Helper<Arg_Tuple> obj,size_t currIndex)
	{
		if(currIndex == 0) // This means that no argument is present
		{
			size_t counter = 0;
			auto size = obj.tokens.size();
			
			for(auto elem:	obj.tokens)
			{
				//cout<<"Inside"<<"\n"<<obj.isLastCharDelim<<"\n";;
				std::string elemCopy(elem);
				counter++;
				auto &last = elemCopy.back();
				if(last == '\\' && ((counter!=size) || (counter == size && obj.isLastCharDelim==true)))
                        	{
					//cout<<"HERE!!!!!!!!!!!!!!!!!!!"<<"\n";
                                	last = '%';
                        	}
				os<<elemCopy;
			}
		}
		else
		{
		
			size_t counter = currIndex;
			auto size = obj.tokens.size();
			auto iter = obj.tokens.begin() + currIndex;
			for(;iter!=obj.tokens.end();iter++)
			{
				auto elem = *iter;	
				counter++;
				auto &last = elem.back();
				if(last == '\\'  && ((counter!=size) || (counter == size && obj.isLastCharDelim==true)))
                        	{
					//cout<<"HERE!!!!!!"<<"\n";
                                	last = '%';
                        	}
				else if(last!= '\\' && ((counter!=size) || (counter == size && obj.isLastCharDelim==true)))
				{
					
					throw WrongNumberOfArgs();
					//cout<<"Wrong number of arguments..Less";
				}
				os<<elem;
			}
		}
	
		return os;
	}

	int checkType(std::ios_base &(*)(std::ios_base &))
	{
		return 1; // Represents Maipulator with no 
	}


	// For flush
	int checkType(std::ostream&(*f)(std::ostream&))
	{
		if(f == (std::basic_ostream<char>& (*)(std::basic_ostream<char>&)) &std::flush)
		{
			return 1;
		}
		else if(f == (std::basic_ostream<char>& (*)(std::basic_ostream<char>&)) &std::ends)
		{
			return 0;
		}
		else if(f == (std::basic_ostream<char>& (*)(std::basic_ostream<char>&)) &std::endl)
		{
			return 0;
		}
		return 1;
	}

	
	template<typename T>
	int checkType(T&)
	{
		if(typeid(T) == typeid(decltype(std::setbase(1))))
		{
			return 1;
		}
		else if(typeid(T) == typeid(decltype(std::resetiosflags(std::ios_base::basefield))))
		{
			return 1;
		}
		else if(typeid(T) == typeid(decltype(std::setiosflags(std::ios_base::hex))))
		{
			return 1;
		}
		else if(typeid(T) == typeid(decltype(std::setw(13))))
		{
			return 1;
		}
		else if(typeid(T) == typeid(decltype(std::setprecision(10))))
		{
			return 1;
		}
		else if(typeid(T) == typeid(decltype(std::setfill('-'))))
		{
			return 1;
		}
		else if(typeid(T) == typeid(decltype(std::put_money(112, true))))
		{
			return 0;
		}


		return 0; //Implies not a manipulator
	}


	template<std::size_t index, std::size_t... indices,typename Arg_Tuple>
	std::ostream& outputHelper2(std::ostream& os, Interpolate_Helper<Arg_Tuple> obj,size_t currIndex)
	{
		size_t localIndex= currIndex;
		//cout<<"Current Index:"<<index<<"\n";

		while(1)
		{
			auto& curr = std::get<index>(obj.argTuple);	
			size_t sizeOfTokens = obj.tokens.size();
			if(checkType(curr) == 1)
			{
				//cout<<"Return type=1"<<"\n";
				os<<curr;
				break;
			}

			if(localIndex > sizeOfTokens-1)
			{
				if(1)
				{
					throw WrongNumberOfArgs();
				}
			}
			else
			{
				std::string currToken = obj.tokens[localIndex];
				auto& last = currToken.back();
				if(last == '\\')
				{
					localIndex ++;
					last = '%';
					os<<currToken;
				}
				else
				{
					int ret = checkType(curr);
					//cout<<"Return type="<<ret<<"\n";

					if(ret == 1)
					{
						//cout<<"Applying "<<index<<"\n";
						os<<curr;
					}
					else
					{
						//cout<<"Applying token:"<<index<<"\n";
						//cout<<"current token is:"<<currToken<<".\n";
						if(currToken == "")
						{
							os<<curr;
						}
						else
						{
							os<<currToken;
							os<<curr;
						}
						localIndex++;
					}
					break;
				}
			}
		}	
			outputHelper2<indices...>(os,obj,localIndex);	

		return os;
		//%std::cout<<"Mayank"<<std::get<index>(obj.argTuple);
		//outputHelper2<indices...,Args...>(os,obj);
	}


	template<std::size_t...indices,typename Arg_Tuple> 
	std::ostream& outputHelper(std::ostream& os, Interpolate_Helper<Arg_Tuple> obj, std::index_sequence<indices...> )
	{
		int lastindex = 0;
		//cout<<"Input String="<<obj.fmtstr<<"\n";
		//std::cout<<std::get<0>(obj.argTuple);	
		outputHelper2<indices...>(os,obj,lastindex);
		return os;
	}

	template<typename Arg_Tuple>  std::ostream& 
	operator<< ( std::ostream& os,Interpolate_Helper<Arg_Tuple> obj)
	{
		//cout<<"New String\n"<<std::endl;
		//outputHelper(os,obj,std::make_index_sequence<std::tuple_size<decltype(obj.argTuple)>::value>());
		constexpr auto Size = std::tuple_size<typename std::decay<decltype(obj.argTuple)>::type>::value;

		//std::cout<<std::get<0>(obj.argTuple)<<":Mayank";
		outputHelper(os,obj,std::make_index_sequence<Size>());
		return os;
		
	}

	

	template<typename...Args>
	auto Interpolate(const char* fmt,const Args&... args)
	{
		std::string fmtStr(fmt);     
		bool isLastCharDelim = false;
		auto vec = split(fmtStr,'%',isLastCharDelim);
		auto args_tuple = std::tie(args...);
		return Interpolate_Helper<decltype(args_tuple)>(fmtStr,args_tuple,vec,isLastCharDelim);
	}
}
