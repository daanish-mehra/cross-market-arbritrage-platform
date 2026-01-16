#include "websocket_server.h"
#include <iostream>
#include <sstream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#include <pthread.h>
#include <cstring>
#include <algorithm>
#include <vector>
#include <map>
#include <cstdint>

static const char base64_chars[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

static std::string base64_encode(const unsigned char* data, size_t length) {
    std::string result;
    int i = 0;
    int j = 0;
    unsigned char char_array_3[3];
    unsigned char char_array_4[4];
    
    while (length--) {
        char_array_3[i++] = *(data++);
        if (i == 3) {
            char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
            char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
            char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
            char_array_4[3] = char_array_3[2] & 0x3f;
            
            for (i = 0; i < 4; i++) {
                result += base64_chars[char_array_4[i]];
            }
            i = 0;
        }
    }
    
    if (i) {
        for (j = i; j < 3; j++) {
            char_array_3[j] = '\0';
        }
        
        char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
        char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
        char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
        char_array_4[3] = char_array_3[2] & 0x3f;
        
        for (j = 0; j < i + 1; j++) {
            result += base64_chars[char_array_4[j]];
        }
        
        while (i++ < 3) {
            result += '=';
        }
    }
    
    return result;
}

static void sha1(const unsigned char* data, size_t length, unsigned char* hash) {
    uint32_t h[5] = {0x67452301, 0xEFCDAB89, 0x98BADCFE, 0x10325476, 0xC3D2E1F0};
    
    size_t orig_len = length;
    size_t new_len = ((length + 9) / 64) * 64 + 64;
    unsigned char* msg = new unsigned char[new_len];
    memcpy(msg, data, length);
    msg[length] = 0x80;
    memset(msg + length + 1, 0, new_len - length - 1);
    
    uint64_t bit_len = orig_len * 8;
    for (int i = 0; i < 8; i++) {
        msg[new_len - 8 + i] = (bit_len >> (56 - i * 8)) & 0xFF;
    }
    
    for (size_t chunk = 0; chunk < new_len; chunk += 64) {
        uint32_t w[80];
        for (int i = 0; i < 16; i++) {
            w[i] = (msg[chunk + i * 4] << 24) | (msg[chunk + i * 4 + 1] << 16) |
                   (msg[chunk + i * 4 + 2] << 8) | msg[chunk + i * 4 + 3];
        }
        for (int i = 16; i < 80; i++) {
            w[i] = ((w[i-3] ^ w[i-8] ^ w[i-14] ^ w[i-16]) << 1) |
                   ((w[i-3] ^ w[i-8] ^ w[i-14] ^ w[i-16]) >> 31);
        }
        
        uint32_t a = h[0], b = h[1], c = h[2], d = h[3], e = h[4];
        
        for (int i = 0; i < 80; i++) {
            uint32_t f, k;
            if (i < 20) {
                f = (b & c) | ((~b) & d);
                k = 0x5A827999;
            } else if (i < 40) {
                f = b ^ c ^ d;
                k = 0x6ED9EBA1;
            } else if (i < 60) {
                f = (b & c) | (b & d) | (c & d);
                k = 0x8F1BBCDC;
            } else {
                f = b ^ c ^ d;
                k = 0xCA62C1D6;
            }
            
            uint32_t temp = ((a << 5) | (a >> 27)) + f + e + k + w[i];
            e = d;
            d = c;
            c = ((b << 30) | (b >> 2));
            b = a;
            a = temp;
        }
        
        h[0] += a;
        h[1] += b;
        h[2] += c;
        h[3] += d;
        h[4] += e;
    }
    
    for (int i = 0; i < 5; i++) {
        hash[i * 4] = (h[i] >> 24) & 0xFF;
        hash[i * 4 + 1] = (h[i] >> 16) & 0xFF;
        hash[i * 4 + 2] = (h[i] >> 8) & 0xFF;
        hash[i * 4 + 3] = h[i] & 0xFF;
    }
    
    delete[] msg;
}

struct ClientInfo {
    int fd;
    bool websocket;
};

static std::vector<int> clients;
static pthread_mutex_t clients_mutex = PTHREAD_MUTEX_INITIALIZER;

void* server_thread_func(void* arg) {
    WebSocketServer* server = (WebSocketServer*)arg;
    server->server_loop();
    return NULL;
}

WebSocketServer::WebSocketServer(int port) {
    this->port = port;
    this->server_fd = -1;
    this->running = false;
    this->server_thread = NULL;
}

WebSocketServer::~WebSocketServer() {
    stop();
}

bool WebSocketServer::start() {
    if (running) {
        return true;
    }
    
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) {
        std::cerr << "Failed to create socket" << std::endl;
        return false;
    }
    
    int opt = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    
    struct sockaddr_in address;
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(port);
    
    if (bind(server_fd, (struct sockaddr*)&address, sizeof(address)) < 0) {
        std::cerr << "Failed to bind to port " << port << std::endl;
        close(server_fd);
        server_fd = -1;
        return false;
    }
    
    if (listen(server_fd, 10) < 0) {
        std::cerr << "Failed to listen" << std::endl;
        close(server_fd);
        server_fd = -1;
        return false;
    }
    
    running = true;
    
    pthread_t* thread = new pthread_t;
    if (pthread_create(thread, NULL, server_thread_func, this) != 0) {
        std::cerr << "Failed to start server thread" << std::endl;
        running = false;
        close(server_fd);
        server_fd = -1;
        delete thread;
        return false;
    }
    
    server_thread = thread;
    std::cout << "WebSocket server started on port " << port << std::endl;
    return true;
}

