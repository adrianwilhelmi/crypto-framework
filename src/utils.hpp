#pragma once

#include<bigint.hpp>

namespace utils{

template<uint64_t N>
constexpr bool is_prime(const BigInt<N>& n){
	if(n <= BigInt<N>(1)) return false;
	if(n == BigInt<N>(2) || n == BigInt<N>(3)) return true;
	if(n.is_even()) return false;

	for(BigInt<N> i(3); i <= n/i; i+=2){
		if(n % i == 0) return false;
	}
	return true;
}

} // namespace utils
