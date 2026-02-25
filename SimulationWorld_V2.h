#ifndef SIMULATION_WORLD 
#define SIMULATION_WORLD

// 1. 标准库头文件
#include <map>
#include <list>
#include <vector>
#include <string>
#include <memory>
#include <algorithm>
#include <cmath>
#include <random>
#include <chrono>
#include <fstream>
#include <iomanip>
#include <thread>
#include <iostream>
using namespace std;

// 2. 项目自定义头文件
#include "Cultivator_V2.h"
#include "Alliance_V2.h"
#include "Generator_Random_V2.h"

const double MAP_SIZE=10000.0;
// 世界模拟器
class SimulationWorld {
private:
    int current_year;
    int next_cultivator_id;
    int next_alliance_id;
    double world_size;
    std::map<int, std::shared_ptr<Cultivator>> cultivator_map;  // ID -> 指针
    
    // 所有修士（按等级索引）
    std::map<int, std::list<std::shared_ptr<Cultivator>>> population_by_level;
    
    // 所有联盟
    std::map<int, std::shared_ptr<Alliance>> alliances;
    
    // 修士ID到联盟ID的映射
    std::map<int, int> cultivator_alliance;
    
    // 统计数据
    struct Statistics {
        int year;
        int total_population;
        std::map<int, int> level_counts;
        std::map<int, double> avg_courage_by_level;
        std::map<int, double> avg_power_by_level;
        int alliance_count;
        int kill_count;
        int natural_death_count;
        int promotion_count;
    	int fight_count;
        double gini_coefficient;
    };
    
    std::vector<Statistics> history;
    // 简单的并行辅助函数
	template<typename Func>
	void parallel_for_cultivators(Func func) {
	    // 收集所有活着的修士
	    std::vector<std::shared_ptr<Cultivator>> alive_cultivators;
	    for (auto& level_pop : population_by_level) {
	        for (auto& c : level_pop.second) {
	            if (c->is_alive) {
	                alive_cultivators.push_back(c);
	            }
	        }
	    }
	    
	    if (alive_cultivators.empty()) return;
	    
	    // 获取硬件支持的线程数
	    unsigned int num_threads = std::thread::hardware_concurrency();
	    if (num_threads == 0) num_threads = 2;  // 保底
	    
	    // 每个线程处理的修士数量
	    size_t chunk_size = alive_cultivators.size() / num_threads + 1;
	    std::vector<std::thread> threads;
	    
	    // 创建线程
	    for (unsigned int t = 0; t < num_threads; ++t) {
	        size_t start = t * chunk_size;
	        size_t end = std::min((t + 1) * chunk_size, alive_cultivators.size());
	        
	        if (start >= end) break;
	        
	        threads.emplace_back([&, start, end]() {
	            for (size_t i = start; i < end; ++i) {
	                func(alive_cultivators[i]);
	            }
	        });
	    }
	    
	    // 等待所有线程完成
	    for (auto& thread : threads) {
	        thread.join();
	    }
	}
    
public:
    SimulationWorld(double size = MAP_SIZE) 
        : current_year(0)
        , next_cultivator_id(0)
        , next_alliance_id(0)
        , world_size(size) {
        Random::init();
    }
    
    // 人口管理
    void add_newbies(int count = 10000);
    std::shared_ptr<Cultivator> fast_find_cultivator(int id);
    
    // 统计查询
    double get_global_avg_power();
    double get_avg_power_by_level(int level);
    double calculate_gini();
    int get_current_year() const { return current_year; }  // 可以内联
    
    // 核心模拟循环
    void run_year(int year);
    void run_years(int years);
    
    // 联盟系统
    void form_alliances();
    
    // 战斗系统
    int combat_phase(int& fights);
    void update_all_courage();
    
    // 寿元系统
    std::pair<int, int> lifespan_and_promotion();
    
    // 统计记录
    void record_stats(int kills, int natural_deaths, int promotions, int fights);
    void print_current_status();
    void export_stats(const std::string& filename);
};

#endif
