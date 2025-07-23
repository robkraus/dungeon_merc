#include "ssh_server.hpp"
#include "player.hpp"
#include <iostream>
#include <cstring>
#include <openssl/evp.h>
#include <openssl/rand.h>
#include <iomanip>
#include <sstream>

namespace dungeon_merc {

// SSHConnection implementation
SSHConnection::SSHConnection(int socket_fd, const std::string& client_ip)
    : socket_fd_(socket_fd)
    , client_ip_(client_ip)
    , state_(SSHConnectionState::CONNECTING)
    , session_(nullptr)
    , channel_(nullptr)
    , receive_buffer_(SSH_BUFFER_SIZE) {

    LOG_INFO("New SSH connection from " + client_ip_);
}

SSHConnection::~SSHConnection() {
    close();
}

bool SSHConnection::initialize() {
    if (!set_nonblocking()) {
        LOG_ERROR("Failed to set socket non-blocking");
        return false;
    }

    if (!setup_ssh_session()) {
        LOG_ERROR("Failed to setup SSH session");
        return false;
    }

    state_ = SSHConnectionState::AUTHENTICATING;
    return true;
}

void SSHConnection::close() {
    if (state_ == SSHConnectionState::DISCONNECTED) {
        return;
    }

    LOG_INFO("Closing SSH connection from " + client_ip_);

    cleanup_ssh();

    if (socket_fd_ >= 0) {
        ::close(socket_fd_);
        socket_fd_ = -1;
    }

    state_ = SSHConnectionState::DISCONNECTED;
}

bool SSHConnection::is_connected() const {
    return state_ != SSHConnectionState::DISCONNECTED && socket_fd_ >= 0;
}

SSHAuthResult SSHConnection::authenticate(const std::string& username, const std::string& password) {
    if (state_ != SSHConnectionState::AUTHENTICATING) {
        return SSHAuthResult::AUTH_ERROR;
    }

    if (username.empty() || username.length() > MAX_USERNAME_LENGTH) {
        return SSHAuthResult::INVALID_USERNAME;
    }

    if (password.empty() || password.length() > MAX_PASSWORD_LENGTH) {
        return SSHAuthResult::INVALID_PASSWORD;
    }

    // Try password authentication
    int auth_result = libssh2_userauth_password(session_, username.c_str(), password.c_str());

    if (auth_result == 0) {
        username_ = username;
        state_ = SSHConnectionState::AUTHENTICATED;
        LOG_INFO("SSH authentication successful for user: " + username);
        return SSHAuthResult::SUCCESS;
    } else {
        LOG_WARNING("SSH authentication failed for user: " + username);
        return SSHAuthResult::INVALID_PASSWORD;
    }
}

bool SSHConnection::is_authenticated() const {
    return state_ == SSHConnectionState::AUTHENTICATED || state_ == SSHConnectionState::PLAYING;
}

bool SSHConnection::send_message(const std::string& message) {
    if (!is_authenticated() || !channel_) {
        return false;
    }

    std::string formatted_message = message + "\r\n";
    ssize_t written = libssh2_channel_write(channel_, formatted_message.c_str(), formatted_message.length());

    if (written < 0) {
        LOG_ERROR("Failed to send message to SSH client: " + std::to_string(written));
        return false;
    }

    return true;
}

std::string SSHConnection::receive_message() {
    if (!is_authenticated() || !channel_) {
        return "";
    }

    char buffer[SSH_BUFFER_SIZE];
    ssize_t bytes_read = libssh2_channel_read(channel_, buffer, sizeof(buffer) - 1);

    if (bytes_read > 0) {
        buffer[bytes_read] = '\0';
        std::string message(buffer);

        // Remove carriage return and line feed
        message.erase(std::remove(message.begin(), message.end(), '\r'), message.end());
        message.erase(std::remove(message.begin(), message.end(), '\n'), message.end());

        return message;
    }

    return "";
}

bool SSHConnection::has_data() const {
    if (!is_authenticated() || !channel_) {
        return false;
    }
    
    // Check if channel has data available
    char buffer[1];
    ssize_t result = libssh2_channel_read(channel_, buffer, 1);
    if (result > 0) {
        // Put the byte back
        libssh2_channel_write(channel_, buffer, 1);
        return true;
    }
    
    return false;
}

void SSHConnection::set_player(std::shared_ptr<Player> player) {
    player_ = player;
    if (player) {
        state_ = SSHConnectionState::PLAYING;
    }
}

std::shared_ptr<Player> SSHConnection::get_player() const {
    return player_;
}

bool SSHConnection::setup_ssh_session() {
    session_ = libssh2_session_init();
    if (!session_) {
        LOG_ERROR("Failed to initialize SSH session");
        return false;
    }

    // Set session to non-blocking
    libssh2_session_set_blocking(session_, 0);

    // Perform SSH handshake
    int handshake_result;
    while ((handshake_result = libssh2_session_handshake(session_, socket_fd_)) == LIBSSH2_ERROR_EAGAIN) {
        // Wait for handshake to complete
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }

    if (handshake_result != 0) {
        LOG_ERROR("SSH handshake failed: " + std::to_string(handshake_result));
        return false;
    }

    return create_channel();
}

bool SSHConnection::create_channel() {
    // Request a shell channel
    while ((channel_ = libssh2_channel_open_session(session_)) == nullptr &&
           libssh2_session_last_error(session_, nullptr, nullptr, 0) == LIBSSH2_ERROR_EAGAIN) {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }

    if (!channel_) {
        LOG_ERROR("Failed to create SSH channel");
        return false;
    }

    // Request a pseudo-terminal
    int pty_result = libssh2_channel_request_pty(channel_, "xterm");
    if (pty_result != 0) {
        LOG_WARNING("Failed to request PTY: " + std::to_string(pty_result));
    }

    // Request a shell
    int shell_result = libssh2_channel_shell(channel_);
    if (shell_result != 0) {
        LOG_ERROR("Failed to request shell: " + std::to_string(shell_result));
        return false;
    }

    return true;
}

void SSHConnection::cleanup_ssh() {
    if (channel_) {
        libssh2_channel_free(channel_);
        channel_ = nullptr;
    }

    if (session_) {
        libssh2_session_disconnect(session_, "Normal Shutdown");
        libssh2_session_free(session_);
        session_ = nullptr;
    }
}

bool SSHConnection::set_nonblocking() {
    int flags = fcntl(socket_fd_, F_GETFL, 0);
    if (flags < 0) {
        return false;
    }

    return fcntl(socket_fd_, F_SETFL, flags | O_NONBLOCK) == 0;
}

// SSHServer implementation
SSHServer::SSHServer(int port)
    : port_(port)
    , server_socket_(-1)
    , running_(false) {

    // Initialize libssh2
    int result = libssh2_init(0);
    if (result != 0) {
        LOG_ERROR("Failed to initialize libssh2");
        throw std::runtime_error("libssh2 initialization failed");
    }

    LOG_INFO("SSH Server initialized on port " + std::to_string(port_));
}

SSHServer::~SSHServer() {
    shutdown();
    libssh2_exit();
}

bool SSHServer::initialize() {
    if (!create_server_socket()) {
        LOG_ERROR("Failed to create server socket");
        return false;
    }

    if (!set_socket_options()) {
        LOG_ERROR("Failed to set socket options");
        return false;
    }

    running_ = true;
    LOG_INFO("SSH Server started on port " + std::to_string(port_));
    return true;
}

void SSHServer::shutdown() {
    if (!running_) {
        return;
    }

    LOG_INFO("Shutting down SSH server...");

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

    LOG_INFO("SSH Server shutdown complete");
}

bool SSHServer::is_running() const {
    return running_;
}

void SSHServer::accept_connections() {
    if (!running_) {
        return;
    }

    struct sockaddr_in client_addr;
    socklen_t client_len = sizeof(client_addr);

    int client_socket = accept(server_socket_, (struct sockaddr*)&client_addr, &client_len);
    if (client_socket >= 0) {
        std::string client_ip = inet_ntoa(client_addr.sin_addr);

        auto connection = std::make_shared<SSHConnection>(client_socket, client_ip);
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

void SSHServer::process_connections() {
    std::lock_guard<std::mutex> lock(connections_mutex_);

    for (auto& connection : connections_) {
        if (!connection->is_connected()) {
            continue;
        }

        // Process incoming messages
        if (connection->has_data()) {
            std::string message = connection->receive_message();
            if (!message.empty()) {
                // Handle the message
                if (connection->get_state() == SSHConnectionState::AUTHENTICATED) {
                    // Send welcome message
                    connection->send_message("Welcome to Dungeon Merc!");
                    connection->send_message("Type 'help' for available commands.");
                }
            }
        }
    }
}

void SSHServer::remove_disconnected_connections() {
    std::lock_guard<std::mutex> lock(connections_mutex_);

    connections_.erase(
        std::remove_if(connections_.begin(), connections_.end(),
            [this](const std::shared_ptr<SSHConnection>& conn) {
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

bool SSHServer::add_user(const std::string& username, const std::string& password_hash) {
    std::lock_guard<std::mutex> lock(users_mutex_);
    users_[username] = password_hash;
    LOG_INFO("Added user: " + username);
    return true;
}

bool SSHServer::remove_user(const std::string& username) {
    std::lock_guard<std::mutex> lock(users_mutex_);
    auto it = users_.find(username);
    if (it != users_.end()) {
        users_.erase(it);
        LOG_INFO("Removed user: " + username);
        return true;
    }
    return false;
}

bool SSHServer::validate_credentials(const std::string& username, const std::string& password) {
    std::lock_guard<std::mutex> lock(users_mutex_);

    auto it = users_.find(username);
    if (it == users_.end()) {
        return false;
    }

    return verify_password(password, it->second);
}

bool SSHServer::create_server_socket() {
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

bool SSHServer::set_socket_options() {
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

std::string SSHServer::hash_password(const std::string& password) {
    return ssh_utils::hash_password(password);
}

bool SSHServer::verify_password(const std::string& password, const std::string& hash) {
    return ssh_utils::verify_password(password, hash);
}

// SSH utilities implementation
namespace ssh_utils {

std::string hash_password(const std::string& password) {
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

bool verify_password(const std::string& password, const std::string& hash) {
    std::string password_hash = hash_password(password);
    return password_hash == hash;
}

bool generate_server_key(const std::string& key_path) {
    (void)key_path; // Suppress unused parameter warning
    // TODO: Implement server key generation
    LOG_WARNING("Server key generation not implemented yet");
    return false;
}

bool load_server_key(const std::string& key_path) {
    (void)key_path; // Suppress unused parameter warning
    // TODO: Implement server key loading
    LOG_WARNING("Server key loading not implemented yet");
    return false;
}

std::string get_client_ip(int socket_fd) {
    struct sockaddr_in addr;
    socklen_t addr_size = sizeof(struct sockaddr_in);
    int res = getpeername(socket_fd, (struct sockaddr *)&addr, &addr_size);
    if (res == 0) {
        return inet_ntoa(addr.sin_addr);
    }
    return "unknown";
}

bool set_socket_nonblocking(int socket_fd) {
    int flags = fcntl(socket_fd, F_GETFL, 0);
    if (flags < 0) {
        return false;
    }
    return fcntl(socket_fd, F_SETFL, flags | O_NONBLOCK) == 0;
}

bool set_socket_reuseaddr(int socket_fd) {
    int opt = 1;
    return setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == 0;
}

} // namespace ssh_utils

} // namespace dungeon_merc
