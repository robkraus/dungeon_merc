#pragma once

#include "common.hpp"
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <vector>
#include <memory>
#include <functional>

namespace dungeon_merc {

// Forward declarations
class Player;
class TelnetConnection;

// Telnet connection state
enum class TelnetConnectionState {
    CONNECTING,
    AUTHENTICATING,
    AUTHENTICATED,
    PLAYING,
    DISCONNECTED
};

// Telnet connection class
class TelnetConnection {
public:
    TelnetConnection(int socket_fd, const std::string& client_ip);
    ~TelnetConnection();

    // Connection management
    bool initialize();
    void close();
    bool is_connected() const;

    // Authentication
    bool authenticate(const std::string& username, const std::string& password);
    bool is_authenticated() const;

    // I/O operations
    bool send_message(const std::string& message);
    std::string receive_message();
    bool has_data() const;

    // Player association
    void set_player(std::shared_ptr<Player> player);
    std::shared_ptr<Player> get_player() const;

    // Getters
    int get_socket_fd() const { return socket_fd_; }
    const std::string& get_client_ip() const { return client_ip_; }
    const std::string& get_username() const { return username_; }
    TelnetConnectionState get_state() const { return state_; }

    // Event callbacks
    using MessageCallback = std::function<void(const std::string&)>;
    void set_message_callback(MessageCallback callback) { message_callback_ = callback; }

private:
    int socket_fd_;
    std::string client_ip_;
    std::string username_;
    TelnetConnectionState state_;

    // Player association
    std::shared_ptr<Player> player_;

    // Callbacks
    MessageCallback message_callback_;

    // Buffer for receiving data
    std::vector<char> receive_buffer_;

    // Helper methods
    bool set_nonblocking();
};

// Telnet server class
class TelnetServer {
public:
    TelnetServer(int port = 4000);
    ~TelnetServer();

    // Server management
    bool initialize();
    void shutdown();
    bool is_running() const;

    // Connection handling
    void accept_connections();
    void process_connections();
    void remove_disconnected_connections();

    // Authentication
    bool add_user(const std::string& username, const std::string& password_hash);
    bool remove_user(const std::string& username);
    bool validate_credentials(const std::string& username, const std::string& password);

    // Event callbacks
    using ConnectionCallback = std::function<void(std::shared_ptr<TelnetConnection>)>;
    using DisconnectionCallback = std::function<void(std::shared_ptr<TelnetConnection>)>;

    void set_connection_callback(ConnectionCallback callback) { connection_callback_ = callback; }
    void set_disconnection_callback(DisconnectionCallback callback) { disconnection_callback_ = callback; }

    // Getters
    int get_port() const { return port_; }
    const std::vector<std::shared_ptr<TelnetConnection>>& get_connections() const { return connections_; }
    size_t get_connection_count() const { return connections_.size(); }

private:
    int port_;
    int server_socket_;
    bool running_;

    // Active connections
    std::vector<std::shared_ptr<TelnetConnection>> connections_;

    // User database (simple in-memory for now)
    std::unordered_map<std::string, std::string> users_; // username -> password_hash

    // Callbacks
    ConnectionCallback connection_callback_;
    DisconnectionCallback disconnection_callback_;

    // Helper methods
    bool create_server_socket();
    bool set_socket_options();
    std::string hash_password(const std::string& password);
    bool verify_password(const std::string& password, const std::string& hash);

    // Thread safety
    mutable std::mutex connections_mutex_;
    mutable std::mutex users_mutex_;
};

} // namespace dungeon_merc
