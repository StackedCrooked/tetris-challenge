#ifndef H_ON_DESTROY_61D858BF84F9FE4096899AF857EB65AC
#define H_ON_DESTROY_61D858BF84F9FE4096899AF857EB65AC

namespace stm {
	template <typename F>
	struct on_destroy_impl {// : guard_impl {
		on_destroy_impl(F func) : func(func) {}
		~on_destroy_impl() { func(); }

	private:
		F func;
	};

	template <typename F>
	inline on_destroy_impl<F> on_destroy(F func) {
		return on_destroy_impl<F>(func);
	}

	//auto grd = on_destroy([]() { std::cout << "Hi"; });
}

#endif
