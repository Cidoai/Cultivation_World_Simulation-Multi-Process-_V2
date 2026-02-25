#include <iostream>
#include <vector>
#include <list>
#include <map>
#include <random>
#include <cmath>
#include <algorithm>
#include <fstream>
#include <iomanip>
#include <memory>
#include <chrono>
#include <thread>
#include "Generator_Random_V2.h"
#include "Cultivator_V2.h"
#include "Alliance_V2.h"
#include "SimulationWorld_V2.h"
using namespace std;

// 主函数
int main() {
	freopen("Cultivation_World_Simulation_V2.out","w",stdout);
    std::cout << "Cultivation World Simulation - C++11 Version" << std::endl;
    std::cout << "=============================================" << std::endl;
    
    // 创建世界
    SimulationWorld world(10000.0);
    
    // 初始化第一年的人口
    world.add_newbies(10000);
    
    // 运行模拟
    int simulation_years = 500;
    std::cout << "Running simulation for " << simulation_years << " years..." << std::endl;
    
    auto start_time = std::chrono::steady_clock::now();
    
    world.run_years(simulation_years);
    
    auto end_time = std::chrono::steady_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::seconds>(end_time - start_time);
    
    std::cout << "\nSimulation completed in " << duration.count() << " seconds." << std::endl;
    
    // 导出数据
    world.export_stats("cultivation_stats_V2.csv");
    
    // 输出最终状态
    std::cout << "\n=== Final Status ===" << std::endl;
    world.print_current_status();
    
    return 0;
}
