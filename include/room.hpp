#pragma once

#include <string>
#include <map>
#include <memory>
#include <vector>
#include "player.hpp"

namespace dungeon_merc {

class Room {
public:
    Room(int id, const std::string& name, const std::string& description);

    // Getters
    int get_id() const { return id_; }
    const std::string& get_name() const { return name_; }
    const std::string& get_description() const { return description_; }

    // Exit management
    void add_exit(Direction dir, int target_room_id);
    bool has_exit(Direction dir) const;
    int get_exit_room_id(Direction dir) const;
    std::string get_exit_description(Direction dir) const;
    std::vector<std::string> get_available_exits() const;

    // Player management
    void add_player(std::shared_ptr<Player> player);
    void remove_player(std::shared_ptr<Player> player);
    const std::vector<std::shared_ptr<Player>>& get_players() const { return players_; }

    // Room display
    std::string get_full_description() const;
    std::string get_exits_list() const;

private:
    int id_;
    std::string name_;
    std::string description_;
    std::map<Direction, int> exits_;  // Direction -> target room ID
    std::vector<std::shared_ptr<Player>> players_;
};

} // namespace dungeon_merc
