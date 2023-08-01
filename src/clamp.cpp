#include <cstdint>
#include <cstdlib>
#include <algorithm>
#include <chrono>
#include <iostream>
#include <memory>

extern "C" {
	using clamp_func = void(std::uint32_t * data, std::uint64_t count);
	clamp_func cmov_clamp, jmp_clamp, opt_clamp; // handwritten assembly
	clamp_func c_clamp, c_clamp_restrict, c_clamp_expect, c_clamp_cond, c_clamp_cond_expect; // index-loop
	clamp_func cpp_clamp, cpp_expect; // STL
}

[[gnu::noinline]] void c_clamp(std::uint32_t * data, std::uint64_t count) {
	for(std::uint64_t i{}; i < count; ++i)
		data[i] = data[i] > 255 ? 255 : data[i];
}

[[gnu::noinline]] void c_clamp_restrict(std::uint32_t * __restrict__ data, std::uint64_t count) {
	for(std::uint64_t i{}; i < count; ++i)
		data[i] = data[i] > 255 ? 255 : data[i];
}

[[gnu::noinline]] void c_clamp_expect(std::uint32_t * data, std::uint64_t count) {
	for(std::uint64_t i{}; i < count; ++i)
		data[i] = __builtin_expect(data[i] > 255, true) ? 255 : data[i];
}

[[gnu::noinline]] void c_clamp_cond(std::uint32_t * data, std::uint64_t count) {
	for(std::uint64_t i{}; i < count; ++i)
		if(data[i] > 255)
			data[i] = 255;
}

[[gnu::noinline]] void c_clamp_cond_expect(std::uint32_t * data, std::uint64_t count) {
	for(std::uint64_t i{}; i < count; ++i)
		if(__builtin_expect(data[i] > 255, true))
			data[i] = 255;
}

[[gnu::noinline]] void cpp_clamp(std::uint32_t * data, std::uint64_t count) {
	std::replace_if(data, data + count, [](auto x) { return x > 255; }, 255);
}

[[gnu::noinline]] void cpp_clamp_expect(std::uint32_t * data, std::uint64_t count) {
	std::replace_if(data, data + count, [](auto x) { return __builtin_expect(x > 255, true); }, 255);
}

struct func_t {
	char const * name;
	clamp_func * func;
};

#define FUNC(func) func_t{#func, func}

inline constexpr func_t functions[]{
	FUNC(cmov_clamp),
	FUNC(jmp_clamp),
	FUNC(opt_clamp),
	FUNC(c_clamp),
	FUNC(c_clamp_restrict),
	FUNC(c_clamp_expect),
	FUNC(c_clamp_cond),
	FUNC(c_clamp_cond_expect),
	FUNC(cpp_clamp),
	FUNC(cpp_clamp_expect),
};

int main(int, char ** argv) {
	using namespace std::chrono;
	using clock = high_resolution_clock;
	for(auto && f : functions) {
		std::cout << "Profiling function: " << f.name << '\n';
		for(auto count{1 << 5}; count <= (1 << 18); count <<= 1) {
			auto arr = std::make_unique<std::uint32_t[]>(count);
			auto buf = std::make_unique<std::uint32_t[]>(count);
			std::generate(arr.get(), arr.get() + count, std::rand);
			nanoseconds duration_ns{0};
			std::uint64_t iterations = 0;
			while(duration_ns < seconds{1}) {
				std::copy(arr.get(), arr.get() + count, buf.get());
				auto start = clock::now();
				f.func(arr.get(), count);
				auto end = clock::now();
				duration_ns += end - start;
				++iterations;
			}
			auto duration_s = duration_cast<duration<double>>(duration_ns);
			auto seconds = duration_s.count();
			std::cout <<
				"count=" << std::setw(4) << count <<
				"\tbytes=" << std::setw(10) << iterations * count <<
				"\tclamps/s=" << iterations * count / seconds << std::endl;
		}
		std::cout << '\n';
	}
}

