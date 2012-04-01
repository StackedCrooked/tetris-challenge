#ifndef STM_WRITELIST_HPP
#define STM_WRITELIST_HPP

#include <algorithm>
#include <vector>

namespace stm {
  namespace frontend {
    struct address_of {
      backend::shared_base** operator()(backend::shared_base*& shared){
        return &shared;
      }
    };

    enum writelist_policies {
      sorted_on_heap, // allocate a vector on the heap in which write entries can be sorted
      sorted_on_stack, // allocate an array on the stack
      unsorted // don't sort (dangerous, can introduce livelocks, but can perhaps be used for the first attempt at committing before reverting to a safer strategy)
    };

    template <typename transaction_type, writelist_policies policy>
    struct writelist;

    // todo: currently has a different interface than sorted variants. Bring in line
    //template <typename transaction_type>
    //struct writelist<transaction_type, unsorted> {
    //	typedef typename transaction_type::iterator_rw iterator;
    //	writelist(tx_manager& mgr, const typename tx_manager::tx_marker& marker) : mgr(mgr), marker(marker) {}
    //	iterator begin() {
    //		return mgr.marker().second;
    //	}
    //	iterator end() {
    //		return marker.second;
    //	}

    //private:
    //	tx_manager& mgr;
    //	const typename tx_manager::tx_marker& marker;
    //};

    template <typename transaction_type>
    struct writelist<transaction_type, sorted_on_stack> {
      typedef backend::shared_base** list_entry;
      typedef list_entry* iterator;
      writelist(transaction_type& tx) {
        last = std::transform(tx.tx_begin().second, tx.tx_end().second, writes, address_of());
        std::sort(writes, last);
      }
      iterator begin() {
        return writes;
      }
      iterator end() {
        return last;
      }

    private:
      list_entry writes[config::max_writeset_size_for_stack];
      iterator last;
    };

    template <typename transaction_type>
    struct writelist<transaction_type, sorted_on_heap> {
      typedef backend::shared_base** list_entry;
      typedef std::vector<list_entry>::iterator iterator;
      writelist(transaction_type& tx) : writes(tx.manager().writeset_size(tx.tx_end().second)){
        std::transform(tx.tx_begin().second, tx.tx_end().second, writes.begin(), address_of());
        std::sort(writes.begin(), writes.end());
      }
      iterator begin() {
        return writes.begin();
      }
      iterator end() {
        return writes.end();
      }

    private:
      std::vector<list_entry> writes;
    };
  }
}

#endif
