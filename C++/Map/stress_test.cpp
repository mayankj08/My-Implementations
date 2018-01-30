//Stress testing teast case

#include "Map.hpp"
#include <iostream>
#include <string>
#include <chrono>
#include <iterator>
#include <cassert>
#include <utility>
#include <random>

int main () {
    auto seed = std::chrono::system_clock::now().time_since_epoch().count();
    std::default_random_engine gen(seed);
    std::uniform_int_distribution<unsigned int> dist(0, 10000);
    
    cs540::Map<int, int> m;

    using TimePoint = std::chrono::time_point<std::chrono::system_clock>;
    using Milli = std::chrono::duration<double, std::ratio<1,1000>>;
    using namespace std::chrono;

    TimePoint start, end;
    start = system_clock::now();
    
    for(int i = 0; i < 1000000; ++i) {
        auto g = dist(gen);
        m.insert({g, g});
    }
    
    int num_erases = gen() % m.size();
    for(int i = 0; i < num_erases; ++i) {
        //select a random element
        int choice = gen() % m.size();
        auto iter = std::begin(m);
        for (int j = 0; j < choice; ++j) {
            ++iter;
        }
        
        m.erase(iter);
    }

    end = system_clock::now();
    Milli elapsed = end - start;
    assert(elapsed.count() < 2500);

    return 0;
}
