#pragma once

#include <memory>
#include <unordered_map>
#include <string>
#include "room.hpp"
#include "player.hpp"

namespace dungeon_merc {

class GameWorld {
public:
    GameWorld();
    ~GameWorld() = default;

    // Room management
    void add_room(std::shared_ptr<Room> room);
    std::shared_ptr<Room> get_room(int room_id) const;
    std::shared_ptr<Room> get_player_room(std::shared_ptr<Player> player) const;

    // Player management
    void add_player(std::shared_ptr<Player> player, int starting_room_id = 1);
    void remove_player(std::shared_ptr<Player> player);
    bool move_player(std::shared_ptr<Player> player, Direction direction);

    // Game commands
    std::string handle_look_command(std::shared_ptr<Player> player);
    std::string handle_move_command(std::shared_ptr<Player> player, const std::string& direction);
    std::string handle_players_command(std::shared_ptr<Player> player);

    // World initialization
    void initialize_world();

    // Utility
    bool is_valid_room_id(int room_id) const;
    std::string get_room_list() const;

private:
    std::unordered_map<int, std::shared_ptr<Room>> rooms_;
    std::unordered_map<std::shared_ptr<Player>, int> player_locations_; // Player -> Room ID

    void create_starting_areas();
};

} // namespace dungeon_merc
