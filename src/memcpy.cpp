#include <cstdlib>
#include <cstring>
#include <algorithm>
#include <iostream>
#include <memory>
#include "performace.hpp"

using func_t = void * (void *, void const *, size_t) noexcept;

extern "C" func_t movs_memcpy, rep_movs_memcpy, avx_memcpy, stream_memcpy, opt_memcpy;

struct func {
	char const * name;
	func_t * func;
};

#define FUNC(FUNC) func{#FUNC, FUNC}

inline constexpr func functions[] = {
	// FUNC(movs_memcpy),
	// FUNC(rep_movs_memcpy),
	FUNC(avx_memcpy),
	FUNC(stream_memcpy),
	FUNC(opt_memcpy),
	FUNC(std::memcpy),
};

int main() {
	using namespace std::chrono_literals;
	constexpr auto min_duration = 1500ms;
	constexpr auto max_count = 1 << 25;
	auto src = std::make_unique<char[]>(max_count);
	auto dst = std::make_unique<char[]>(max_count);
	std::generate(src.get(), src.get() + max_count, [] { return std::rand() % 128; });
	rep_movs_memcpy(dst.get(), src.get(), max_count);
	for(auto && [name, f] : functions) {
		#if 1
		std::cout << "Profiling function: " << name << std::endl;
		for(auto count = 1024; count <= max_count; count *= 2) {
			auto [duration, iterations] = performance::measure_performance([&] { f(dst.get(), src.get(), count); }, min_duration);
			auto seconds = std::chrono::duration<double>{duration}.count();
			std::cout <<
				"count=" << std::setw(8) << count <<
				"\tbytes/s=" << iterations * count / seconds << std::endl;
		}
		std::cout << '\n';
		#else
		std::cout << "Testing function: " << name << std::flush;
		for(int count{}; count <= 1 << 21; count = count ? count * 2 : 1) {
			std::memset(dst.get(), 255, count + 32);
			f(dst.get(), src.get(), count);
			if(!std::equal(src.get(), src.get() + count, dst.get()) || std::any_of(dst.get() + count, dst.get() + count + 32, [](unsigned char x) { return x != 255; })) {
				std::cout << " - failed\n";
				std::cout << "count=" << count << '\n';
				return 1;
			}
		}
		std::cout << " - ok\n";
		#endif
	}
}
