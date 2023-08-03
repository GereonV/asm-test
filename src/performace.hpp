#ifndef PERFORMANCE_HPP
#define PERFORMANCE_HPP

#include <chrono>

namespace performance {

	using Clock    = std::chrono::high_resolution_clock;
	using Duration = Clock::duration;
	struct Result {
		Duration duration;
		std::uint64_t iterations;
	};

	template<typename F, typename S>
	[[nodiscard]] inline Result measure_performance(
		F && f,
		Duration min_duration,
		S && s = [] {}
	) noexcept(noexcept(s()) && noexcept(f())) {
		Duration duration{0};
		std::uint64_t iterations{0};
		while(duration < min_duration) {
			static_cast<S &&>(s)();
			auto start = Clock::now();
			static_cast<F &&>(f)();
			auto end = Clock::now();
			duration += end - start;
			++iterations;
		}
		return { duration, iterations };
	}

}

#endif
