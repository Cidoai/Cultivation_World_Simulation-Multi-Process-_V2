#include "SimulationWorld_V2.h"

// 添加新人
void SimulationWorld::add_newbies(int count) {
    for (int i = 0; i < count; ++i) {
        auto cultivator = std::make_shared<Cultivator>(next_cultivator_id++, 1);
        population_by_level[1].push_back(cultivator);
        cultivator_map[cultivator->id] = cultivator;
    }
}

std::shared_ptr<Cultivator> SimulationWorld::fast_find_cultivator(int id) {
    auto it = cultivator_map.find(id);
    if (it != cultivator_map.end() && it->second->is_alive) {
        return it->second;
    }
    return nullptr;
}

// 寿元消耗和晋升
std::pair<int, int> SimulationWorld::lifespan_and_promotion() {
    int promotions = 0;
    int natural_deaths = 0;
    
    // 收集所有活着的修士
    std::vector<std::shared_ptr<Cultivator>> all_cultivators;
    for (auto& level_pop : population_by_level) {
        for (auto& c : level_pop.second) {
            if (c->is_alive) {
                all_cultivators.push_back(c);
            }
        }
    }
    
    // 一次性处理所有修士
    for (auto& c : all_cultivators) {
        if (!c->is_alive) continue;
        
        // 先进行恢复（如果有受伤）
        if (c->is_injured) {
            c->recover();
        }
        
        // 消耗寿元（每年基础消耗1年）
        c->update_lifespan();
        
        // 检查是否寿元耗尽
        if (c->remaining_lifespan <= 0.0) {
            c->is_alive = false;
            natural_deaths++;
            continue;
        }
        
        // 检查是否可以晋升
        if (c->can_advance() && c->level < 10) {
            c->advance();
            promotions++;
        }
    }
    
    // 重新构建 population_by_level（每年重建一次）
    std::map<int, std::list<std::shared_ptr<Cultivator>>> new_population;
    for (auto& c : all_cultivators) {
        if (c->is_alive) {
            new_population[c->level].push_back(c);
        }
    }
    
    population_by_level = std::move(new_population);
    
    return {promotions, natural_deaths};
}
