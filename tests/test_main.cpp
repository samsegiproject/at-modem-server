#include <cassert>
#include <iostream>
#include <fstream>
#include <cstdio>
#include "pattern.h"
#include "dictionary.h"

void testPattern() {
    Pattern p1("AT");
    assert(p1.match("AT"));
    assert(!p1.match("ATI"));
    assert(!p1.match("A"));

    Pattern p2("AT*");
    assert(p2.match("AT"));
    assert(p2.match("ATI"));
    assert(p2.match("AT+COPS"));
    assert(!p2.match("A"));

    Pattern p3("AT*I");
    assert(p3.match("ATI"));
    assert(p3.match("ATXI"));
    assert(p3.match("AT123I"));
    assert(!p3.match("AT"));

    Pattern p4("ATE[01]");
    assert(p4.match("ATE0"));
    assert(p4.match("ATE1"));
    assert(!p4.match("ATE2"));
    assert(!p4.match("ATE,"));

    Pattern p5("AT+COPS");
    assert(p5.match("AT+COPS"));
    assert(!p5.match("AT+COP"));

    Pattern p6("A.T");
    assert(p6.match("AAT"));
    assert(p6.match("ABT"));
    assert(!p6.match("AT"));

    Pattern p7("[a-z]T");
    assert(p7.match("aT"));
    assert(p7.match("zT"));
    assert(!p7.match("AT"));

    Pattern p8("[^0-9]T");
    assert(p8.match("AT"));
    assert(p8.match("aT"));
    assert(!p8.match("5T"));

    std::cout << "Pattern tests passed\n";
}

void testDictionary() {
    const std::string dictContent =
        "# Test dictionary\n"
        "AT=OK\n"
        "ATI=Model: Test\n"
        "ATE[01]=OK\n"
        "AT+COPS=+COPS: 0,0,\"Operator\"\n";

    std::string filename = "test_dict.txt";
    std::ofstream f(filename);
    f << dictContent;
    f.close();

    Dictionary dict;
    assert(dict.loadFromFile(filename));

    std::string answer;
    assert(dict.find("AT", answer) && answer == "OK");
    assert(dict.find("ATI", answer) && answer == "Model: Test");
    assert(dict.find("ATE0", answer) && answer == "OK");
    assert(dict.find("ATE1", answer) && answer == "OK");
    assert(dict.find("AT+COPS", answer) && answer == "+COPS: 0,0,\"Operator\"");
    assert(!dict.find("ATX", answer));

    std::remove(filename.c_str());
    std::cout << "Dictionary tests passed\n";
}

int main() {
    testPattern();
    testDictionary();
    return 0;
}
