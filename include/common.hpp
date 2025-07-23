#pragma once

#include <string>
#include <vector>
#include <memory>
#include <chrono>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <functional>
#include <unordered_map>
#include <queue>
#include <iostream>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <random>
#include <cstdint>
#include <cassert>

namespace dungeon_merc {

// Forward declarations
class Player;
class Room;
class Item;
class Monster;
class GameWorld;

// Type definitions
using PlayerId = uint32_t;
using RoomId = uint32_t;
using ItemId = uint32_t;
using MonsterId = uint32_t;
using Timestamp = std::chrono::system_clock::time_point;

// Constants
constexpr int DEFAULT_PORT = 4000;
constexpr int MAX_PLAYERS = 100;
constexpr int MAX_ROOM_SIZE = 100;
constexpr int MAX_INVENTORY_SIZE = 50;
constexpr int DEFAULT_HEALTH = 100;
constexpr int DEFAULT_MANA = 50;

// Enums
enum class Direction {
    NORTH,
    SOUTH,
    EAST,
    WEST,
    UP,
    DOWN
};

enum class CharacterClass {
    SCOUT,
    ENFORCER,
    TECH,
    GHOST
};

enum class ItemType {
    WEAPON,
    ARMOR,
    CONSUMABLE,
    KEY,
    TREASURE
};

enum class MonsterType {
    GUARD,
    DRONE,
    CULTIST,
    ALIEN
};

enum class GameState {
    LOBBY,
    IN_GAME,
    COMBAT,
    MENU
};

// Utility functions
inline std::string direction_to_string(Direction dir) {
    switch (dir) {
        case Direction::NORTH: return "north";
        case Direction::SOUTH: return "south";
        case Direction::EAST: return "east";
        case Direction::WEST: return "west";
        case Direction::UP: return "up";
        case Direction::DOWN: return "down";
        default: return "unknown";
    }
}

inline Direction string_to_direction(const std::string& str) {
    std::string lower = str;
    std::transform(lower.begin(), lower.end(), lower.begin(), ::tolower);

    if (lower == "north" || lower == "n") return Direction::NORTH;
    if (lower == "south" || lower == "s") return Direction::SOUTH;
    if (lower == "east" || lower == "e") return Direction::EAST;
    if (lower == "west" || lower == "w") return Direction::WEST;
    if (lower == "up" || lower == "u") return Direction::UP;
    if (lower == "down" || lower == "d") return Direction::DOWN;

    throw std::invalid_argument("Invalid direction: " + str);
}

inline std::string class_to_string(CharacterClass cls) {
    switch (cls) {
        case CharacterClass::SCOUT: return "Scout";
        case CharacterClass::ENFORCER: return "Enforcer";
        case CharacterClass::TECH: return "Tech";
        case CharacterClass::GHOST: return "Ghost";
        default: return "Unknown";
    }
}

// Random number generation
class RandomGenerator {
public:
    static RandomGenerator& get_instance() {
        static RandomGenerator instance;
        return instance;
    }

    int random_int(int min, int max) {
        std::uniform_int_distribution<int> dist(min, max);
        return dist(engine_);
    }

    double random_double(double min, double max) {
        std::uniform_real_distribution<double> dist(min, max);
        return dist(engine_);
    }

    bool random_bool(double probability = 0.5) {
        return random_double(0.0, 1.0) < probability;
    }

    template<typename Container>
    typename Container::value_type random_choice(const Container& container) {
        if (container.empty()) {
            throw std::runtime_error("Cannot choose from empty container");
        }
        auto it = container.begin();
        std::advance(it, random_int(0, container.size() - 1));
        return *it;
    }

private:
    RandomGenerator() : engine_(std::chrono::system_clock::now().time_since_epoch().count()) {}
    std::mt19937 engine_;
};

// Logging
enum class LogLevel {
    DEBUG,
    INFO,
    WARNING,
    ERROR
};

class Logger {
public:
    static Logger& get_instance() {
        static Logger instance;
        return instance;
    }

    void log(LogLevel level, const std::string& message) {
        std::lock_guard<std::mutex> lock(mutex_);

        auto now = std::chrono::system_clock::now();
        auto time_t = std::chrono::system_clock::to_time_t(now);
        auto tm = *std::localtime(&time_t);

        std::string level_str;
        switch (level) {
            case LogLevel::DEBUG: level_str = "DEBUG"; break;
            case LogLevel::INFO: level_str = "INFO"; break;
            case LogLevel::WARNING: level_str = "WARNING"; break;
            case LogLevel::ERROR: level_str = "ERROR"; break;
        }

        std::cout << "[" << std::put_time(&tm, "%Y-%m-%d %H:%M:%S") << "] "
                  << "[" << level_str << "] " << message << std::endl;
    }

    void debug(const std::string& message) { log(LogLevel::DEBUG, message); }
    void info(const std::string& message) { log(LogLevel::INFO, message); }
    void warning(const std::string& message) { log(LogLevel::WARNING, message); }
    void error(const std::string& message) { log(LogLevel::ERROR, message); }

private:
    std::mutex mutex_;
};

// Macros for easy logging
#define LOG_DEBUG(msg) dungeon_merc::Logger::get_instance().debug(msg)
#define LOG_INFO(msg) dungeon_merc::Logger::get_instance().info(msg)
#define LOG_WARNING(msg) dungeon_merc::Logger::get_instance().warning(msg)
#define LOG_ERROR(msg) dungeon_merc::Logger::get_instance().error(msg)

// Exception classes
class GameException : public std::runtime_error {
public:
    explicit GameException(const std::string& message) : std::runtime_error(message) {}
};

class NetworkException : public GameException {
public:
    explicit NetworkException(const std::string& message) : GameException(message) {}
};

class ValidationException : public GameException {
public:
    explicit ValidationException(const std::string& message) : GameException(message) {}
};

// Utility functions
inline std::string trim(const std::string& str) {
    size_t start = str.find_first_not_of(" \t\r\n");
    if (start == std::string::npos) return "";
    size_t end = str.find_last_not_of(" \t\r\n");
    return str.substr(start, end - start + 1);
}

inline std::vector<std::string> split(const std::string& str, char delimiter) {
    std::vector<std::string> tokens;
    std::stringstream ss(str);
    std::string token;

    while (std::getline(ss, token, delimiter)) {
        if (!token.empty()) {
            tokens.push_back(trim(token));
        }
    }

    return tokens;
}

inline std::string to_lower(const std::string& str) {
    std::string result = str;
    std::transform(result.begin(), result.end(), result.begin(), ::tolower);
    return result;
}

inline std::string to_upper(const std::string& str) {
    std::string result = str;
    std::transform(result.begin(), result.end(), result.begin(), ::toupper);
    return result;
}

} // namespace dungeon_merc