void WebSocketServer::stop() {
    if (!running) {
        return;
    }
    
    running = false;
    
    if (server_fd >= 0) {
        close(server_fd);
        server_fd = -1;
    }
    
    if (server_thread != NULL) {
        pthread_join(*(pthread_t*)server_thread, NULL);
        delete (pthread_t*)server_thread;
        server_thread = NULL;
    }
}

bool WebSocketServer::is_running() {
    return running;
}

void WebSocketServer::server_loop() {
    while (running) {
        struct sockaddr_in client_addr;
        socklen_t client_len = sizeof(client_addr);
        
        fd_set read_fds;
        FD_ZERO(&read_fds);
        FD_SET(server_fd, &read_fds);
        
        struct timeval timeout;
        timeout.tv_sec = 1;
        timeout.tv_usec = 0;
        
        int activity = select(server_fd + 1, &read_fds, NULL, NULL, &timeout);
        
        if (activity > 0 && FD_ISSET(server_fd, &read_fds)) {
            int client_fd = accept(server_fd, (struct sockaddr*)&client_addr, &client_len);
            if (client_fd >= 0) {
                struct ThreadClientData {
                    WebSocketServer* server;
                    int fd;
                };
                
                ThreadClientData* thread_data = new ThreadClientData;
                thread_data->server = this;
                thread_data->fd = client_fd;
                
                pthread_t client_thread;
                pthread_create(&client_thread, NULL, [](void* arg) -> void* {
                    ThreadClientData* td = (ThreadClientData*)arg;
                    td->server->handle_client(td->fd);
                    delete td;
                    return NULL;
                }, thread_data);
                pthread_detach(client_thread);
            }
        }
    }
}

