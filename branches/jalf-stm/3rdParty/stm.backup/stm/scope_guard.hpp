#ifndef H_SCOPE_GUARD_7C0504FBBDBAD840BFC3B3AA6358755C
#define H_SCOPE_GUARD_7C0504FBBDBAD840BFC3B3AA6358755C

namespace stm {
	template <typename F>
	struct scope_exit_impl {
		explicit scope_exit_impl(F&& f) : f(std::move(f)), active(true) {}
		scope_exit_impl(scope_exit_impl&& other) : f(std::move(other.f)), active() {
			std::swap(active, other.active);
		}
		~scope_exit_impl() { if (active) {f(); } }
	private:
		scope_exit_impl(const scope_exit_impl& other);
		scope_exit_impl& operator=(const scope_exit_impl&);

		F f;
		bool active;

	};

	template <typename F>
	inline scope_exit_impl<F> scope_guard(F&& f) {
		return scope_exit_impl<F>(std::move(f));
	}
}
#endif
