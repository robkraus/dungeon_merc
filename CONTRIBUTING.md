# Contributing to Dungeon Merc

Thank you for your interest in contributing to Dungeon Merc! This document provides guidelines and information for contributors.

## Getting Started

1. **Fork the repository** on GitHub
2. **Clone your fork** locally
3. **Create a feature branch** for your changes
4. **Make your changes** following the coding standards
5. **Test your changes** thoroughly
6. **Submit a pull request**

## Development Setup

### Prerequisites
- C++17 compatible compiler
- CMake 3.16+
- Git

### Building from Source
```bash
git clone https://github.com/your-username/dungeon_merc.git
cd dungeon_merc
mkdir build && cd build
cmake ..
make
```

## Coding Standards

### C++ Style Guide
- Use **C++17** features
- Follow **snake_case** for variables and functions
- Use **PascalCase** for classes and structs
- Use **UPPER_CASE** for constants and macros
- Indent with **4 spaces** (no tabs)
- Line length: **120 characters** maximum

### Code Organization
- Keep functions focused and under 50 lines when possible
- Use meaningful variable and function names
- Add comments for complex logic
- Include header guards in all header files
- Use `#pragma once` for header guards

### Example Code Style
```cpp
#pragma once

#include <string>
#include <vector>

namespace dungeon_merc {

class Player {
public:
    Player(const std::string& name, int health);
    ~Player() = default;

    void take_damage(int amount);
    bool is_alive() const;

private:
    std::string name_;
    int health_;
    int max_health_;
};

} // namespace dungeon_merc
```

## Testing

### Running Tests
```bash
cd build
make test
```

### Writing Tests
- Create test files in the `test/` directory
- Use descriptive test names
- Test both success and failure cases
- Mock external dependencies when appropriate

### Test Example
```cpp
#include <gtest/gtest.h>
#include "player.hpp"

TEST(PlayerTest, TakeDamage) {
    Player player("TestPlayer", 100);

    player.take_damage(30);
    EXPECT_EQ(player.get_health(), 70);
    EXPECT_TRUE(player.is_alive());

    player.take_damage(100);
    EXPECT_FALSE(player.is_alive());
}
```

## Git Workflow

### Branch Naming
- Feature branches: `feature/description`
- Bug fixes: `fix/description`
- Documentation: `docs/description`
- Refactoring: `refactor/description`

### Commit Messages
Use conventional commit format:
```
type(scope): description

[optional body]

[optional footer]
```

Types:
- `feat`: New feature
- `fix`: Bug fix
- `docs`: Documentation changes
- `style`: Code style changes
- `refactor`: Code refactoring
- `test`: Adding or updating tests
- `chore`: Maintenance tasks

Examples:
```
feat(player): add health regeneration system

fix(combat): resolve damage calculation bug

docs(readme): update installation instructions
```

### Pull Request Guidelines
1. **Title**: Clear, descriptive title
2. **Description**: Explain what and why (not how)
3. **Related Issues**: Link to any related issues
4. **Testing**: Describe how you tested your changes
5. **Screenshots**: Include screenshots for UI changes

## Issue Reporting

### Bug Reports
When reporting bugs, please include:
- **Description**: Clear description of the bug
- **Steps to Reproduce**: Step-by-step instructions
- **Expected Behavior**: What should happen
- **Actual Behavior**: What actually happens
- **Environment**: OS, compiler version, etc.
- **Screenshots**: If applicable

### Feature Requests
When requesting features, please include:
- **Description**: Clear description of the feature
- **Use Case**: Why this feature is needed
- **Proposed Implementation**: Any ideas for implementation
- **Alternatives**: Any alternative solutions considered

## Code Review Process

1. **Automated Checks**: All PRs must pass CI checks
2. **Code Review**: At least one maintainer must approve
3. **Testing**: Changes must include appropriate tests
4. **Documentation**: Update documentation as needed

## Getting Help

- **Issues**: Use GitHub issues for bugs and feature requests
- **Discussions**: Use GitHub discussions for questions and ideas
- **Discord**: Join our Discord server for real-time chat

## Recognition

Contributors will be recognized in:
- The project README
- Release notes
- Contributor hall of fame

Thank you for contributing to Dungeon Merc!
