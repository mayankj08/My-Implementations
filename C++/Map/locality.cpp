#include<iostream>
#include"Map.hpp"
#include <chrono>

using std::cout;
using std::endl;
using cs540::Map;

int main()
{
    Map<int,int>mymap;

    cout<< "Initial height is:" << mymap.height()<<endl;

    for(int i=0;i<10000000;i++)
        mymap.insert(std::pair<int,int>(i,i+1));
    
    using TimePoint = std::chrono::time_point<std::chrono::system_clock>;
    using Milli = std::chrono::duration<double, std::ratio<1,1000>>;
    using namespace std::chrono;
    
    TimePoint start, end;
    start = system_clock::now();
    
    // Searching 4,000,000 elements in array which are located
    // at consecutive position
    // so this exploits locality
    for(int i=10;i<4000000;i++)
    {
        mymap.find(i);
    }
    end = system_clock::now();
    Milli elapsed = end - start;
    cout<<"Time to search around 10,000,000 element in map of size 4,000,000 when locality was exploited:"<<elapsed.count()<<endl;

    //Searching 4,000,000 elements when the elements are not 
    //located nearby

    TimePoint start1, end1;
    start1 = system_clock::now();
    for(int i=10;i<8000000;i=i+2) 
    {
        mymap.find(i);
    }
    end1 = system_clock::now();
    elapsed = end1 - start1;
    cout<<"Time to search around 10,000,000 element in map of size 4,000,000 when elements are not located nearby:"<<elapsed.count()<<endl;

    return 0;
}
