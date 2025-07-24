#include "room.hpp"
#include "player.hpp"
#include <algorithm>
#include <sstream>

using namespace dungeon_merc;

Room::Room(int id, const std::string& name, const std::string& description)
    : id_(id), name_(name), description_(description) {
}

void Room::add_exit(Direction dir, int target_room_id) {
    exits_[dir] = target_room_id;
}

bool Room::has_exit(Direction dir) const {
    return exits_.find(dir) != exits_.end();
}

int Room::get_exit_room_id(Direction dir) const {
    auto it = exits_.find(dir);
    return (it != exits_.end()) ? it->second : -1;
}

std::string Room::get_exit_description(Direction dir) const {
    if (!has_exit(dir)) {
        return "There is no exit in that direction.";
    }

    std::string dir_str = direction_to_string(dir);
    return "You can go " + dir_str + ".";
}

std::vector<std::string> Room::get_available_exits() const {
    std::vector<std::string> exits;
    for (const auto& exit : exits_) {
        exits.push_back(direction_to_string(exit.first));
    }
    return exits;
}

void Room::add_player(std::shared_ptr<Player> player) {
    // Check if player is already in this room
    auto it = std::find(players_.begin(), players_.end(), player);
    if (it == players_.end()) {
        players_.push_back(player);
    }
}

void Room::remove_player(std::shared_ptr<Player> player) {
    auto it = std::find(players_.begin(), players_.end(), player);
    if (it != players_.end()) {
        players_.erase(it);
    }
}

std::string Room::get_full_description() const {
    std::stringstream ss;
    ss << name_ << "\n";
    ss << description_ << "\n";

    if (!players_.empty()) {
        ss << "\nPlayers here: ";
        for (size_t i = 0; i < players_.size(); ++i) {
            if (i > 0) ss << ", ";
            ss << players_[i]->get_name();
        }
        ss << "\n";
    }

    ss << get_exits_list();
    return ss.str();
}

std::string Room::get_exits_list() const {
    if (exits_.empty()) {
        return "\nThere are no visible exits.";
    }

    std::stringstream ss;
    ss << "\nExits: ";

    std::vector<std::string> exit_names;
    for (const auto& exit : exits_) {
        exit_names.push_back(direction_to_string(exit.first));
    }

    for (size_t i = 0; i < exit_names.size(); ++i) {
        if (i > 0) ss << ", ";
        ss << exit_names[i];
    }

    return ss.str();
}
