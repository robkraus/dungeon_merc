#include "player.hpp"

namespace dungeon_merc {

Player::Player(const std::string& name, CharacterClass character_class)
    : name_(name)
    , character_class_(character_class)
    , health_(DEFAULT_HEALTH)
    , max_health_(DEFAULT_HEALTH)
    , level_(1)
    , experience_(0)
    , experience_to_next_level_(100)
    , game_state_(GameState::LOBBY)
    , last_login_(std::chrono::system_clock::now()) {

    // Set character class specific stats
    switch (character_class_) {
        case CharacterClass::SCOUT:
            max_health_ = 80;
            break;
        case CharacterClass::ENFORCER:
            max_health_ = 120;
            break;
        case CharacterClass::TECH:
            max_health_ = 90;
            break;
        case CharacterClass::GHOST:
            max_health_ = 85;
            break;
    }

    health_ = max_health_;

    LOG_INFO("Created player: " + name + " (" + class_to_string(character_class_) + ")");
}

void Player::take_damage(int amount) {
    if (amount <= 0) {
        return;
    }

    health_ = std::max(0, health_ - amount);
    LOG_INFO("Player " + name_ + " took " + std::to_string(amount) + " damage. Health: " + std::to_string(health_));

    if (!is_alive()) {
        LOG_INFO("Player " + name_ + " has died!");
    }
}

void Player::heal(int amount) {
    if (amount <= 0) {
        return;
    }

    health_ = std::min(max_health_, health_ + amount);
    LOG_INFO("Player " + name_ + " healed " + std::to_string(amount) + " health. Health: " + std::to_string(health_));
}

bool Player::is_alive() const {
    return health_ > 0;
}

void Player::gain_experience(int amount) {
    if (amount <= 0) {
        return;
    }

    experience_ += amount;
    LOG_INFO("Player " + name_ + " gained " + std::to_string(amount) + " experience. Total: " + std::to_string(experience_));

    // Check for level up
    while (experience_ >= experience_to_next_level_) {
        level_up();
    }
}

void Player::level_up() {
    level_++;
    experience_ -= experience_to_next_level_;

    // Increase stats
    max_health_ += 10;
    health_ = max_health_; // Full heal on level up

    calculate_experience_to_next_level();

    LOG_INFO("Player " + name_ + " reached level " + std::to_string(level_) + "!");
}



void Player::calculate_experience_to_next_level() {
    // Simple exponential experience curve
    experience_to_next_level_ = level_ * 100;
}

} // namespace dungeon_merc
