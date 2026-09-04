#include "serial_port.h"

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
        }
        else {
            std::cerr << "tcgetattr failed: " << std::strerror(errno) << std::endl;
            return false;
        }
    }

    old_termios_ = tty;
    mode_set_ = true;

    cfmakeraw(&tty);

    // data bits
    tty.c_cflag &= ~CSIZE;
    switch (config_.dataBits) {
    case 5: tty.c_cflag |= CS5; break;
    case 6: tty.c_cflag |= CS6; break;
    case 7: tty.c_cflag |= CS7; break;
    case 8: default: tty.c_cflag |= CS8; break;
    }

    // parity
    if (config_.parity == 'E') {
        tty.c_cflag |= PARENB;
        tty.c_cflag &= ~PARODD;
    }
    else if (config_.parity == 'O') {
        tty.c_cflag |= PARENB | PARODD;
    }
    else {
        tty.c_cflag &= ~PARENB;
    }

    // stop bits
    if (config_.stopBits == 2) {
        tty.c_cflag |= CSTOPB;
    }
    else {
        tty.c_cflag &= ~CSTOPB;
    }

    // hardware flow control
    if (config_.hwFlow) {
        tty.c_cflag |= CRTSCTS;
    }
    else {
        tty.c_cflag &= ~CRTSCTS;
    }

    // software flow control off
    tty.c_iflag &= ~(IXON | IXOFF | IXANY);

    // baud rate
    speed_t speed = B115200;
    switch (config_.baud) {
    case 9600: speed = B9600; break;
    case 19200: speed = B19200; break;
    case 38400: speed = B38400; break;
    case 57600: speed = B57600; break;
    case 115200: speed = B115200; break;
    default: speed = B115200; break;
    }
    cfsetispeed(&tty, speed);
    cfsetospeed(&tty, speed);

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