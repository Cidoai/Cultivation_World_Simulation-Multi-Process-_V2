#ifndef GENERATOR_RANDOM_V2 
#define GENERATOR_RANDOM_V2
#include <random>
#include <chrono>

// 随机数生成器
class Random {
public:
    static std::mt19937 generator;
    static void init() {
        generator.seed(std::chrono::system_clock::now().time_since_epoch().count());
    }
    
    static double uniform(double min = 0.0, double max = 1.0) {
        std::uniform_real_distribution<double> dist(min, max);
        return dist(generator);
    }
    
    static int uniform_int(int min, int max) {
        std::uniform_int_distribution<int> dist(min, max);
        return dist(generator);
    }
    
    static bool bernoulli(double p) {
        return uniform() < p;
    }
};
#endif
