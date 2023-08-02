#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <algorithm>
#include <chrono>
#include <execution>
#include <iostream>
#include <memory>

using clamp_func = void(std::uint32_t * data, std::uint64_t count);

extern "C" clamp_func simple_clamp, opt_clamp; // handwritten assembly

extern "C" [[gnu::noinline]] void replace_if(std::uint32_t * data, std::uint64_t count) {
	std::replace_if(std::execution::par_unseq, data, data + count, [](auto x) { return x > 255; }, 255);
}

extern "C" [[gnu::noinline]] void transform_ternary(std::uint32_t * data, std::uint64_t count) {
	std::transform(std::execution::par_unseq, data, data + count, data, [](auto x) { return x > 255 ? 255 : x; });
}

extern "C" [[gnu::noinline]] void transform_mod(std::uint32_t * data, std::uint64_t count) {
	std::transform(std::execution::par_unseq, data, data + count, data, [](auto x) { return x % 256; });
}

struct func_t {
	char const * name;
	clamp_func * func;
};

#define FUNC(func) func_t{#func, func}

inline constexpr func_t functions[]{
	FUNC(simple_clamp),
	FUNC(replace_if),
	FUNC(transform_ternary),
	FUNC(transform_mod),
	FUNC(opt_clamp),
};

int main(int, char ** argv) {
	using namespace std::chrono;
	using clock = high_resolution_clock;
	constexpr auto max_count = 1 << 16;
	auto arr = std::make_unique<std::uint32_t[]>(max_count);
	auto buf = std::make_unique<std::uint32_t[]>(max_count);
	std::generate(arr.get(), arr.get() + max_count, std::rand);
	for(auto && f : functions) {
		std::cout << "Profiling function: " << f.name << '\n';
		for(auto count{1 << 10}; count <= (1 << 16); count <<= 1) {
			#if 1
			nanoseconds duration_ns{0};
			std::uint64_t iterations = 0;
			while(duration_ns < seconds{2}) {
				std::memcpy(buf.get(), arr.get(), 4 * count);
				auto start = clock::now();
				f.func(buf.get(), count);
				auto end = clock::now();
				duration_ns += end - start;
				++iterations;
			}
			auto duration_s = duration_cast<duration<double>>(duration_ns);
			auto seconds = duration_s.count();
			std::cout <<
				"count=" << std::setw(6) << count <<
				"\tclamps=" << std::setw(10) << iterations * count <<
				"\tclamps/s=" << iterations * count / seconds << std::endl;
			#else
			std::memcpy(buf.get(), arr.get(), 4 * count);
			if(std::any_of(buf.get(), buf.get() + count, [](auto x) { return x > 255; })) {
				std::cout << "function failed\n";
				return 1;
			}
			break;
			#endif
		}
		std::cout << '\n';
	}
}

