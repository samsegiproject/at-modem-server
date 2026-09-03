#include <sys/socket.h>
#include "server.h"
#include "utils.h"
#include <iostream>
#include <cstring>
#include <cerrno>
#include <poll.h>
#include <unistd.h>
#include <fcntl.h>
#include <pty.h>
#include <termios.h>

Server* Server::instance = nullptr;

Server::Server(const std::vector<std::string>& devices,
    const std::string& dictFile,
    bool usePty)
    : running_(false), usePty_(usePty), devices_(devices) {
    instance = this;

    if (!dict_.loadFromFile(dictFile)) {
        throw std::runtime_error("Failed to load dictionary file: " + dictFile);
    }

    if (usePty_) {
        int master, slave;
        char slave_name[256];
        if (openpty(&master, &slave, slave_name, nullptr, nullptr) != 0) {
            throw std::runtime_error("openpty failed");
        }

        // Сохраняем slave-дескриптор, чтобы pty не закрылся
        pty_slave_fds_.push_back(slave);
        pty_master_fds_.push_back(master);
        pty_slave_paths_.push_back(slave_name);

        // Настраиваем slave в raw mode, чтобы отключить эхо и обработку сигналов
        struct termios tty;
        if (tcgetattr(slave, &tty) == 0) {
            cfmakeraw(&tty);
            tty.c_cflag &= ~CRTSCTS;
            tty.c_iflag &= ~(IXON | IXOFF | IXANY);
            tty.c_cc[VMIN] = 0;
            tty.c_cc[VTIME] = 1;
            tcsetattr(slave, TCSANOW, &tty);
        }

        std::cout << "PTY slave device: " << slave_name << std::endl;
    }
}

Server::~Server() {
    stop();
    for (auto& t : threads_) {
        if (t.joinable()) t.join();
    }
    instance = nullptr;
}

void Server::run() {
    running_ = true;

    if (usePty_) {
        for (size_t i = 0; i < pty_master_fds_.size(); ++i) {
            int fd = pty_master_fds_[i];
            std::string desc = "pty master (slave: " + pty_slave_paths_[i] + ")";
            threads_.emplace_back(&Server::processFd, this, fd, desc);
        }
    }
    else {
        for (const auto& dev : devices_) {
            threads_.emplace_back(&Server::processDevice, this, dev);
        }
    }

    for (auto& t : threads_) {
        if (t.joinable()) t.join();
    }
}

void Server::stop() {
    running_ = false;
    for (int fd : pty_slave_fds_) {
        ::close(fd);
    }
    pty_slave_fds_.clear();

    for (int fd : pty_master_fds_) {
        ::shutdown(fd, SHUT_RDWR);
    }
}

void Server::processDevice(const std::string& device) {
    SerialPort port;
    if (!port.open(device)) {
        std::cerr << "Failed to open device: " << device << std::endl;
        return;
    }
    std::cout << "Opened device: " << device << std::endl;
    clientLoop(port.getFd(), device);
}

void Server::processFd(int fd, const std::string& description) {
    std::cout << "Processing fd " << fd << ": " << description << std::endl;
    clientLoop(fd, description);
}

void Server::clientLoop(int fd, const std::string& deviceName) {
    struct pollfd pfd;
    pfd.fd = fd;
    pfd.events = POLLIN;

    std::string buffer;
    char readBuf[256];

    while (running_) {
        int ret = poll(&pfd, 1, 100);
        if (ret < 0) {
            if (errno == EINTR) continue;
            std::cerr << "poll error on " << deviceName << ": " << std::strerror(errno) << std::endl;
            break;
        }
        if (ret == 0) continue;

        if (pfd.revents & POLLIN) {
            ssize_t n = ::read(fd, readBuf, sizeof(readBuf));
            if (n > 0) {
                buffer.append(readBuf, n);
                size_t pos;
                while ((pos = buffer.find_first_of("\r\n")) != std::string::npos) {
                    std::string command = buffer.substr(0, pos);
                    size_t next = pos;
                    while (next < buffer.size() &&
                        (buffer[next] == '\r' || buffer[next] == '\n')) {
                        ++next;
                    }
                    buffer.erase(0, next);

                    if (!command.empty()) {
                        std::cout << "[" << deviceName << "] Received: " << command << std::endl;
                        std::string answer;
                        if (dict_.find(command, answer)) {
                            sendAnswer(fd, answer);
                        }
                        else {
                            sendAnswer(fd, "ERROR");
                        }
                    }
                }
            }
            else if (n == 0) {
                std::cout << "[" << deviceName << "] EOF" << std::endl;
                break;
            }
            else {
                if (errno != EAGAIN && errno != EWOULDBLOCK && errno != EINTR) {
                    std::cerr << "Read error on " << deviceName << ": " << std::strerror(errno) << std::endl;
                    break;
                }
            }
        }

        if (pfd.revents & (POLLERR | POLLHUP | POLLNVAL)) {
            std::cerr << "[" << deviceName << "] Error/hangup" << std::endl;
            break;
        }
    }
}

bool Server::sendAnswer(int fd, const std::string& answer) {
    std::string toSend = answer;
    if (!utils::ends_with_newline(toSend)) {
        toSend += "\r\n";
    }
    std::cout << "Sending to fd " << fd << ": " << toSend << std::flush;
    ssize_t written = ::write(fd, toSend.c_str(), toSend.size());
    if (written < 0) {
        std::cerr << "Write error: " << std::strerror(errno) << std::endl;
        return false;
    }
    return written == static_cast<ssize_t>(toSend.size());
}