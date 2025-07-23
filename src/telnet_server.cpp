#include "telnet_server.hpp"
#include "player.hpp"
#include <iostream>
#include <cstring>
#include <openssl/evp.h>
#include <iomanip>
#include <sstream>

namespace dungeon_merc {

// TelnetConnection implementation
TelnetConnection::TelnetConnection(int socket_fd, const std::string& client_ip)
    : socket_fd_(socket_fd)
    , client_ip_(client_ip)
    , state_(TelnetConnectionState::CONNECTING)
    , receive_buffer_(1024) {

    LOG_INFO("New telnet connection from " + client_ip_);
}

TelnetConnection::~TelnetConnection() {
    close();
}

bool TelnetConnection::initialize() {
    if (!set_nonblocking()) {
        LOG_ERROR("Failed to set socket non-blocking");
        return false;
    }

    // For vibe coding, auto-authenticate for now
    state_ = TelnetConnectionState::AUTHENTICATED;
    username_ = "player";
    LOG_INFO("Auto-authenticated connection from " + client_ip_);
    return true;
}

void TelnetConnection::close() {
    if (state_ == TelnetConnectionState::DISCONNECTED) {
        return;
    }

    LOG_INFO("Closing telnet connection from " + client_ip_);

    if (socket_fd_ >= 0) {
        ::close(socket_fd_);
        socket_fd_ = -1;
    }

    state_ = TelnetConnectionState::DISCONNECTED;
}

bool TelnetConnection::is_connected() const {
    return state_ != TelnetConnectionState::DISCONNECTED && socket_fd_ >= 0;
}

bool TelnetConnection::authenticate(const std::string& username, const std::string& password) {
    (void)password; // Suppress unused parameter warning
    if (state_ != TelnetConnectionState::AUTHENTICATING) {
        return false;
    }

    if (username.empty() || username.length() > MAX_USERNAME_LENGTH) {
        return false;
    }

    // For vibe coding, accept any username for now
    username_ = username;
    state_ = TelnetConnectionState::AUTHENTICATED;
    LOG_INFO("Telnet authentication successful for user: " + username);
    return true;
}

bool TelnetConnection::is_authenticated() const {
    return state_ == TelnetConnectionState::AUTHENTICATED || state_ == TelnetConnectionState::PLAYING;
}

bool TelnetConnection::send_message(const std::string& message) {
    if (!is_authenticated()) {
        LOG_DEBUG("Cannot send message - not authenticated");
        return false;
    }

    std::string formatted_message = message + "\r\n";
    ssize_t written = send(socket_fd_, formatted_message.c_str(), formatted_message.length(), 0);

    if (written < 0) {
        LOG_ERROR("Failed to send message to telnet client: " + std::to_string(written));
        return false;
    }

    LOG_DEBUG("Sent message: " + message);
    return true;
}

std::string TelnetConnection::receive_message() {
    char buffer[1024];
    ssize_t bytes_read = recv(socket_fd_, buffer, sizeof(buffer) - 1, 0);

    if (bytes_read > 0) {
        buffer[bytes_read] = '\0';
        std::string message(buffer);

        LOG_DEBUG("Raw message received: '" + message + "'");

        // Remove carriage return and line feed (like the working simple server)
        if (message.back() == '\n') message.pop_back();
        if (message.back() == '\r') message.pop_back();

        LOG_DEBUG("Processed message: '" + message + "'");
        return message;
    }

    return "";
}

bool TelnetConnection::has_data() const {
    // Always check for data, regardless of authentication state
    char buffer[1];
    ssize_t result = recv(socket_fd_, buffer, 1, MSG_PEEK | MSG_DONTWAIT);
    if (result > 0) {
        LOG_DEBUG("Data available on socket " + std::to_string(socket_fd_));
    }
    return result > 0;
}

void TelnetConnection::set_player(std::shared_ptr<Player> player) {
    player_ = player;
    if (player) {
        state_ = TelnetConnectionState::PLAYING;
    }
}

std::shared_ptr<Player> TelnetConnection::get_player() const {
    return player_;
}

bool TelnetConnection::set_nonblocking() {
    int flags = fcntl(socket_fd_, F_GETFL, 0);
    if (flags < 0) {
        return false;
    }

    return fcntl(socket_fd_, F_SETFL, flags | O_NONBLOCK) == 0;
}

// TelnetServer implementation
TelnetServer::TelnetServer(int port)
    : port_(port)
    , server_socket_(-1)
    , running_(false) {

    LOG_INFO("Telnet Server initialized on port " + std::to_string(port_));
}

TelnetServer::~TelnetServer() {
    shutdown();
}

bool TelnetServer::initialize() {
    if (!create_server_socket()) {
        LOG_ERROR("Failed to create server socket");
        return false;
    }

    if (!set_socket_options()) {
        LOG_ERROR("Failed to set socket options");
        return false;
    }

    running_ = true;
    LOG_INFO("Telnet Server started on port " + std::to_string(port_));
    return true;
}

void TelnetServer::shutdown() {
    if (!running_) {
        return;
    }

    LOG_INFO("Shutting down telnet server...");

    running_ = false;

    // Close all connections
    {
        std::lock_guard<std::mutex> lock(connections_mutex_);
        for (auto& conn : connections_) {
            conn->close();
        }
        connections_.clear();
    }

    // Close server socket
    if (server_socket_ >= 0) {
        ::close(server_socket_);
        server_socket_ = -1;
    }

    LOG_INFO("Telnet Server shutdown complete");
}

bool TelnetServer::is_running() const {
    return running_;
}

void TelnetServer::accept_connections() {
    if (!running_) {
        return;
    }

    struct sockaddr_in client_addr;
    socklen_t client_len = sizeof(client_addr);

    int client_socket = accept(server_socket_, (struct sockaddr*)&client_addr, &client_len);
    if (client_socket >= 0) {
        std::string client_ip = inet_ntoa(client_addr.sin_addr);

        auto connection = std::make_shared<TelnetConnection>(client_socket, client_ip);
        if (connection->initialize()) {
            std::lock_guard<std::mutex> lock(connections_mutex_);
            connections_.push_back(connection);

            if (connection_callback_) {
                connection_callback_(connection);
            }
        } else {
            connection->close();
        }
    }
}

void TelnetServer::process_connections() {
    std::lock_guard<std::mutex> lock(connections_mutex_);

    for (auto& connection : connections_) {
        if (!connection->is_connected()) {
            continue;
        }

        // Send welcome message if first time
        static std::unordered_map<int, bool> sent_welcome;
        if (!sent_welcome[connection->get_socket_fd()]) {
            connection->send_message("Welcome to Dungeon Merc!");
            connection->send_message("Type 'help' for available commands.");
            connection->send_message("> ");
            sent_welcome[connection->get_socket_fd()] = true;
        }

        // Process incoming messages - use the same approach as the working simple server
        char buffer[1024];
        ssize_t bytes_read = recv(connection->get_socket_fd(), buffer, sizeof(buffer) - 1, MSG_DONTWAIT);

        if (bytes_read > 0) {
            buffer[bytes_read] = '\0';
            std::string message(buffer);

            // Remove \r\n (like the working simple server)
            if (message.back() == '\n') message.pop_back();
            if (message.back() == '\r') message.pop_back();

            LOG_DEBUG("Game message from " + connection->get_client_ip() + ": " + message);

            // Handle game commands
            if (message == "help") {
                LOG_DEBUG("Sending help response");
                connection->send_message("Available commands:");
                connection->send_message("  help - Show this help");
                connection->send_message("  quit - Disconnect from server");
                connection->send_message("  status - Show your status");
                connection->send_message("> "); // Add prompt
            } else if (message == "quit") {
                LOG_DEBUG("User requested quit");
                connection->send_message("Goodbye!");
                connection->close();
            } else if (message == "status") {
                LOG_DEBUG("Sending status response");
                connection->send_message("You are connected to Dungeon Merc!");
                connection->send_message("Game features coming soon...");
                connection->send_message("> "); // Add prompt
            } else {
                LOG_DEBUG("Unknown command: " + message);
                connection->send_message("Unknown command: " + message);
                connection->send_message("Type 'help' for available commands.");
                connection->send_message("> "); // Add prompt
            }
        }
    }
}

void TelnetServer::remove_disconnected_connections() {
    std::lock_guard<std::mutex> lock(connections_mutex_);

    connections_.erase(
        std::remove_if(connections_.begin(), connections_.end(),
            [this](const std::shared_ptr<TelnetConnection>& conn) {
                if (!conn->is_connected()) {
                    if (disconnection_callback_) {
                        disconnection_callback_(conn);
                    }
                    return true;
                }
                return false;
            }),
        connections_.end()
    );
}

bool TelnetServer::add_user(const std::string& username, const std::string& password_hash) {
    std::lock_guard<std::mutex> lock(users_mutex_);
    users_[username] = password_hash;
    LOG_INFO("Added user: " + username);
    return true;
}

bool TelnetServer::remove_user(const std::string& username) {
    std::lock_guard<std::mutex> lock(users_mutex_);
    auto it = users_.find(username);
    if (it != users_.end()) {
        users_.erase(it);
        LOG_INFO("Removed user: " + username);
        return true;
    }
    return false;
}

bool TelnetServer::validate_credentials(const std::string& username, const std::string& password) {
    std::lock_guard<std::mutex> lock(users_mutex_);

    auto it = users_.find(username);
    if (it == users_.end()) {
        return false;
    }

    return verify_password(password, it->second);
}

bool TelnetServer::create_server_socket() {
    server_socket_ = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket_ < 0) {
        LOG_ERROR("Failed to create server socket");
        return false;
    }

    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(port_);

