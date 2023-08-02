#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <algorithm>
#include <chrono>
#include <execution>
#include <iostream>
#include <memory>

using clamp_func = void(std::uint32_t * data, std::uint64_t count) noexcept;

extern "C" clamp_func simple_clamp, opt_clamp; // handwritten assembly

extern "C" [[gnu::noinline]] void replace_if(std::uint32_t * data, std::uint64_t count) noexcept {
	std::replace_if(std::execution::par_unseq, data, data + count, [](auto x) { return x > 255; }, 255);
}

extern "C" [[gnu::noinline]] void transform(std::uint32_t * data, std::uint64_t count) noexcept {
	std::transform(std::execution::par_unseq, data, data + count, data, [](auto x) { return x > 255 ? 255 : x; });
}

struct func_t {
	char const * name;
	clamp_func * func;
};

#define FUNC(func) func_t{#func, func}

inline constexpr func_t functions[]{
	FUNC(simple_clamp),
	FUNC(replace_if),
	FUNC(transform),
	// FUNC(opt_clamp),
};

int main(int, char ** argv) {
	using namespace std::chrono;
	using clock = high_resolution_clock;
	constexpr auto max_count = 1 << 20;
	auto arr = std::make_unique<std::uint32_t[]>(max_count);
	auto buf = std::make_unique<std::uint32_t[]>(max_count);
	std::generate(arr.get(), arr.get() + max_count, std::rand);
	for(auto && f : functions) {
		#if 1
		std::cout << "Profiling function: " << f.name << '\n';
		for(std::uint64_t count{max_count >> 5}; count <= max_count; count <<= 1) {
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
			duration<double> duration_s{duration_ns};
			auto seconds = duration_s.count();
			std::cout <<
				"count=" << std::setw(6) << count <<
				"\tclamps=" << std::setw(10) << iterations * count <<
				"\tclamps/s=" << iterations * count / seconds << std::endl;
		}
		std::cout << '\n';
		#else
		std::transform(std::execution::par_unseq, arr.get(), arr.get() + max_count, arr.get(), [](auto x) { return x % 512; });
		std::cout << "Testing function: " << f.name << std::flush;
		for(std::uint64_t alignment{4}; alignment <= 64; alignment *= 2) {
			auto offset = (64 + alignment - ((std::uintptr_t) buf.get() % 64)) / 4;
			for(std::uint64_t count{}; count <= 512; ++count) {
				auto ptr = buf.get() + offset;
				auto og  = arr.get() + offset;
				std::memcpy(ptr, og, 4 * count);
				f.func(ptr, count);
				if(!std::ranges::equal(og, og + count, ptr, ptr + count, {}, [](auto x) { return x > 255 ? 255 : x; })) {
					std::cout << " - function failed\n";
					return 1;
				}
			}
		}
		std::cout << " - function ok\n";
		#endif
	}
}

