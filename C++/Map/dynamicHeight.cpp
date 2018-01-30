#include<iostream>
#include"Map.hpp"

using std::cout;
using std::endl;
using cs540::Map;

int main()
{
    Map<int,int>mymap;

    cout<< "Initial height is:" << mymap.height()<<endl;
    
    for(int i=0;i<100;i++)
        mymap.insert(std::pair<int,int>(i,i+1));

     cout<< "Current height is:" << mymap.height()<<endl;

    for(int i=100;i<600;i++)
        mymap.insert(std::pair<int,int>(i,i+1));

    cout<< "Height now is:" << mymap.height()<<endl;

    for(int i=1;i<590;i++)
    {
        mymap.erase(mymap.end()--);
    }
    
    cout<< "Height now after deleting few elements:" << mymap.height()<<endl;
    
    return 0;
}
