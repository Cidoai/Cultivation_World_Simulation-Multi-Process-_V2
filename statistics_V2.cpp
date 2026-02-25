#include "SimulationWorld_V2.h"

// 获取全局平均修为
double SimulationWorld::get_global_avg_power() {
    double total_power = 0;
    int count = 0;
    for (const auto& level_pop : population_by_level) {
        for (const auto& c : level_pop.second) {
            if (c->is_alive) {
                total_power += c->get_combat_power();
                count++;
            }
        }
    }
    return count > 0 ? total_power / count : 100.0;
}

// 计算基尼系数
double SimulationWorld::calculate_gini() {
    std::vector<double> powers;
    for (const auto& level_pop : population_by_level) {
        for (const auto& c : level_pop.second) {
            if (c->is_alive) {
                powers.push_back(c->power);
            }
        }
    }
    
    if (powers.empty()) return 0.0;
    
    std::sort(powers.begin(), powers.end());
    
    double sum = 0;
    double total = 0;
    int n = powers.size();
    
    for (int i = 0; i < n; ++i) {
        sum += powers[i] * (i + 1);
        total += powers[i];
    }
    
    double gini = (2.0 * sum) / (n * total) - (n + 1.0) / n;
    return gini;
}

// 记录统计数据
void SimulationWorld::record_stats(int kills, int natural_deaths, int promotions, int fights) {
    Statistics stats;
    stats.year = current_year;
    stats.total_population = 0;
    stats.alliance_count = alliances.size();
    stats.kill_count = kills;
    stats.natural_death_count = natural_deaths;
    stats.promotion_count = promotions;
	stats.fight_count = fights;
    
    // 统计各等级人数和平均勇气
    for (int level = 1; level <= 10; ++level) {
        int count = 0;
        double total_courage = 0;
        double total_power = 0;
        
        for (const auto& c : population_by_level[level]) {
            if (c->is_alive) {
                count++;
                total_courage += c->courage;
                total_power += c->power;
            }
        }
        
        stats.level_counts[level] = count;
        stats.total_population += count;
        
        if (count > 0) {
            stats.avg_courage_by_level[level] = total_courage / count;
            stats.avg_power_by_level[level] = total_power / count;
        } else {
            stats.avg_courage_by_level[level] = 0;
            stats.avg_power_by_level[level] = 0;
        }
    }
    
    stats.gini_coefficient = calculate_gini();
    
    history.push_back(stats);
}
