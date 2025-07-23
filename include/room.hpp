#pragma once

#include "common.hpp"
#include <string>
#include <memory>
#include <unordered_map>

namespace dungeon_merc {

// Forward declarations
class Player;
class Monster;

// Room class (placeholder for now)
class Room {
public:
    Room(RoomId id, const std::string& name, const std::string& description);
    ~Room() = default;

    // Basic properties
    RoomId get_id() const { return id_; }
    const std::string& get_name() const { return name_; }
    const std::string& get_description() const { return description_; }
    
    // Exits
    void add_exit(Direction direction, std::shared_ptr<Room> room);
    std::shared_ptr<Room> get_exit(Direction direction) const;
    
    // Players and monsters
    void add_player(std::shared_ptr<Player> player);
    void remove_player(std::shared_ptr<Player> player);
    const std::vector<std::shared_ptr<Player>>& get_players() const;
    
    void add_monster(std::shared_ptr<Monster> monster);
    void remove_monster(std::shared_ptr<Monster> monster);
    const std::vector<std::shared_ptr<Monster>>& get_monsters() const;

private:
    RoomId id_;
    std::string name_;
    std::string description_;
    
    std::unordered_map<Direction, std::shared_ptr<Room>> exits_;
    std::vector<std::shared_ptr<Player>> players_;
    std::vector<std::shared_ptr<Monster>> monsters_;
};

} // namespace dungeon_merc 