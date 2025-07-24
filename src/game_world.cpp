#include "game_world.hpp"
#include "common.hpp"
#include <sstream>
#include <algorithm>

using namespace dungeon_merc;

GameWorld::GameWorld() {
    initialize_world();
}

void GameWorld::add_room(std::shared_ptr<Room> room) {
    rooms_[room->get_id()] = room;
}

std::shared_ptr<Room> GameWorld::get_room(int room_id) const {
    auto it = rooms_.find(room_id);
    return (it != rooms_.end()) ? it->second : nullptr;
}

std::shared_ptr<Room> GameWorld::get_player_room(std::shared_ptr<Player> player) const {
    auto it = player_locations_.find(player);
    if (it == player_locations_.end()) {
        return nullptr;
    }
    return get_room(it->second);
}

void GameWorld::add_player(std::shared_ptr<Player> player, int starting_room_id) {
    if (!is_valid_room_id(starting_room_id)) {
        starting_room_id = 1; // Default to room 1 if invalid
    }

    player_locations_[player] = starting_room_id;

    auto room = get_room(starting_room_id);
    if (room) {
        room->add_player(player);
    }
}

void GameWorld::remove_player(std::shared_ptr<Player> player) {
    auto current_room = get_player_room(player);
    if (current_room) {
        current_room->remove_player(player);
    }

    player_locations_.erase(player);
}

bool GameWorld::move_player(std::shared_ptr<Player> player, Direction direction) {
    auto current_room = get_player_room(player);
    if (!current_room) {
        return false;
    }

    if (!current_room->has_exit(direction)) {
        return false;
    }

    int target_room_id = current_room->get_exit_room_id(direction);
    auto target_room = get_room(target_room_id);
    if (!target_room) {
        return false;
    }

    // Remove player from current room
    current_room->remove_player(player);

    // Add player to new room
    target_room->add_player(player);
    player_locations_[player] = target_room_id;

    return true;
}

std::string GameWorld::handle_look_command(std::shared_ptr<Player> player) {
    auto room = get_player_room(player);
    if (!room) {
        return "You are lost in the void...";
    }

    return room->get_full_description();
}

std::string GameWorld::handle_move_command(std::shared_ptr<Player> player, const std::string& direction) {
    if (!is_valid_direction(direction)) {
        return "You can't go that way. Try: north, south, east, west, up, down";
    }

    Direction dir = string_to_direction(direction);
    auto current_room = get_player_room(player);

    if (!current_room) {
        return "You are lost in the void...";
    }

    if (!current_room->has_exit(dir)) {
        return "There is no exit in that direction.";
    }

    if (move_player(player, dir)) {
        auto new_room = get_player_room(player);
        return "You move " + direction_to_string(dir) + ".\n\n" + new_room->get_full_description();
    }

    return "You can't go that way.";
}

std::string GameWorld::handle_players_command(std::shared_ptr<Player> player) {
    auto room = get_player_room(player);
    if (!room) {
        return "You are lost in the void...";
    }

    const auto& players = room->get_players();
    if (players.empty()) {
        return "You are alone here.";
    }

    std::stringstream ss;
    ss << "Players in this room: ";
    for (size_t i = 0; i < players.size(); ++i) {
        if (i > 0) ss << ", ";
        ss << players[i]->get_name();
    }

    return ss.str();
}

void GameWorld::initialize_world() {
    create_starting_areas();
}

bool GameWorld::is_valid_room_id(int room_id) const {
    return rooms_.find(room_id) != rooms_.end();
}

std::string GameWorld::get_room_list() const {
    std::stringstream ss;
    ss << "Available rooms:\n";
    for (const auto& room_pair : rooms_) {
        ss << "  " << room_pair.first << ": " << room_pair.second->get_name() << "\n";
    }
    return ss.str();
}

void GameWorld::create_starting_areas() {
    // Create a simple starting area with a few connected rooms

    // Room 1: Town Square
    auto town_square = std::make_shared<Room>(1, "Town Square",
        "You stand in the bustling town square of Dungeon Merc. The cobblestone streets are worn smooth by countless adventurers who have passed through here. A fountain bubbles in the center, and you can see various shops and inns lining the square.");
    add_room(town_square);

    // Room 2: Tavern
    auto tavern = std::make_shared<Room>(2, "The Rusty Sword Tavern",
        "The warm glow of candlelight fills this cozy tavern. The air is thick with the smell of ale and roasted meat. Adventurers gather here to share tales of their exploits and plan their next dungeon dive.");
    add_room(tavern);

    // Room 3: Blacksmith
    auto blacksmith = std::make_shared<Room>(3, "Ironforge Blacksmith",
        "The clang of hammer on anvil echoes through this workshop. The blacksmith's forge glows red-hot, and weapons and armor of all kinds hang from the walls. The air is thick with the smell of burning coal and hot metal.");
    add_room(blacksmith);

    // Room 4: Dungeon Entrance
    auto dungeon_entrance = std::make_shared<Room>(4, "Dungeon Entrance",
        "A dark opening in the earth yawns before you. Ancient stone steps lead down into the depths, and a cold breeze carries the scent of damp earth and mystery from below. This is where the real adventure begins.");
    add_room(dungeon_entrance);

    // Room 5: First Dungeon Chamber
    auto dungeon_chamber = std::make_shared<Room>(5, "Ancient Chamber",
        "You find yourself in a large, circular chamber carved from solid stone. Torches flicker on the walls, casting dancing shadows. Ancient runes are carved into the walls, telling tales of forgotten heroes and lost treasures.");
    add_room(dungeon_chamber);

    // Connect the rooms
    town_square->add_exit(Direction::NORTH, 2);  // To tavern
    town_square->add_exit(Direction::EAST, 3);   // To blacksmith
    town_square->add_exit(Direction::SOUTH, 4);  // To dungeon entrance

    tavern->add_exit(Direction::SOUTH, 1);       // Back to town square

    blacksmith->add_exit(Direction::WEST, 1);    // Back to town square

    dungeon_entrance->add_exit(Direction::NORTH, 1);  // Back to town square
    dungeon_entrance->add_exit(Direction::DOWN, 5);   // To dungeon chamber

    dungeon_chamber->add_exit(Direction::UP, 4);      // Back to dungeon entrance
}
