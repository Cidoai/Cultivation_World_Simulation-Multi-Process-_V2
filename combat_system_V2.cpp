#include "SimulationWorld_V2.h"

// 战斗阶段
int SimulationWorld::combat_phase(int& fights) {
    int total_kills = 0;
    
    // 获取所有活跃联盟
    std::vector<std::shared_ptr<Alliance>> active_alliances;
    for (auto& pair : alliances) {
        if (pair.second->is_active && !pair.second->member_ids.empty()) {
            active_alliances.push_back(pair.second);
        }
    }
    
    if (active_alliances.size() < 5) return 0;
    
    // 随机打乱
    std::shuffle(active_alliances.begin(), active_alliances.end(), Random::generator);
    
    // 限制每年最多 30% 的联盟参战
    int max_fights = std::max(1, (int)(active_alliances.size() * 0.3));
    fights = 0;
    
    // 只遍历前 50% 的联盟作为攻击者（随机抽样）
    int attackers_to_check = std::max(1, (int)(active_alliances.size() * 0.5));
    
    for (int i = 0; i < attackers_to_check && fights < max_fights; ++i) {
        auto& attacker = active_alliances[i];
        
        // 勇气值影响攻击欲望（核心修改）
        double attack_desire = attacker->avg_courage;  // 0.1-0.9
        
        // 30% 基础概率 + 勇气值影响
        double base_chance = 0.3;
        double final_chance = base_chance * (0.5 + attack_desire);  // 勇气越高越爱打
        
        if (!Random::bernoulli(final_chance)) continue;
        
        // 随机选择一个不同的目标
        int target_idx;
        do {
            target_idx = Random::uniform_int(0, active_alliances.size() - 1);
        } while (target_idx == i);
        
        auto& target = active_alliances[target_idx];
        
        // 实力对比
        double perceived = target->combat_power * Random::uniform(0.7, 1.3);
        double power_ratio = attacker->combat_power / perceived;
        
        // 勇气值影响风险评估（核心修改）
        // 勇气高的人更容易低估风险（power_ratio 要求更低）
        double required_ratio = 1.2 - (attacker->avg_courage * 0.4);  // 勇气0.9时需0.93，勇气0.1时需1.17
        
        if (power_ratio < required_ratio) continue;
        
        // 战斗概率（勇气值再次影响）
        double fight_prob = std::min(1.0, power_ratio * attacker->avg_courage);
        
        if (Random::bernoulli(fight_prob)) {
            fights++;
            
            // 简单战斗结算
            double attacker_roll = attacker->combat_power * Random::uniform(0.9, 1.1) * 1.1;
            double target_roll = target->combat_power * Random::uniform(0.9, 1.1);
            
            if (attacker_roll > target_roll) {
			    // 攻击方胜利
			    int target_casualties = 0;
			    
			    // 处理败方（原目标）
			    for (int member_id : target->member_ids) {
			        auto c = fast_find_cultivator(member_id);
			        if (!c || !c->is_alive) continue;
			        
			        // 根据等级计算死亡概率（败方基础死亡率）
			        double death_base = 0.15;  // 败方基础死亡率
			        double death_prob = death_base / (1.0 + c->level * 0.6);  // 等级越高越难死
			        
			        double roll = Random::uniform();
			        if (roll < death_prob) {  // 死亡
			            c->is_alive = false;
			            target_casualties++;
			            total_kills++;
			        } else if (roll < death_prob + 0.35) {  // 35%重伤
			            double injury = 0.6 + 0.3 * Random::uniform();  // 0.6-0.9
			            c->take_damage(injury);
			            c->courage = std::max(0.1, c->courage - 0.05);  // 失败大幅降勇气
			        } else if (roll < death_prob + 0.65) {  // 30%轻伤
			            double injury = 0.2 + 0.3 * Random::uniform();  // 0.2-0.5
			            c->take_damage(injury);
			            c->courage = std::max(0.1, c->courage - 0.02);  // 失败微降勇气
			        } else {  // 剩余概率无伤
			            c->courage = std::max(0.1, c->courage - 0.01);  // 虽无伤但士气受挫
			        }
			    }
			    
			    // 处理胜方（原攻击者）
			    for (int member_id : attacker->member_ids) {
			        auto c = fast_find_cultivator(member_id);
			        if (!c || !c->is_alive) continue;
			        
			        // 胜方伤亡概率较低
			        if (Random::bernoulli(0.15)) {  // 15%概率受伤
			            double injury = 0.1 + 0.2 * Random::uniform();  // 0.1-0.3轻伤
			            c->take_damage(injury);
			        }
			        
			        // 胜利微增勇气
			        c->courage = std::min(0.95, c->courage + 0.01);
			        c->wins_this_year++;
			    }
			    
			    // 分配修为（从败方掠夺）
			    double total_spoils = target->combat_power * 0.3;  // 掠夺30%修为
			    double per_member = total_spoils / attacker->member_ids.size();
			    
			    for (int member_id : attacker->member_ids) {
			        auto c = fast_find_cultivator(member_id);
			        if (c && c->is_alive) {
			            c->power += per_member;
			        }
			    }
            } else {
			    // 攻击方失败
			    int attacker_casualties = 0;
			    
			    for (int member_id : attacker->member_ids) {
			        auto c = fast_find_cultivator(member_id);
			        if (!c || !c->is_alive) continue;
			        
			        // 根据等级计算死亡概率（败方死亡率略高）
			        double death_base = 0.15;  // 败方基础死亡率略高
			        double death_prob = death_base / (1.0 + c->level * 0.6);  // 等级越高越难死
			        
			        double roll = Random::uniform();
			        if (roll < death_prob) {  // 死亡
			            c->is_alive = false;
			            attacker_casualties++;
			            total_kills++;
			        } else if (roll < death_prob + 0.35) {  // 35%重伤
			            double injury = 0.6 + 0.3 * Random::uniform();  // 0.6-0.9
			            c->take_damage(injury);
			            c->courage = std::max(0.1, c->courage - 0.05);  // 失败大幅降勇气
			        } else if (roll < death_prob + 0.65) {  // 30%轻伤
			            double injury = 0.2 + 0.3 * Random::uniform();  // 0.2-0.5
			            c->take_damage(injury);
			            c->courage = std::max(0.1, c->courage - 0.02);  // 失败微降勇气
			        } else {  // 剩余概率无伤
			            c->courage = std::max(0.1, c->courage - 0.01);  // 虽无伤但士气受挫
			        }
			    }
			    
			    // 胜利方（原目标）也有少量伤亡
			    for (int member_id : target->member_ids) {
			        auto c = fast_find_cultivator(member_id);
			        if (!c || !c->is_alive) continue;
			        
			        // 胜利方伤亡概率较低
			        if (Random::bernoulli(0.15)) {  // 15%概率受伤
			            double injury = 0.1 + 0.2 * Random::uniform();  // 0.1-0.3轻伤
			            c->take_damage(injury);
			        }
			        
			        // 胜利微增勇气
			        c->courage = std::min(0.95, c->courage + 0.01);
			        c->wins_this_year++;
			    }
			    
			    // 胜利方获得部分修为（从败方受伤者身上掠夺）
			    double total_spoils = attacker->combat_power * 0.3;  // 败方贡献30%修为
			    double per_member = total_spoils / target->member_ids.size();
			    
			    for (int member_id : target->member_ids) {
			        auto c = fast_find_cultivator(member_id);
			        if (c && c->is_alive) {
			            c->power += per_member;
			        }
			    }
            }
        }
    }
    
    return total_kills;
}