    if (bind(server_socket_, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        LOG_ERROR("Failed to bind server socket to port " + std::to_string(port_));
        return false;
    }

    if (listen(server_socket_, 10) < 0) {
        LOG_ERROR("Failed to listen on server socket");
        return false;
    }

    return true;
}

bool TelnetServer::set_socket_options() {
    int opt = 1;
    if (setsockopt(server_socket_, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        LOG_ERROR("Failed to set SO_REUSEADDR");
        return false;
    }

    if (fcntl(server_socket_, F_SETFL, O_NONBLOCK) < 0) {
        LOG_ERROR("Failed to set socket non-blocking");
        return false;
    }

    return true;
}

std::string TelnetServer::hash_password(const std::string& password) {
    unsigned char hash[EVP_MAX_MD_SIZE];
    unsigned int hash_len;

    EVP_MD_CTX* ctx = EVP_MD_CTX_new();
    if (!ctx) {
        return "";
    }

    if (EVP_DigestInit_ex(ctx, EVP_sha256(), nullptr) != 1) {
        EVP_MD_CTX_free(ctx);
        return "";
    }

    if (EVP_DigestUpdate(ctx, password.c_str(), password.length()) != 1) {
        EVP_MD_CTX_free(ctx);
        return "";
    }

    if (EVP_DigestFinal_ex(ctx, hash, &hash_len) != 1) {
        EVP_MD_CTX_free(ctx);
        return "";
    }

    EVP_MD_CTX_free(ctx);

    std::stringstream ss;
    for (unsigned int i = 0; i < hash_len; i++) {
        ss << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(hash[i]);
    }
    return ss.str();
}

bool TelnetServer::verify_password(const std::string& password, const std::string& hash) {
    std::string password_hash = hash_password(password);
    return password_hash == hash;
}

} // namespace dungeon_merc