void WebSocketServer::handle_client(int client_fd) {
    char buffer[4096];
    std::string request;
    
    int bytes_read = recv(client_fd, buffer, sizeof(buffer) - 1, 0);
    if (bytes_read <= 0) {
        close(client_fd);
        return;
    }
    
    buffer[bytes_read] = '\0';
    request = buffer;
    
    if (request.find("Upgrade: websocket") != std::string::npos ||
        request.find("upgrade: websocket") != std::string::npos) {
        if (handle_websocket_upgrade(client_fd, request)) {
            if (on_connect) {
                on_connect(client_fd);
            }
            
            pthread_mutex_lock(&clients_mutex);
            clients.push_back(client_fd);
            pthread_mutex_unlock(&clients_mutex);
            
            while (running) {
                fd_set read_fds;
                FD_ZERO(&read_fds);
                FD_SET(client_fd, &read_fds);
                
                struct timeval timeout;
                timeout.tv_sec = 1;
                timeout.tv_usec = 0;
                
                int activity = select(client_fd + 1, &read_fds, NULL, NULL, &timeout);
                
                if (activity > 0 && FD_ISSET(client_fd, &read_fds)) {
                    bytes_read = recv(client_fd, buffer, sizeof(buffer), 0);
                    if (bytes_read <= 0) {
                        break;
                    }
                    
                    // Parse WebSocket frame
                    if (bytes_read >= 2) {
                        unsigned char first_byte = buffer[0];
                        unsigned char second_byte = buffer[1];
                        unsigned char opcode = first_byte & 0x0F;
                        bool masked = (second_byte & 0x80) != 0;
                        size_t payload_len = second_byte & 0x7F;
                        
                        size_t mask_offset = 2;
                        if (payload_len == 126) {
                            if (bytes_read < 4) continue;
                            payload_len = ((unsigned char)buffer[2] << 8) | (unsigned char)buffer[3];
                            mask_offset = 4;
                        } else if (payload_len == 127) {
                            if (bytes_read < 10) continue;
                            // Skip 64-bit length for simplicity
                            mask_offset = 10;
                            payload_len = 0; // Too large, skip
                        }
                        
                        if (masked && bytes_read >= mask_offset + 4) {
                            unsigned char mask[4];
                            memcpy(mask, buffer + mask_offset, 4);
                            size_t payload_offset = mask_offset + 4;
                            
                            if (bytes_read >= payload_offset + payload_len && payload_len > 0 && payload_len < 4096) {
                                std::string payload;
                                payload.resize(payload_len);
                                
                                for (size_t i = 0; i < payload_len; i++) {
                                    payload[i] = buffer[payload_offset + i] ^ mask[i % 4];
                                }
                                
                                // Handle ping message (opcode 0x9 or JSON ping)
                                if (opcode == 0x9 || (opcode == 0x1 && payload.find("\"type\":\"ping\"") != std::string::npos)) {
                                    // Send pong response
                                    if (opcode == 0x9) {
                                        // WebSocket ping frame - send pong frame (opcode 0xA)
                                        unsigned char pong_frame[10];
                                        size_t pong_frame_len = 2;
                                        pong_frame[0] = 0x8A; // FIN + Pong opcode
                                        pong_frame[1] = payload_len;
                                        if (payload_len < 126) {
                                            pong_frame[1] = payload_len;
                                            pong_frame_len = 2;
                                        } else if (payload_len < 65536) {
                                            pong_frame[1] = 126;
                                            pong_frame[2] = (payload_len >> 8) & 0xFF;
                                            pong_frame[3] = payload_len & 0xFF;
                                            pong_frame_len = 4;
                                        }
                                        send(client_fd, pong_frame, pong_frame_len, 0);
                                        send(client_fd, payload.c_str(), payload_len, 0);
                                    } else {
                                        // JSON ping - respond with JSON pong
                                        std::string pong_msg = "{\"type\":\"pong\"";
                                        size_t ts_pos = payload.find("\"timestamp\":");
                                        if (ts_pos != std::string::npos) {
                                            size_t ts_start = payload.find(":", ts_pos) + 1;
                                            size_t ts_end = payload.find_first_of(",}", ts_start);
                                            if (ts_end != std::string::npos) {
                                                std::string timestamp = payload.substr(ts_start, ts_end - ts_start);
                                                pong_msg += ",\"timestamp\":" + timestamp;
                                            }
                                        }
                                        pong_msg += "}";
                                        send_message(client_fd, pong_msg);
                                    }
                                }
                            }
                        }
                    }
                } else if (activity < 0) {
                    break;
                }
            }
            
            pthread_mutex_lock(&clients_mutex);
            clients.erase(std::remove(clients.begin(), clients.end(), client_fd), clients.end());
            pthread_mutex_unlock(&clients_mutex);
            
            if (on_disconnect) {
                on_disconnect(client_fd);
            }
        }
    } else {
        std::string response = "HTTP/1.1 200 OK\r\n"
                              "Content-Type: application/json\r\n"
                              "Access-Control-Allow-Origin: *\r\n"
                              "\r\n"
                              "{\"status\":\"ok\"}";
        send(client_fd, response.c_str(), response.length(), 0);
    }
    
    close(client_fd);
}

