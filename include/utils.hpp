#include <string>
#include <sstream>
#include <algorithm>
#include <iterator>
#include <iostream>
#include <chrono>
#include <thread>

template<class Container>
void split1(const std::string &str, Container &cont) {
    std::istringstream iss(str);
    std::copy(std::istream_iterator<std::string>(iss),
              std::istream_iterator<std::string>(),
              std::back_inserter(cont));
}
