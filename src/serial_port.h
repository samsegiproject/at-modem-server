#pragma once

#include <string>
#include <termios.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdexcept>

class SerialPort {
public:
    SerialPort();
    ~SerialPort();

    bool open(const std::string& device);
    bool openFd(int fd);
    void close();

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
};
