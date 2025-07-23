#pragma once

#include "common.hpp"
#include <string>

namespace dungeon_merc {

// Item class (placeholder for now)
class Item {
public:
    Item(ItemId id, const std::string& name, ItemType type, int value = 0);
    ~Item() = default;

    // Basic properties
    ItemId get_id() const { return id_; }
    const std::string& get_name() const { return name_; }
    ItemType get_type() const { return type_; }
    int get_value() const { return value_; }
    
    // Item actions
    virtual bool use(std::shared_ptr<Player> player);
    virtual std::string get_description() const;

private:
    ItemId id_;
    std::string name_;
    ItemType type_;
    int value_;
};

} // namespace dungeon_merc 