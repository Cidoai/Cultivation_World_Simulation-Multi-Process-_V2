#include "Generator_Random_V2.h"

#ifndef CULTIVATOR 
#define CULTIVATOR

// 修士类
class Cultivator {
public:
    int id;
    int level;
    double power;           // 修为
    double base_power;      // 基础修为（用于计算等级）
    double courage;         // 勇气值 0.1-0.9
    double base_courage;    // 基础勇气值
    int age;
    bool is_alive;
    double power_history[10]; // 最近10场战斗的贡献
    int fight_count;
    int wins;
    int losses;
    double position_x;
    double position_y;
    int alliance_id;        // -1表示无联盟
    double max_lifespan;           // 理论最大寿命
	double remaining_lifespan;     // 剩余寿命
	int fights_this_year;          // 当年战斗次数
	int wins_this_year;            // 当年胜利次数
	int killed_this_year;          // 当年杀人数量
	double injury_level;      // 受伤程度 0-1，0为无伤，1为濒死
    int recovery_time;        // 恢复所需年数
    bool is_injured;          // 是否受伤
    
    Cultivator(int id, int start_level = 1) 
        : id(id)
        , level(start_level)
        , base_power(100.0 * start_level)
        , power(100.0 * start_level)
        , base_courage(Random::uniform(0.1, 0.9))
        , courage(base_courage)
        , age(0)
        , is_alive(true)
        , fight_count(0)
        , wins(0)
        , losses(0)
        , position_x(Random::uniform(0, 10000.0))
        , position_y(Random::uniform(0, 10000.0))
        , alliance_id(-1)
    {
        for (int i = 0; i < 10; ++i) {
            power_history[i] = 0;
        }
		// 寿元初始化
		max_lifespan = 0;
		remaining_lifespan = 0;
		fights_this_year = 0;
		wins_this_year = 0;
		injury_level = 0;
		killed_this_year = 0;
        recovery_time = 0;
        is_injured = false;
		
		// 根据等级初始化寿元
		max_lifespan = 100 + (level - 1) * (level - 1) * 50;
		
		// 初始剩余寿元等于最大寿元
		remaining_lifespan = max_lifespan;
		
		// 勇气值影响初始寿元（性格决定命运）
		if (courage < 0.3) {
		    remaining_lifespan *= 0.95;  // 怂人郁结于心
		} else if (courage > 0.7) {
		    remaining_lifespan *= 0.9;   // 勇人容易意外
		}
    }
    
    // 获取等级阈值
    static double get_level_threshold(int level) {
    	double prod = 100.0 * level * level;
    	prod *= 1<<(level/3);
        return prod;
    }
    
    // 修炼
    void train() {
        if (!is_alive) return;
        
        // 基础修炼收益
        double base_gain = 10.0 * level;
        
        // 勇气影响修炼效率
        double mindset_bonus = 1.0;
        if (courage >= 0.4 && courage <= 0.6) {
            mindset_bonus = 1.2;  // 平和状态最佳
        } else if (courage < 0.2 || courage > 0.8) {
            mindset_bonus = 0.7;  // 极端状态最差
        }
        
        double gain = base_gain * mindset_bonus;
        power += gain;
        age++;
    }
    
    // 检查是否可以晋升
    bool can_advance() {
        return power >= get_level_threshold(level + 1);
    }
    
    // 根据等级重新计算最大寿元（突破时调用）
	void recalc_max_lifespan() {
		max_lifespan = 100 + (level - 1) * (level - 1) * 100;
	}
	
	// 新增：受伤处理函数
    void take_damage(double damage) {
        injury_level = std::min(1.0, injury_level + damage);
        is_injured = true;
        recovery_time = static_cast<int>(injury_level * 5) + 1;
    }

    // 新增：恢复函数
    void recover() {
        if (recovery_time > 0) {
            recovery_time--;
            if (recovery_time == 0) {
                injury_level = 0;
                is_injured = false;
            }
        }
    }
	
	// 年度寿元更新
	void update_lifespan() {
	    if (!is_alive) return;
	    
	    // 基础消耗1年
	    double consumption = 1.0;
	    
	    // 战斗消耗：每次战斗消耗0.1年
	    consumption += fights_this_year * 0.1;
	    
	    // 受伤消耗：受伤程度越重消耗越大
	    consumption += injury_level * 0.5;
	    
	    // 勇气值调整：心态平和的人消耗更少
	    if (courage >= 0.4 && courage <= 0.6) {
	        consumption *= 0.9;  // 心态好，活得久
	    } else if (courage < 0.2 || courage > 0.8) {
	        consumption *= 1.2;  // 极端心态，消耗大
	    }
	    
	    // 杀人延寿：每杀一人增加3年
	    if (killed_this_year > 0) {
	        remaining_lifespan += killed_this_year * 3;
	    }
	    
	    // 消耗寿元
	    remaining_lifespan -= consumption;
	    
	    // 如果寿元耗尽，标记死亡
	    if (remaining_lifespan <= 0) {
	        is_alive = false;
	    }
	}
	
    // 新增：获取战斗效能（受伤影响）
    double get_combat_effectiveness() {
        if (!is_injured) return 1.0;
        return 1.0 - injury_level * 0.7;
    }
	
	// 1. 等级决定基础值
    double get_base_power_for_level() {
        return 100 * level;
    }
    
    // 2. 等级决定修炼速度
    double get_train_rate() {
        return 5 * level;
    }
    
    // 3. 等级决定战力系数
    double get_level_bonus() {
        return 1.0 + level * 0.2;  // 每级+20%
    }
    
    // 4. 总战力
    double get_combat_power() {
        return power * get_level_bonus() * get_combat_effectiveness();
    }
    
	void advance() {
	    level++;
	    power = get_level_threshold(level);  // 重置到新等级的基础值
	    if (courage < 0.1) courage = 0.1;
	    
	    // 新增：突破时重新计算最大寿元并刷新剩余寿元
	    recalc_max_lifespan();
	    remaining_lifespan = max_lifespan;  // 突破时满血复活
	}
    
    // 更新勇气值
    void update_courage(double global_avg_power, double local_density) {
        if (!is_alive) return;
        
        double new_courage = base_courage;
        
        // 修为相对水平
        double power_ratio = power / global_avg_power;
        if (power_ratio > 2.0) {
            new_courage += 0.1;
        } else if (power_ratio < 0.5) {
            new_courage -= 0.1;
        }
        
        // 周围密度影响
        if (local_density > 100) {
            new_courage += 0.05;
        }
        
        // 最近胜负影响
        if (fight_count > 0) {
            double win_rate = static_cast<double>(wins) / fight_count;
            new_courage += (win_rate - 0.5) * 0.2;
        }
        
        // 限制范围
        courage = std::max(0.1, std::min(0.9, new_courage));
    }
};

#endif
