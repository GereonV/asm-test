#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <algorithm>
#include <execution>
#include <iomanip>
#include <iostream>
#include <memory>
#include "performace.hpp"

using clamp_func = void(std::uint32_t * data, std::uint64_t count) noexcept;

extern "C" clamp_func simple_clamp, cmov_clamp, opt_clamp; // handwritten assembly

[[gnu::noinline]] void replace_if(std::uint32_t * data, std::uint64_t count) noexcept {
	std::replace_if(std::execution::par_unseq, data, data + count, [](auto x) { return x > 255; }, 255);
}

[[gnu::noinline]] void transform(std::uint32_t * data, std::uint64_t count) noexcept {
	std::transform(std::execution::par_unseq, data, data + count, data, [](auto x) { return x > 255 ? 255 : x; });
}

struct func_t {
	char const * name;
	clamp_func * func;
};

#define FUNC(func) func_t{#func, func}

inline constexpr func_t functions[]{
	FUNC(simple_clamp),
	FUNC(cmov_clamp),
	FUNC(replace_if),
	FUNC(transform),
	FUNC(opt_clamp),
};

int main() {
	using namespace std::chrono_literals;
	constexpr auto min_duration = 1500ms;
	constexpr std::uint64_t min_count{32}, max_count{1 << 15};
	auto arr = std::make_unique<std::uint32_t[]>(max_count);
	auto buf = std::make_unique<std::uint32_t[]>(max_count);
	std::generate(arr.get(), arr.get() + max_count, std::rand);
	auto og  = arr.get();
	auto ptr = buf.get();
	for(auto && f : functions) {
		#if 1
		std::cout << "Profiling function: " << f.name << '\n';
		for(auto count = min_count; count <= max_count; count <<= 1) {
			auto setup = [=] { std::memcpy(ptr, og, 4 * count); };
			auto func = [=] { f.func(ptr, count); };
			auto [duration, iterations] = performance::measure_performance(func, min_duration, setup);
			auto seconds = std::chrono::duration<double>{duration}.count();
			std::cout <<
				"count=" << std::setw(7) << count <<
				"\tclamps=" << std::setw(10) << iterations * count <<
				"\tclamps/s=" << iterations * count / seconds << std::endl;
		}
		std::cout << '\n';
		#else
		std::cout << "Testing function: " << f.name << std::flush;
		for(std::uint64_t alignment{4}; alignment <= 64; alignment *= 2) {
			auto offset = (64 + alignment - ((std::uintptr_t) buf.get() % 64)) / 4;
			arr[offset] = 10;
			arr[offset + 1] = -10;
			for(std::uint64_t count{}; count <= 512; ++count) {
				auto ptr = buf.get() + offset;
				auto og  = arr.get() + offset;
				std::memcpy(ptr, og, 4 * count);
				f.func(ptr, count);
				if(!std::ranges::equal(og, og + count, ptr, ptr + count, {}, [](auto x) { return x > 255 ? 255 : x; })) {
					std::cout << " - function failed\n";
					std::cout << "count=" << count << "\talignment=" << alignment << std::endl;
					return 1;
				}
			}
		}
		std::cout << " - function ok\n";
		#endif
	}
}

