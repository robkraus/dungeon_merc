#pragma once

#include "common.hpp"
#include <string>
#include <memory>
#include <vector>

namespace dungeon_merc {

// Forward declarations
class Item;
class Room;

// Player class
class Player {
public:
    Player(const std::string& name, CharacterClass character_class);
    ~Player() = default;

    // Basic properties
    const std::string& get_name() const { return name_; }
    CharacterClass get_character_class() const { return character_class_; }
    int get_health() const { return health_; }
    int get_max_health() const { return max_health_; }
    int get_level() const { return level_; }
    int get_experience() const { return experience_; }
    
    // Health management
    void take_damage(int amount);
    void heal(int amount);
    bool is_alive() const;
    
    // Experience and leveling
    void gain_experience(int amount);
    void level_up();
    
    // Position and movement
    void set_current_room(std::shared_ptr<Room> room);
    std::shared_ptr<Room> get_current_room() const;
    
    // Inventory management
    void add_item(std::shared_ptr<Item> item);
    void remove_item(std::shared_ptr<Item> item);
    const std::vector<std::shared_ptr<Item>>& get_inventory() const;
    
    // Game state
    GameState get_game_state() const { return game_state_; }
    void set_game_state(GameState state) { game_state_ = state; }
    
    // Timestamps
    Timestamp get_last_login() const { return last_login_; }
    void update_last_login() { last_login_ = std::chrono::system_clock::now(); }

private:
    std::string name_;
    CharacterClass character_class_;
    int health_;
    int max_health_;
    int level_;
    int experience_;
    int experience_to_next_level_;
    
    std::shared_ptr<Room> current_room_;
    std::vector<std::shared_ptr<Item>> inventory_;
    
    GameState game_state_;
    Timestamp last_login_;
    
    // Helper methods
    void calculate_experience_to_next_level();
};

} // namespace dungeon_merc 