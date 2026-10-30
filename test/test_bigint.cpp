#include<array>
#include<string>
#include<iostream>
#include<limits>

#include"bigint.hpp"
#include"test_framework.hpp"

void test_bigint_add(){
	constexpr int N = 128;
	BigInt<N> x(std::numeric_limits<int>::max());
	BigInt<N> y(std::numeric_limits<int>::max());
	x+=y;
	x-=y;
	ASSERT_EQ(x,y);
}

void test_bigint_mul(){
	constexpr int N = 256;
	BigInt<N> two(2);
	BigInt<N> three(3);
	BigInt<N> x(std::numeric_limits<int>::max());
	BigInt<N> y(std::numeric_limits<int>::max());
	x+=y;
	ASSERT_EQ(x,y*two);
	x+=y;
	ASSERT_EQ(x,y*three);
}

void test_bigint_divrem(){
	constexpr int N = 256;
	BigInt<N> two(2);
	BigInt<N> zero(0);
	BigInt<N> x(std::numeric_limits<int>::max());
	BigInt<N> y(std::numeric_limits<int>::max());
	BigInt<N> quo;
	BigInt<N> rem;
	x*=2;
	auto z = x/y;
	auto w = x%y;
	ASSERT_EQ(z,two);
	ASSERT_EQ(w,zero);
}

void test_bigint_scalar_ops(){
	constexpr int N = 256;

	BigInt<N> val(1000);
	val += 500ULL;
	ASSERT_EQ(val, BigInt<N>(1500));

	val -= 200ULL;
	ASSERT_EQ(val, BigInt<N>(1300));

	BigInt<N> base(123456789ULL);
	BigInt<N> res1 = base*42;
	BigInt<N> res2 = 42 * base;
	ASSERT_EQ(res1, res2);

	BigInt<N> num(9876543210ULL);
	uint64_t den = 12345ULL;
	BigInt<N> quo = num / den;
	uint64_t rem = num % den;

	ASSERT_EQ(quo * den + rem, num);
}

void test_bigint_4096(){
	BigInt<4096> m = BigInt<4096>::max();
	BigInt<4096> expected = ~BigInt<4096>(0);
	ASSERT_EQ(m, expected);
}


void tests_bigint(){
	test_bigint_add();
	test_bigint_mul();
	test_bigint_divrem();
	test_bigint_scalar_ops();
	test_bigint_4096();
}
