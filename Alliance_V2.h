#include "Generator_Random_V2.h"

#ifndef ALLIANCE 
#define ALLIANCE

// 联盟类
class Alliance {
public:
    int id;
    std::vector<int> member_ids;
    std::map<int, double> member_powers;  // 成员战力缓存
    double total_base_power;
    double combat_power;
    int formation_year;
    bool is_active;
    double avg_courage;  // 联盟平均勇气值
    
	Alliance(int id, int year) 
	    : id(id)
	    , total_base_power(0)
	    , combat_power(0)
	    , formation_year(year)
	    , is_active(true)
	    , avg_courage(0.5)  // 新增：默认值
	{
	}
    
    // 添加成员
	void add_member(int id, double power) {
	    member_ids.push_back(id);
	    member_powers[id] = power;
	    recalc_power();
	}
    
    // 移除成员
    void remove_member(int cultivator_id) {
        auto it = std::find(member_ids.begin(), member_ids.end(), cultivator_id);
        if (it != member_ids.end()) {
            member_ids.erase(it);
            member_powers.erase(cultivator_id);
            recalc_power();
        }
    }
    
    // 更新联盟平均勇气值（需要传入成员ID到勇气值的映射）
	void update_avg_courage(const std::map<int, double>& courage_map) {
	    if (member_ids.empty()) {
	        avg_courage = 0.5;
	        return;
	    }
	    
	    double total = 0;
	    int count = 0;
	    
	    for (int id : member_ids) {
	        auto it = courage_map.find(id);
	        if (it != courage_map.end()) {
	            total += it->second;
	            count++;
	        }
	    }
	    
	    avg_courage = (count > 0) ? (total / count) : 0.5;
	}
    
    // 重新计算战力（非线性叠加）
    void recalc_power() {
        if (member_ids.empty()) {
            total_base_power = 0;
            combat_power = 0;
            is_active = false;
            return;
        }
        
        // 计算基础总和
        total_base_power = 0;
        double max_power = 0;
        for (const auto& p : member_powers) {
            total_base_power += p.second;
            if (p.second > max_power) {
                max_power = p.second;
            }
        }
        
        int member_count = member_ids.size();
        
        // 协调损耗：每多一人，损耗增加10%，最大50%
        double coordination_penalty = 0.1 * (member_count - 1);
        coordination_penalty = std::min(coordination_penalty, 0.5);
        
        // 领袖加成：如果有一个远超他人的领袖，协调更好
        double avg_power = total_base_power / member_count;
        double leadership_bonus = 0.1 * (max_power / avg_power - 1);
        leadership_bonus = std::min(leadership_bonus, 0.3);
        leadership_bonus = std::max(leadership_bonus, 0.0);
        
        combat_power = total_base_power * (1 - coordination_penalty + leadership_bonus);
    }

};

#endif
