#include"fp.hpp"
#include"test_framework.hpp"

using namespace field;

void tests_fp(){
	using Fp7 = Fp<7>;

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
