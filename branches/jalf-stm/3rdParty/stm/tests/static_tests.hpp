#include <stm.hpp>

void shared_is_copyable(stm::shared<int>& i) {
	stm::shared<int> j = i;
	stm::shared<int> k(i);
}
void shared_is_assignable(stm::shared<int>& i, stm::shared<int>& j) {
	i = j;
}

void shared_is_default_constructible(){
	stm::shared<int> i;
}
