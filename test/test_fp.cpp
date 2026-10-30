#include"fp.hpp"
#include"test_framework.hpp"

using namespace field;

void test_fp_basic_arithmetic(){
	constexpr BigInt<3> modulus(7);
	using Fp7 = Fp<modulus>;

	Fp7 a(3);
	ASSERT_EQ(a * a.inv(), Fp7(1));

	Fp7 b(5);
	Fp7 pow_p_minus_1 = b;

	for(int i = 1; i < 6; ++i) pow_p_minus_1 *= b;

	ASSERT_EQ(pow_p_minus_1, Fp7(1));

	bool threw = false;
	try{
		Fp7 zero(0);
		[[maybe_unused]] auto inv = zero.inv();
	} catch(const std::domain_error&){
		threw = true;
	}

	ASSERT_TRUE(threw);
}

void test_fp_big_prime(){
	constexpr auto modulus = BigInt<256>::from_hex(
        "FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFEFFFFFC2F"
    );

	using Fp256 = Fp<modulus>;

    Fp256 a(BigInt<256>::from_hex("123456789ABCDEF0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF0"));
    Fp256 b(BigInt<256>::from_hex("FEDCBA9876543210FEDCBA9876543210FEDCBA9876543210FEDCBA9876543210"));

    ASSERT_EQ((a + b) - b, a);
    ASSERT_EQ((a - b) + b, a);

    Fp256 p_minus_1(modulus - BigInt<256>(1ULL));
    ASSERT_EQ(p_minus_1 + Fp256(1ULL), Fp256(0ULL));

    Fp256 a_inv = a.inv();
    ASSERT_EQ(a * a_inv, Fp256(1ULL));

    Fp256 prod = a * b;
    ASSERT_EQ(prod / b, a);

    Fp256 c(999999ULL);
    ASSERT_EQ(a * (b + c), (a * b) + (a * c));

    constexpr auto compile_time_check = []() {
        Fp256 x(123456789ULL);
        Fp256 y(987654321ULL);
        return ((x + y) - y) == x;
    }();
    static_assert(compile_time_check, "constexpr failed");
}

void tests_fp(){
	test_fp_basic_arithmetic();
	test_fp_big_prime();
}
