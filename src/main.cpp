#include <iostream>
#include <string>
#include <vector>
#include <csignal>
#include <getopt.h>
#include <cstring>
#include "server.h"

void printUsage(const char* prog) {
    std::cout << "Usage: " << prog << " [options]\n"
        << "Options:\n"
        << "  -d, --device <tty>      Serial device (can be repeated)\n"
        << "  -p, --pty               Create pseudo-terminal pair\n"
        << "  -f, --dict <file>       Dictionary file (default: dictionary.txt)\n"
        << "  -b, --baud <rate>       Baud rate (default: 115200)\n"
        << "  -B, --data-bits <5|6|7|8> Data bits (default: 8)\n"
        << "  -P, --parity <none|even|odd> Parity (default: none)\n"
        << "  -S, --stop-bits <1|2>   Stop bits (default: 1)\n"
        << "  -F, --hw-flow           Enable hardware flow control\n"
        << "  -h, --help              Show this help\n";
}

int main(int argc, char* argv[]) {
    std::vector<std::string> devices;
    bool usePty = false;
    std::string dictFile = "dictionary.txt";

    // Serial port configuration with defaults
    SerialPort::Config portConfig;
    int baud = 115200;
    int dataBits = 8;
    std::string parity = "none";
    int stopBits = 1;
    bool hwFlow = false;

    static struct option long_options[] = {
        {"device", required_argument, 0, 'd'},
        {"pty", no_argument, 0, 'p'},
        {"dict", required_argument, 0, 'f'},
        {"baud", required_argument, 0, 'b'},
        {"data-bits", required_argument, 0, 'B'},
        {"parity", required_argument, 0, 'P'},
        {"stop-bits", required_argument, 0, 'S'},
        {"hw-flow", no_argument, 0, 'F'},
        {"help", no_argument, 0, 'h'},
        {0, 0, 0, 0}
    };

    int opt;
    while ((opt = getopt_long(argc, argv, "d:pf:b:B:P:S:Fh", long_options, nullptr)) != -1) {
        switch (opt) {
        case 'd':
            devices.push_back(optarg);
            break;
        case 'p':
            usePty = true;
            break;
        case 'f':
            dictFile = optarg;
            break;
        case 'b':
            baud = std::stoi(optarg);
            break;
        case 'B':
            dataBits = std::stoi(optarg);
            break;
        case 'P':
            parity = optarg;
            break;
        case 'S':
            stopBits = std::stoi(optarg);
            break;
        case 'F':
            hwFlow = true;
            break;
        case 'h':
            printUsage(argv[0]);
            return 0;
        default:
            printUsage(argv[0]);
            return 1;
        }
    }

    if (devices.empty() && !usePty) {
        std::cerr << "No device specified. Use -d <device> or --pty" << std::endl;
        printUsage(argv[0]);
        return 1;
    }

    // Apply parsed parameters to portConfig
    portConfig.baud = baud;
    portConfig.dataBits = dataBits;
    if (parity == "even") portConfig.parity = 'E';
    else if (parity == "odd") portConfig.parity = 'O';
    else portConfig.parity = 'N';
    portConfig.stopBits = stopBits;
    portConfig.hwFlow = hwFlow;

    signal(SIGINT, [](int) { if (Server::instance) Server::instance->stop(); });
    signal(SIGTERM, [](int) { if (Server::instance) Server::instance->stop(); });
    signal(SIGHUP, [](int) { if (Server::instance) Server::instance->stop(); });

    try {
        Server server(devices, dictFile, portConfig, usePty);
        server.run();
    }
    catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}