bool WebSocketServer::handle_websocket_upgrade(int client_fd, const std::string& request) {
    std::string key;
    size_t key_pos = request.find("Sec-WebSocket-Key:");
    if (key_pos == std::string::npos) {
        key_pos = request.find("sec-websocket-key:");
    }
    
    if (key_pos != std::string::npos) {
        size_t key_start = request.find(" ", key_pos) + 1;
        size_t key_end = request.find("\r\n", key_start);
        if (key_end != std::string::npos) {
            key = request.substr(key_start, key_end - key_start);
        }
    }
    
    if (key.empty()) {
        return false;
    }
    
    const std::string magic = "258EAFA5-E914-47DA-95CA-C5AB0DC85B11";
    std::string accept_key = key + magic;
    
    unsigned char hash[20];
    sha1((unsigned char*)accept_key.c_str(), accept_key.length(), hash);
    
    std::string base64 = base64_encode(hash, 20);
    
    std::string response = "HTTP/1.1 101 Switching Protocols\r\n"
                          "Upgrade: websocket\r\n"
                          "Connection: Upgrade\r\n"
                          "Sec-WebSocket-Accept: " + base64 + "\r\n"
                          "\r\n";
    
    send(client_fd, response.c_str(), response.length(), 0);
    return true;
}

void WebSocketServer::send_message(int client_fd, const std::string& message) {
    unsigned char frame[10];
    size_t frame_len = 2;
    size_t msg_len = message.length();
    
    frame[0] = 0x81;
    
    if (msg_len < 126) {
        frame[1] = msg_len;
    } else if (msg_len < 65536) {
        frame[1] = 126;
        frame[2] = (msg_len >> 8) & 0xFF;
        frame[3] = msg_len & 0xFF;
        frame_len = 4;
    } else {
        frame[1] = 127;
        for (int i = 0; i < 8; i++) {
            frame[2 + i] = (msg_len >> (56 - i * 8)) & 0xFF;
        }
        frame_len = 10;
    }
    
    send(client_fd, frame, frame_len, 0);
    send(client_fd, message.c_str(), msg_len, 0);
}

std::string WebSocketServer::create_opportunity_json(ArbitrageOpportunity* opp) {
    std::ostringstream oss;
    oss << "{\"type\":\"opportunity\",\"data\":{"
        << "\"event_id\":\"" << opp->event_id << "\","
        << "\"buy_market\":" << opp->buy_market << ","
        << "\"sell_market\":" << opp->sell_market << ","
        << "\"buy_price\":" << opp->buy_price << ","
        << "\"sell_price\":" << opp->sell_price << ","
        << "\"profit_percentage\":" << (opp->profit_percentage * 100.0) << ","
        << "\"max_size\":" << opp->max_size
        << "}}";
    return oss.str();
}

std::string WebSocketServer::create_market_data_json(MarketData* data) {
    std::ostringstream oss;
    oss << "{\"type\":\"market_data\",\"data\":{"
        << "\"market_id\":\"" << data->market_id << "\","
        << "\"market\":" << data->market << ","
        << "\"event_name\":\"" << data->event_name << "\","
        << "\"best_bid\":" << data->best_bid << ","
        << "\"best_ask\":" << data->best_ask << ","
        << "\"bid_size\":" << data->bid_size << ","
        << "\"ask_size\":" << data->ask_size
        << "}}";
    return oss.str();
}

void WebSocketServer::broadcast_opportunity(ArbitrageOpportunity* opp) {
    std::string message = create_opportunity_json(opp);
    
    pthread_mutex_lock(&clients_mutex);
    for (int fd : clients) {
        send_message(fd, message);
    }
    pthread_mutex_unlock(&clients_mutex);
}

void WebSocketServer::broadcast_market_data(MarketData* data) {
    std::string message = create_market_data_json(data);
    
    pthread_mutex_lock(&clients_mutex);
    for (int fd : clients) {
        send_message(fd, message);
    }
    pthread_mutex_unlock(&clients_mutex);
}

void WebSocketServer::set_on_connect(std::function<void(int)> callback) {
    on_connect = callback;
}

void WebSocketServer::set_on_disconnect(std::function<void(int)> callback) {
    on_disconnect = callback;
}

