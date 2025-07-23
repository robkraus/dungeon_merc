# Dungeon Merc - SSH MUD Server

A secure console-based Multi-User Dungeon (MUD) server inspired by the Dungeon Merc concept - a game where players are contracted mercenaries delving into procedurally generated underground facilities.

## Tagline
"Delve deep. Get paid. Don't die."

## Overview

Dungeon Merc is an SSH-based MUD server where players take on the role of contracted mercenaries sent into unstable underground facilities to recover data, secure technology, or eliminate threats. Each run provides XP and currency for gear upgrades, with increasing risk as you go deeper.

## Features

- **SSH Server**: Secure multi-user SSH interface for connecting players
- **Authentication**: Username/password authentication with encrypted connections
- **Procedural Dungeons**: Dynamically generated underground facilities
- **Character Classes**: Scout, Enforcer, Tech, and Ghost with unique abilities
- **Combat System**: Turn-based combat with ranged and melee options
- **Loot System**: Ammo, credits, medkits, and weapon modifications
- **XP & Leveling**: Experience points and perk system
- **Persistent Characters**: Save/load character progress

## Gameplay Loop

1. **Choose Contract** - Select difficulty, rewards, and dungeon parameters
2. **Loadout Selection** - Equip weapons, items, and perks
3. **Enter Dungeon** - Navigate procedurally generated maps
4. **Combat & Loot** - Fight enemies, collect resources, complete objectives
5. **Extract or Die** - Success brings rewards, death means lost loot
6. **Return to Base** - Upgrade gear, level up, unlock new abilities

## Building the Project

### Prerequisites
- C++17 compatible compiler (GCC 7+, Clang 5+, or MSVC 2017+)
- CMake 3.16 or higher
- pthread library (usually included with compiler)

### Using CMake (Recommended)
```bash
mkdir build
cd build
cmake ..
make
```

### Using Makefile
```bash
make
```

### Running the Server
```bash
# Default port 22 (SSH)
./bin/dungeon_merc

# Custom port
./bin/dungeon_merc --port 2222

# Debug mode
./bin/dungeon_merc --debug
```

## Connecting to the Server

Use any SSH client to connect:
```bash
ssh localhost
# or
ssh -p 2222 localhost
```

Or use a terminal-based SSH client:
- OpenSSH (built-in on most systems)
- PuTTY (Windows)
- iTerm2 (macOS)
- Any SSH-compatible terminal

## Project Structure

```
dungeon_merc/
├── src/           # Source files
├── include/       # Header files
├── lib/           # Third-party libraries
├── test/          # Unit tests
├── docs/          # Documentation
├── scripts/       # Build and utility scripts
├── CMakeLists.txt # CMake configuration
├── Makefile       # Alternative build system
└── README.md      # This file
```

## Character Classes

### Scout
- **Role**: Fast, sneaky reconnaissance
- **Weapons**: Light weapons, silenced pistols
- **Abilities**: Faster movement, silent steps, enhanced vision

### Enforcer
- **Role**: Tanky frontline combat
- **Weapons**: Shotguns, melee weapons, heavy armor
- **Abilities**: Extra armor, knockback resistance, area damage

### Tech
- **Role**: Gadget specialist and hacker
- **Weapons**: Energy weapons, drones, hacking tools
- **Abilities**: Trap disabling, terminal hacking, drone control

### Ghost
- **Role**: Stealth assassin
- **Weapons**: Melee weapons, silenced weapons, cloaking tech
- **Abilities**: Short invisibility, backstab bonuses, silent movement

## Dungeon Elements

- **Map Types**: Ruined labs, haunted bunkers, alien mines, corporate facilities
- **Hazards**: Gas leaks, electrified floors, locked doors, radiation zones
- **Objectives**: Find keycards, destroy server racks, free prisoners, extract data
- **Enemies**: Mutated guards, rogue drones, cult soldiers, alien creatures

## Development

### Adding New Features
1. Create feature branch: `git checkout -b feature/new-feature`
2. Implement changes in `src/` and `include/`
3. Add tests in `test/`
4. Update documentation
5. Submit pull request

### Running Tests
```bash
cd build
make test
```

### Code Style
- Follow C++17 standards
- Use meaningful variable and function names
- Add comments for complex logic
- Keep functions focused and small

## Configuration

The server can be configured via command line arguments or configuration files:

```bash
./dungeon_merc --port 4000 --max-players 50 --save-interval 300
```

## Contributing

1. Fork the repository
2. Create a feature branch
3. Make your changes
4. Add tests for new functionality
5. Ensure all tests pass
6. Submit a pull request

## License

This project is licensed under the MIT License - see the LICENSE file for details.

## Acknowledgments

- Inspired by classic MUD games and modern roguelike mechanics
- Built with modern C++ for performance and maintainability
- Designed for extensibility and modding

## Roadmap

- [x] Basic SSH server implementation
- [ ] Character creation and persistence
- [ ] Procedural dungeon generation
- [ ] Combat system
- [ ] Multi-player support
- [ ] Web interface
- [ ] Mobile client
- [ ] Modding support
