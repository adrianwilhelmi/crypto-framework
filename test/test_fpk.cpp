#include<array>
#include<type_traits>
#include<iostream>

#include"bigint.hpp"
#include"fp.hpp"
#include"fpk.hpp"
#include"test_framework.hpp"
#include"polynomial_utils.hpp"

using namespace field;

struct Poly4{
	static constexpr BigInt<2> two{2};
	static constexpr std::array<Fp<two>, 3> value = {1,1,1};
};

struct AESPolynomial{
	static constexpr BigInt<2> two{2};
	static constexpr std::array<Fp<two>,9> value = {1,1,0,1,1,0,0,0,1};
};

struct PolyF3k2{
	static constexpr BigInt<2> three{3};
	static constexpr std::array<Fp<three>,3> value = {1,0,1};
};

void test_fpk_xsquared(){
	constexpr BigInt<2> two(2);
	Fpk<two,2,Poly4> x;
	x[0] = 0;
	x[1] = 1;

	auto x2 = x*x;

	ASSERT_EQ(x2[0], 1);
	ASSERT_EQ(x2[1], 1);
}

void test_fpk_ord(){
	// for each a != 0, a^(p^k-1) = 1
	constexpr BigInt<2> two(2);
	Fpk<two,2,Poly4> x;
	x[0] = 0;
	x[1] = 1;

	Fpk<two,2,Poly4> one;
	one[0] = 1;
	one[1] = 0;

	Fpk<two,2,Poly4> x_plus_one;
	x_plus_one[0] = 1;
	x_plus_one[1] = 1;

	ASSERT_EQ(x.pow(3), one);
	ASSERT_EQ(one.pow(3), one);
	ASSERT_EQ(x_plus_one.pow(3), one);
}

void test_fpk_mul_by_inv(){
	constexpr BigInt<2> two(2);
	Fpk<two,2,Poly4> x;
	x[0] = 0;
	x[1] = 1;

	Fpk<two,2,Poly4> one;
	one[0] = 1;
	one[1] = 0;

	ASSERT_EQ(x * x.inv(), one);
}

template<auto p, int k>
void test_single_k_automatic_polynomial(){
	Fpk<p,k> auto_modulus;

	const auto& modulus = auto_modulus.modulus();
	std::vector<Fp<p>> modulus_vec(k+1);

	for(int i = 0; i < k+1; ++i){
		modulus_vec[i] = modulus[i];
	}

	ASSERT_TRUE(utils::is_irreducible<p>(modulus_vec));
}

void test_automatic_polynomial(){
	static constexpr BigInt<2> p(2);

	[]<int... ks>() {
		(test_single_k_automatic_polynomial<p,ks>(), ...);
	}.template operator()<2,4,8,9,10,16>();
}

void test_fpk_fp_edge_case(){
	constexpr BigInt<3> five(5);
	constexpr BigInt<2> two(2);
	constexpr bool is_same_type1 = std::is_same_v<Fpk<five,1>, Fp<five>>;
	constexpr bool is_same_type2 = std::is_same_v<Fpk<five,3>, FpkImpl<five,3>>;
	constexpr bool is_same_type3 = std::is_same_v<Fpk<two,4>, FpkBinaryImpl<two,4>>;

	ASSERT_TRUE(is_same_type1);
	ASSERT_TRUE(is_same_type2);
	ASSERT_TRUE(is_same_type3);
}

void test_binary_field_basic_ops(){
	constexpr BigInt<2> two(2);
	using GF256 = Fpk<two,8,AESPolynomial>;

	// x^3 + x + 1
	GF256 a;
	a[0] = 1;
	a[1] = 1;
	a[3] = 1;

	ASSERT_TRUE(a[0] == 1);
	ASSERT_TRUE(a[1] == 1);
	ASSERT_TRUE(a[2] == 0);
	ASSERT_TRUE(a[3] == 1);

	GF256 b;
	b[0] = a[3];
	b[1] = 0;
	ASSERT_TRUE(b[0] == 1);
	ASSERT_TRUE(b[1] == 0);

	GF256 c = a + b;
	ASSERT_EQ(-a, a);
	ASSERT_EQ(a - b, a + b);
	ASSERT_TRUE(c[0] == 0);
	ASSERT_TRUE(c[1] == 1);
}

void test_binary_field_mul_inv_pow(){
	constexpr BigInt<2> two(2);
	using GF256 = Fpk<two,8,AESPolynomial>;

	GF256 one;
	one[0] = 1;

	// x
	GF256 x;
	x[1] = 1;

	GF256 x_inv = x.inv();

	ASSERT_EQ(x * x_inv, one);
	ASSERT_EQ(x.pow(255), one);
}

void test_binary_field_boundary_k64(){
	constexpr BigInt<2> two(2);
	using GF2_64 = Fpk<two,64>;

	GF2_64 elem1;
	elem1[0] = 1;
	elem1[63] = 1;

	GF2_64 elem2 = elem1 + elem1;
	ASSERT_TRUE(elem2.is_zero());
}

void test_general_fpk_arithmetic(){
	constexpr BigInt<2> three(3);
	using GF9 = Fpk<three,2, PolyF3k2>;

	GF9 a({1,2});
	GF9 b({2,1});

	GF9 sum = a + b;
	ASSERT_TRUE(sum.is_zero());

	GF9 prod = a*b;
	ASSERT_EQ(prod[0], 0);
	ASSERT_EQ(prod[1], 2);

	GF9 a_inv = a.inv();
	ASSERT_EQ(a*a_inv, GF9(1));

	GF9 div_res = prod/b;
	ASSERT_EQ(div_res, a);
}

struct Polynomial163{
	static constexpr std::array<Fp<2>, 164> value = []() {
		std::array<Fp<2>, 164> arr{};
		arr[163] = 1;
		arr[7] = 1;
		arr[6] = 1;
		arr[3] = 1;
		arr[0] = 1;
		return arr;
	}();

};

void test_fpk_big_field(){
	constexpr BigInt<2> two(2);
	using Field163 = Fpk<two,163,Polynomial163>;

	Field163 a;
	a[0] = 1;
	a[162] = 1;

	Field163 one(1);

	Field163 a_inv = a.inv();

	ASSERT_EQ(a*a_inv, one);

	ASSERT_EQ(-a, a);
	ASSERT_TRUE((a+a).is_zero());
}

void tests_fpk(){
	test_fpk_xsquared();
	test_fpk_ord();
	test_fpk_mul_by_inv();
	test_automatic_polynomial();
	test_fpk_fp_edge_case();
	test_binary_field_basic_ops();
	test_binary_field_mul_inv_pow();
	test_binary_field_boundary_k64();
	test_general_fpk_arithmetic();
	test_fpk_big_field();
}
