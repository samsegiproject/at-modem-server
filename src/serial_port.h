#pragma once

#include <string>
#include <termios.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdexcept>
#include <cstring>
#include <cerrno>
#include <iostream>

class SerialPort {
public:
    struct Config {
        int baud = 115200;
        int dataBits = 8;
        char parity = 'N'; // 'N','E','O'
        int stopBits = 1;
        bool hwFlow = false;
    };

    SerialPort();
    ~SerialPort();

    bool open(const std::string& device);
    bool openFd(int fd);
    void close();

    void setConfig(const Config& cfg) { config_ = cfg; }
    const Config& getConfig() const { return config_; }

    bool setRawMode();
    bool restoreMode();

    ssize_t read(char* buffer, size_t size);
    ssize_t write(const char* buffer, size_t size);

    int getFd() const { return fd_; }
    bool isOpen() const { return fd_ != -1; }

private:
    int fd_;
    struct termios old_termios_;
    bool mode_set_;
    bool is_master_pty_;
    Config config_;
};