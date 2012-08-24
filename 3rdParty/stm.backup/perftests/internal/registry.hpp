#ifndef H_REGISTRY_6218782BF10CC6429B8A07BD1FA0DE9C
#define H_REGISTRY_6218782BF10CC6429B8A07BD1FA0DE9C

#include <map>
#include <string>

struct func_registry {
  typedef void (*proto)();
  bool reg(std::string name, proto f) {
    fns.insert(std::make_pair(name, f));
    return true;
  }
  typedef std::map<std::string, proto>::const_iterator iterator;
  const std::map<std::string, proto>& map() { return fns; }
private:
  std::map<std::string, proto> fns;
};

inline func_registry& funcs() {
  static func_registry fs;
  return fs;
}

#define STM_BENCHMARK(x) \
void x(); \
namespace { \
  bool regd = funcs().reg(#x, x); \
} \
void x()  \

#endif
