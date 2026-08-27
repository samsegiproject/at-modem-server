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
              << "  -h, --help              Show this help\n";
}

int main(int argc, char* argv[]) {
    std::vector<std::string> devices;
    bool usePty = false;
    std::string dictFile = "dictionary.txt";

    static struct option long_options[] = {
        {"device", required_argument, 0, 'd'},
        {"pty", no_argument, 0, 'p'},
        {"dict", required_argument, 0, 'f'},
        {"help", no_argument, 0, 'h'},
        {0, 0, 0, 0}
    };

    int opt;
    while ((opt = getopt_long(argc, argv, "d:pf:h", long_options, nullptr)) != -1) {
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

    signal(SIGINT, [](int) { if (Server::instance) Server::instance->stop(); });
    signal(SIGTERM, [](int) { if (Server::instance) Server::instance->stop(); });
    signal(SIGHUP, [](int) { if (Server::instance) Server::instance->stop(); });

    try {
        Server server(devices, dictFile, usePty);
        server.run();
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
