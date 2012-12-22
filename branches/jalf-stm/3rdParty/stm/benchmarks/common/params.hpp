#ifndef STM_PARAMS_HPP
#define STM_PARAMS_HPP

#include <stm/atomic_detail.hpp>

#include <functional>
#include <string>
#include <map>


static const char* tx_size_param_name = "transaction size";
static const int duration_ms = 200;
enum { iter_count = 5 };

struct param_data {
    typedef std::function<void(stm::atomic_detail&, int)> func_type;
    typedef std::vector<int> vals_type;

    param_data(const func_type& func = func_type()) : func(func){}

    func_type func;
    vals_type vals;
};

struct param_pos {
    param_pos() : param(), current_val() {}
    param_pos(std::pair<const std::string, param_data>& param) : param(&param), current_val(param.second.vals.begin()) {}

    const std::string& name() { return param->first; }
    param_data& data() const { return param->second; }
    param_data::vals_type::iterator current_val;
private:
    std::pair<const std::string, param_data>* param;
};

typedef std::map<std::string, param_data> parameter_map_type;
typedef parameter_map_type::value_type full_param_type;

struct inserter {
    inserter(param_data::vals_type& params) : params(params) {}

    inserter& operator()(int value){
        params.push_back(value);
        return *this;
    }

    param_data::vals_type& params;
};

typedef param_data::func_type thread_func;

#endif

