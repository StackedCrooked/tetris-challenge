#include "registry.hpp"
#include <algorithm>
#include <iostream>

namespace {
struct run {
  void operator()(std::pair<std::string, func_registry::proto> pair) {
    pair.second();
  }
  void operator()(std::string str) {
    func_registry::iterator res = funcs().map().find(str);
    if (res == funcs().map().end()){
      std::cout << "Could not find function '" << str << "'\n";
    }
    else {
      (*this)(*res);
    }
  }
};

struct print {
  void operator()(std::pair<std::string, func_registry::proto> pair) {
    std::cout << pair.first << "\n";
  }
};
}

int main(int argc, char** argv) {
  if (argc != 1) {
    std::for_each(argv+1, argv+argc, run());
  }
  else {
    std::for_each(funcs().map().begin(), funcs().map().end(), run());
  }
}
