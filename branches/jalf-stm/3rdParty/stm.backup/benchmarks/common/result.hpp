#ifndef STM_RESULT_HPP
#define STM_RESULT_HPP

#include <stm/atomic_detail.hpp>
#include <map>
#include <string>
#include <vector>

struct single_permutation_result {
    single_permutation_result() : tx_size(0) {}
    int tx_size;
    std::map<std::string, std::vector<stm::atomic_detail>> threads;
};

typedef std::vector<std::vector<single_permutation_result>> result_type;

#endif

