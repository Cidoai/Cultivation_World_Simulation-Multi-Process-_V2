#include "SimulationWorld_V2.h"

void SimulationWorld::form_alliances() {
    // 清空现有联盟
    alliances.clear();
    cultivator_alliance.clear();
    next_alliance_id = 0;
    
    // 收集所有活着的修士
    std::vector<std::shared_ptr<Cultivator>> all_cultivators;
    for (auto& level_pop : population_by_level) {
        for (auto& c : level_pop.second) {
            if (c->is_alive) {
                all_cultivators.push_back(c);
            }
        }
    }
    
    int N = all_cultivators.size();
    if (N < 2) return;
    
    // 计算最大联盟数 = sqrt N
    int max_alliances = static_cast<int>(std::sqrt(N));
    max_alliances = std::min(500, max_alliances);
    
    // 随机打乱顺序
    std::shuffle(all_cultivators.begin(), all_cultivators.end(), Random::generator);
    
    // 直接分组：将修士分成 max_alliances 组
    int members_per_alliance = N / max_alliances;
    int remaining = N % max_alliances;
    
    int index = 0;
    for (int i = 0; i < max_alliances; ++i) {
        // 计算当前联盟的人数
        int size = members_per_alliance + (i < remaining ? 1 : 0);
        if (size < 2) continue;  // 人数太少不成盟
        
        // 创建新联盟
        auto alliance = std::make_shared<Alliance>(next_alliance_id++, current_year);
        
        // 加入成员
        for (int j = 0; j < size; ++j) {
            auto& member = all_cultivators[index + j];
            alliance->add_member(member->id, member->get_combat_power());
            member->alliance_id = alliance->id;
            cultivator_alliance[member->id] = alliance->id;
        }
        
        alliances[alliance->id] = alliance;
        index += size;
    }
    // 构建勇气值映射表
	std::map<int, double> courage_map;
	for (auto& level_pop : population_by_level) {
	    for (auto& c : level_pop.second) {
	        if (c->is_alive) {
	            courage_map[c->id] = c->courage;
	        }
	    }
	}
	
	// 更新所有联盟的平均勇气
	for (auto& pair : alliances) {
	    pair.second->update_avg_courage(courage_map);
	}
}
