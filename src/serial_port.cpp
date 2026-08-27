#include "serial_port.h"
#include <cstring>
#include <cerrno>
#include <iostream>

SerialPort::SerialPort() : fd_(-1), mode_set_(false), is_master_pty_(false) {
    std::memset(&old_termios_, 0, sizeof(old_termios_));
}

SerialPort::~SerialPort() {
    if (fd_ != -1) {
        close();
    }
}

bool SerialPort::open(const std::string& device) {
    if (fd_ != -1) return false;
    fd_ = ::open(device.c_str(), O_RDWR | O_NOCTTY);
    if (fd_ == -1) {
        std::cerr << "Failed to open " << device << ": " << std::strerror(errno) << std::endl;
        return false;
    }
    is_master_pty_ = false;
    return setRawMode();
}

bool SerialPort::openFd(int fd) {
    if (fd_ != -1) return false;
    fd_ = fd;
    is_master_pty_ = true;
    return setRawMode();
}

void SerialPort::close() {
    if (fd_ != -1) {
        restoreMode();
        ::close(fd_);
        fd_ = -1;
        mode_set_ = false;
        is_master_pty_ = false;
    }
}

bool SerialPort::setRawMode() {
    if (fd_ == -1) return false;

    struct termios tty;
    if (tcgetattr(fd_, &tty) != 0) {
        if (is_master_pty_) {
            std::cerr << "tcgetattr failed (pty master?): " << std::strerror(errno) << std::endl;
            return true;
        } else {
            std::cerr << "tcgetattr failed: " << std::strerror(errno) << std::endl;
            return false;
        }
    }

    old_termios_ = tty;
    mode_set_ = true;

    cfmakeraw(&tty);

    tty.c_cflag &= ~CRTSCTS;
    tty.c_iflag &= ~(IXON | IXOFF | IXANY);

    tty.c_cc[VMIN] = 0;
    tty.c_cc[VTIME] = 1;

    if (tcsetattr(fd_, TCSANOW, &tty) != 0) {
        if (!is_master_pty_) {
            std::cerr << "tcsetattr failed: " << std::strerror(errno) << std::endl;
            return false;
        }
    }
    return true;
}

bool SerialPort::restoreMode() {
    if (fd_ == -1 || !mode_set_) return false;
    if (tcsetattr(fd_, TCSANOW, &old_termios_) != 0) {
        std::cerr << "Failed to restore termios: " << std::strerror(errno) << std::endl;
        return false;
    }
    mode_set_ = false;
    return true;
}

ssize_t SerialPort::read(char* buffer, size_t size) {
    if (fd_ == -1) return -1;
    return ::read(fd_, buffer, size);
}

ssize_t SerialPort::write(const char* buffer, size_t size) {
    if (fd_ == -1) return -1;
    return ::write(fd_, buffer, size);
}
