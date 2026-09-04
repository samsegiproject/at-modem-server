#pragma once

#include <string>
#include <vector>
#include <thread>
#include <atomic>
#include <memory>
#include <csignal>
#include "serial_port.h"
#include "dictionary.h"

class Server {
public:
    Server(const std::vector<std::string>& devices,
        const std::string& dictFile,
        const SerialPort::Config& portConfig,
        bool usePty = false);
    ~Server();

    void run();
    void stop();

    static Server* instance;

private:
    void processDevice(const std::string& device);
    void processFd(int fd, const std::string& description);
    void clientLoop(int fd, const std::string& deviceName);
    bool sendAnswer(int fd, const std::string& answer);

    std::vector<std::thread> threads_;
    std::atomic<bool> running_{ false };
    Dictionary dict_;
    bool usePty_;
    std::vector<std::string> devices_;
    std::vector<int> pty_master_fds_;
    std::vector<int> pty_slave_fds_;
    std::vector<std::string> pty_slave_paths_;
    SerialPort::Config portConfig_;
};

