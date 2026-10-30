#include"fp.hpp"
#include"fpk.hpp"
#include"elliptic.hpp"

#include"test_framework.hpp"

struct AESPolynomial{
	static constexpr std::array<Fp<2>,9> value = {1,1,0,1,1,0,0,0,1};
};

void test_e2e(){
	using Field = field::Fpk<2,8,AESPolynomial>;

	Field a = a;
	Field b = b;

	crypto::EllipticCurve<Field> curve(a,b);
}