// 更新所有人的勇气值
void SimulationWorld::update_all_courage() {
    // 1. 重建密度网格（每年一次）
    std::map<std::pair<int, int>, int> grid_count;
    const double CELL_SIZE = 100.0;
    
    // 统计每个格子的人数
    for (auto& level_pop : population_by_level) {
        for (auto& c : level_pop.second) {
            if (c->is_alive) {
                int gx = static_cast<int>(c->position_x / CELL_SIZE);
                int gy = static_cast<int>(c->position_y / CELL_SIZE);
                grid_count[{gx, gy}]++;
            }
        }
    }
    
    // 2. 计算每个格子的密度缓存（考虑相邻格子）
    std::map<std::pair<int, int>, double> density_cache;
    const int NEIGHBOR_RADIUS = 2;
    
    for (auto& cell_pair : grid_count) {
        auto& cell = cell_pair.first;
        double total = 0;
        
        for (int dx = -NEIGHBOR_RADIUS; dx <= NEIGHBOR_RADIUS; ++dx) {
            for (int dy = -NEIGHBOR_RADIUS; dy <= NEIGHBOR_RADIUS; ++dy) {
                auto neighbor = std::make_pair(cell.first + dx, cell.second + dy);
                auto it = grid_count.find(neighbor);
                if (it != grid_count.end()) {
                    double distance = std::sqrt(dx*dx + dy*dy);
                    double weight = 1.0 / (1.0 + distance);
                    total += it->second * weight;
                }
            }
        }
        
        density_cache[cell] = total;
    }
    
    // 3. 获取全局平均修为
    double global_avg_power = get_global_avg_power();
    
    // 4. 并行更新勇气值（使用缓存的密度）
    parallel_for_cultivators([this, &density_cache, global_avg_power, CELL_SIZE]
                             (std::shared_ptr<Cultivator> c) {
        int gx = static_cast<int>(c->position_x / CELL_SIZE);
        int gy = static_cast<int>(c->position_y / CELL_SIZE);
        
        double local_density = 0;
        auto it = density_cache.find({gx, gy});
        if (it != density_cache.end()) {
            local_density = it->second;
        }
        
        c->update_courage(global_avg_power, local_density);
    });
}
