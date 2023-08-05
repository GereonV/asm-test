#include <cstdlib>
#include <cstring>
#include <algorithm>
#include <iostream>
#include <memory>
#include "performace.hpp"

using func_t = void * (void *, void const *, size_t) noexcept;

extern "C" func_t mov_memcpy, movs_memcpy, rep_movs_memcpy;

struct func {
	char const * name;
	func_t * func;
};

#define FUNC(FUNC) func{#FUNC, FUNC}

inline constexpr func functions[] = {
	FUNC(movs_memcpy),
	FUNC(rep_movs_memcpy),
	FUNC(std::memcpy),
};

int main() {
	constexpr auto max_count = 1 << 25;
	auto src = std::make_unique<char[]>(max_count);
	auto dst = std::make_unique<char[]>(max_count);
	std::transform(src.get(), src.get() + max_count, src.get(), [](auto) { return std::rand() % 128; });
	for(auto && [name, f] : functions) {
		std::cout << "Testing function: " << name << std::flush;
		for(int count{}; count < 1024; ++count) {
			std::memset(dst.get(), -1, count + 32);
			f(dst.get(), src.get(), count);
			if(!std::equal(src.get(), src.get() + count, dst.get()) || std::count(dst.get() + count, dst.get() + count + 32, -1) != 32) {
				std::cout << " - failed\n";
				std::cout << "count=" << count << '\n';
				return 1;
			}
		}
		std::cout << " - ok\n";
	}
}
