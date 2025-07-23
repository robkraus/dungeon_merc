#include "common.hpp"
#include "telnet_server.hpp"
#include "player.hpp"
#include <iostream>
#include <csignal>
#include <cstdlib>
#include <memory>

using namespace dungeon_merc;

// Global flag for graceful shutdown
std::atomic<bool> g_shutdown_requested(false);

// Signal handler for graceful shutdown
void signal_handler(int signal) {
    LOG_INFO("Received shutdown signal: " + std::to_string(signal));
    g_shutdown_requested = true;
}

// Function to print usage information
void print_usage(const char* program_name) {
    std::cout << "Dungeon Merc - Telnet MUD Server\n";
    std::cout << "Usage: " << program_name << " [OPTIONS]\n\n";
    std::cout << "Options:\n";
    std::cout << "  -p, --port PORT        Server port (default: " << DEFAULT_PORT << ")\n";
    std::cout << "  -m, --max-players NUM  Maximum players (default: " << MAX_PLAYERS << ")\n";
    std::cout << "  -d, --debug            Enable debug mode\n";
    std::cout << "  -v, --version          Show version information\n";
    std::cout << "  -h, --help             Show this help message\n\n";
    std::cout << "Examples:\n";
    std::cout << "  " << program_name << "                    # Start with default settings\n";
    std::cout << "  " << program_name << " --port 4000        # Start on port 4000\n";
    std::cout << "  " << program_name << " --debug            # Start in debug mode\n";
}

// Function to print version information
void print_version() {
    std::cout << "Dungeon Merc Telnet MUD Server v1.0.0\n";
    std::cout << "Copyright (c) 2024 Dungeon Merc Project\n";
    std::cout << "License: MIT\n";
}

// Function to parse command line arguments
struct ServerConfig {
    int port = DEFAULT_PORT;
    int max_players = MAX_PLAYERS;
    bool debug_mode = false;
};

ServerConfig parse_arguments(int argc, char* argv[]) {
    ServerConfig config;

    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];

        if (arg == "-h" || arg == "--help") {
            print_usage(argv[0]);
            exit(0);
        } else if (arg == "-v" || arg == "--version") {
            print_version();
            exit(0);
        } else if (arg == "-p" || arg == "--port") {
            if (i + 1 >= argc) {
                LOG_ERROR("Port number required after --port");
                exit(1);
            }
            try {
                config.port = std::stoi(argv[++i]);
                if (config.port <= 0 || config.port > 65535) {
                    throw std::invalid_argument("Port out of range");
                }
            } catch (const std::exception& e) {
                LOG_ERROR("Invalid port number: " + std::string(argv[i]));
                exit(1);
            }
        } else if (arg == "-m" || arg == "--max-players") {
            if (i + 1 >= argc) {
                LOG_ERROR("Player count required after --max-players");
                exit(1);
            }
            try {
                config.max_players = std::stoi(argv[++i]);
                if (config.max_players <= 0) {
                    throw std::invalid_argument("Player count must be positive");
                }
            } catch (const std::exception& e) {
                LOG_ERROR("Invalid player count: " + std::string(argv[i]));
                exit(1);
            }
        } else if (arg == "-d" || arg == "--debug") {
            config.debug_mode = true;
        } else {
            LOG_WARNING("Unknown argument: " + arg);
        }
    }

    return config;
}

// Main server initialization and run function
int run_server(const ServerConfig& config) {
    try {
        LOG_INFO("Starting Dungeon Merc Telnet MUD Server");
        LOG_INFO("Port: " + std::to_string(config.port));
        LOG_INFO("Max Players: " + std::to_string(config.max_players));
        LOG_INFO("Debug Mode: " + std::string(config.debug_mode ? "Enabled" : "Disabled"));

        // Initialize telnet server
        auto telnet_server = std::make_unique<TelnetServer>(config.port);

        if (!telnet_server->initialize()) {
            LOG_ERROR("Failed to initialize telnet server");
            return 1;
        }

        LOG_INFO("Telnet Server initialized successfully");

        // Main server loop
        while (!g_shutdown_requested) {
            // Accept new connections
            telnet_server->accept_connections();

            // Process existing connections
            telnet_server->process_connections();

            // Clean up disconnected connections
            telnet_server->remove_disconnected_connections();

            // Small delay to prevent busy waiting
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }

        LOG_INFO("Shutting down server...");
        telnet_server->shutdown();
        LOG_INFO("Server shutdown complete");
        return 0;

    } catch (const std::exception& e) {
        LOG_ERROR("Server error: " + std::string(e.what()));
        return 1;
    }
}

int main(int argc, char* argv[]) {
    try {
        // Set up signal handlers for graceful shutdown
        signal(SIGINT, signal_handler);
        signal(SIGTERM, signal_handler);

        // Parse command line arguments
        ServerConfig config = parse_arguments(argc, argv);

        // Set up logging
        if (config.debug_mode) {
            LOG_INFO("Debug mode enabled");
        }

        // Run the server
        return run_server(config);

    } catch (const std::exception& e) {
        std::cerr << "Fatal error: " << e.what() << std::endl;
        return 1;
    } catch (...) {
        std::cerr << "Unknown fatal error occurred" << std::endl;
        return 1;
    }
}
