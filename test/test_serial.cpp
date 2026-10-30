#include<string>
#include<array>

#include"fp.hpp"
#include"fpk.hpp"
#include"test_framework.hpp"
#include"polynomial_utils.hpp"
#include"serializer.hpp"

struct AESPolynomial{
	static constexpr std::array<Fp<2>,9> value = {1,1,0,1,1,0,0,0,1};
};

void test_fp_serde(){
	constexpr int p=7;
	using Fp7 = field::Fp<p>;
	Fp7 a(3);

	std::string a_serd10 = ser::serialize<p>(a, ser::Encoding::Base10);
	ASSERT_EQ("3", a_serd10);

	Fp7 b10 = ser::deserialize_fp<p>(a_serd10, ser::Encoding::Base10);
	ASSERT_EQ(a,b10);

	std::string a_serd16 = ser::serialize<p>(a, ser::Encoding::Base16);
	ASSERT_EQ("0x3", a_serd16);

	Fp7 b16 = ser::deserialize_fp<p>(a_serd16, ser::Encoding::Base16);
	ASSERT_EQ(a, b16);
}

void test_fp_serde2(){
	constexpr int p=19;
	using Fp19 = field::Fp<p>;
	Fp19 a(17);

	std::string a_serd10 = ser::serialize<p>(a, ser::Encoding::Base10);
	ASSERT_EQ("17", a_serd10);

	Fp19 b10 = ser::deserialize_fp<p>(a_serd10, ser::Encoding::Base10);
	ASSERT_EQ(a,b10);

	std::string a_serd16 = ser::serialize<p>(a, ser::Encoding::Base16);
	ASSERT_EQ("0x11", a_serd16);

	Fp19 b16 = ser::deserialize_fp<p>(a_serd16, ser::Encoding::Base16);
	ASSERT_EQ(a, b16);
}

void test_fp_serde3(){
	constexpr int p=331;
	using Fp19 = field::Fp<p>;
	Fp19 a(16*15 + 3);

	std::string a_serd10 = ser::serialize<p>(a, ser::Encoding::Base10);
	ASSERT_EQ("243", a_serd10);

	Fp19 b10 = ser::deserialize_fp<p>(a_serd10, ser::Encoding::Base10);
	ASSERT_EQ(a,b10);

	std::string a_serd16 = ser::serialize<p>(a, ser::Encoding::Base16);
	ASSERT_EQ("0xf3", a_serd16);

	Fp19 b16 = ser::deserialize_fp<p>(a_serd16, ser::Encoding::Base16);
	ASSERT_EQ(a, b16);
}

void test_f2m_serde(){
	constexpr int p=2;
	constexpr int k=8;

	using F2m = Fpk<p,k,AESPolynomial>;
	F2m f2m_elem;
	f2m_elem[0] = 1;
	f2m_elem[3] = 1;
	f2m_elem[7] = 1;

	for(auto enc : {ser::Encoding::Base10, ser::Encoding::Base16, ser::Encoding::Base64}){
		std::string s = ser::serialize(f2m_elem, enc);
		F2m restored = ser::deserialize_f2m<k,AESPolynomial>(s,enc);
		ASSERT_EQ(f2m_elem,restored);
	}
}

void test_fpk_serde(){
	using Fpk = Fpk<5,3>;
	Fpk fpk_elem({1,4,2});

	for(auto enc : {ser::Encoding::Base10, ser::Encoding::Base16, ser::Encoding::Base64}){
		std::string s = ser::serialize(fpk_elem, enc);
		Fpk restored = ser::deserialize_fpk<5,3,AutoIrreducible<5,3>>(s,enc);
		ASSERT_EQ(fpk_elem,restored);
	}
}

void test_deser_field_general(){
	using Fp19 = field::Fp<19>;
	Fp19 fp_elem(17);

	for(auto enc : {ser::Encoding::Base10, ser::Encoding::Base16, ser::Encoding::Base64}){
		std::string s = ser::serialize(fp_elem, enc);
		Fp19 restored = ser::deserialize<Fp19>(s,enc);
		ASSERT_EQ(fp_elem,restored);
	}

	using F2m = Fpk<2,8,AESPolynomial>;
	F2m f2m_elem;
	f2m_elem[0] = 1;
	f2m_elem[3] = 1;
	f2m_elem[7] = 1;

	for(auto enc : {ser::Encoding::Base10, ser::Encoding::Base16, ser::Encoding::Base64}){
		std::string s = ser::serialize(f2m_elem, enc);
		F2m restored = ser::deserialize<F2m>(s,enc);
		ASSERT_EQ(f2m_elem,restored);
	}

	using Fpk = Fpk<5,3>;
	Fpk fpk_elem({15,3,1});

	for(auto enc : {ser::Encoding::Base10, ser::Encoding::Base16, ser::Encoding::Base64}){
		std::string s = ser::serialize(fpk_elem, enc);
		Fpk restored = ser::deserialize<Fpk>(s,enc);
		ASSERT_EQ(fpk_elem,restored);
	}
}

void test_point_and_curve_serde(){
	using Field = Fpk<5,2>;
	Field a = Field(1);
	Field b = Field(2);
	crypto::EllipticCurve<Field> curve(a,b);

	Field x({1,0});
	Field y({2,0});

	crypto::Point<Field> p = curve.point(x,y);

	for(auto enc : {ser::Encoding::Base10, ser::Encoding::Base16, ser::Encoding::Base64}){
		std::string p_str = ser::serialize(p,enc);
		auto restored = ser::deserialize_point<Field>(curve, p_str,enc);
		ASSERT_EQ(p, restored);

		const std::string curve_str = ser::serialize<Field>(curve,enc);
		crypto::EllipticCurve<Field> curve_restored = 
			ser::deserialize_curve<Field>(curve_str,enc);
		ASSERT_EQ(curve_restored, curve);
	}

}


void tests_serial(){
	test_fp_serde();
	test_fp_serde2();
	test_fp_serde3();
	test_f2m_serde();
	test_fpk_serde();
	test_deser_field_general();
	test_point_and_curve_serde();
}
