#include "SimulationWorld_V2.h"

// 运行一年的模拟
void SimulationWorld::run_year(int now_year) {
    std::cout << "Running year " << current_year << "..." << std::endl;
    
    auto start_time = std::chrono::high_resolution_clock::now();
    // 1. 新人涌入
    add_newbies(3000);
    
    // 2. 所有人修炼
	parallel_for_cultivators([](std::shared_ptr<Cultivator> c) {
	    c->train();
	});
    
    // 3. 更新勇气值
    update_all_courage();
    
    // 4. 形成联盟
    if(now_year%50==0)
    	form_alliances();
    
    // 4.5 寿元更新阶段（新增）
	parallel_for_cultivators([](std::shared_ptr<Cultivator> c) {
	    c->update_lifespan();
	});
	auto end_time = std::chrono::high_resolution_clock::now();;
	auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
	std::cout << "\nThis year's before combat completed in " << duration.count() << " miliseconds." << std::endl;
    
	start_time = std::chrono::high_resolution_clock::now();
    // 5. 战斗阶段
    int fights = 0;
    int kills = combat_phase(fights);
    
    // 6. 寿元和晋升
    std::pair<int, int> result = lifespan_and_promotion();
	int promotions = result.first;
	int natural_deaths = result.second;

    // 7. 记录统计
    record_stats(kills, natural_deaths, promotions, fights);
    
    current_year++;
    
    // 8. 重置年度计数器（新增）
	for (auto& level_pop : population_by_level) {
	    for (auto& c : level_pop.second) {
	        if (c->is_alive) {
	            c->fights_this_year = 0;
	            c->wins_this_year = 0;
	            c->killed_this_year = 0;
	        }
	    }
	}
	end_time = std::chrono::high_resolution_clock::now();;
	duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
	std::cout << "\nThis year's in and after combat completed in " << duration.count() << " miliseconds." << std::endl;
}

// 运行多年模拟
void SimulationWorld::run_years(int years) {
    for (int i = 0; i < years; ++i) {
        run_year(i);
        print_current_status();
    }
}

// 打印当前状态
void SimulationWorld::print_current_status() {
    if (history.empty()) return;
    
    const auto& stats = history.back();
    
    std::cout << "\n=== Year " << stats.year << " Status ===" << std::endl;
    std::cout << "Total Population: " << stats.total_population << std::endl;
    std::cout << "Alliances: " << stats.alliance_count << std::endl;
	std::cout << "Fights this year: " << stats.fight_count << std::endl;
    std::cout << "Kills this year: " << stats.kill_count << std::endl;
    std::cout << "Natural Deaths: " << stats.natural_death_count << std::endl;
    std::cout << "Promotions: " << stats.promotion_count << std::endl;
    std::cout << "Gini Coefficient: " << std::fixed << std::setprecision(3) 
              << stats.gini_coefficient << std::endl;
    
    std::cout << "\nPopulation by Level:" << std::endl;
    for (int level = 1; level <= 10; ++level) {
        std::cout << "  Level " << level << ": " 
                  << stats.level_counts.at(level) << " cultivators, "
                  << "Avg Courage: " << std::fixed << std::setprecision(3)
                  << stats.avg_courage_by_level.at(level)
                  << std::endl;
    }
    std::cout << std::endl;
}

// 导出统计数据到CSV
void SimulationWorld::export_stats(const std::string& filename) {
    std::ofstream file(filename);
    
    // 写入标题行
    file << "Year,Total,Alliances,Fights,Kills,NaturalDeaths,Promotions,Gini";
    for (int level = 1; level <= 10; ++level) {
        file << ",L" << level << "_Count,L" << level << "_Courage";
    }
    file << "\n";
    
    // 写入数据
    for (const auto& stats : history) {
		file << stats.year << ","
		     << stats.total_population << ","
		     << stats.alliance_count << ","
		     << stats.fight_count << ","  // 新增
		     << stats.kill_count << ","
		     << stats.natural_death_count << ","
		     << stats.promotion_count << ","
		     << std::fixed << std::setprecision(4) << stats.gini_coefficient;
        
        for (int level = 1; level <= 10; ++level) {
            file << "," << stats.level_counts.at(level)
                 << "," << std::fixed << std::setprecision(4) 
                 << stats.avg_courage_by_level.at(level);
        }
        file << "\n";
    }
    
    file.close();
    std::cout << "Statistics exported to " << filename << std::endl;
}
