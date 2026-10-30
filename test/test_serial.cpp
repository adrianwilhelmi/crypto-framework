#include<iostream>
#include<string>
#include<array>

#include"bigint.hpp"
#include"fp.hpp"
#include"fpk.hpp"
#include"test_framework.hpp"
#include"polynomial_utils.hpp"
#include"serializer.hpp"

struct AESPolynomial{
	static constexpr BigInt<2> two{2};
	static constexpr std::array<Fp<two>,9> value = {1,1,0,1,1,0,0,0,1};
};

void test_fp_serde(){
	constexpr BigInt<3> p(7);
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
	constexpr BigInt<5> p(19);
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
	constexpr BigInt<9> p(331);
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
	constexpr BigInt<2> p(2);
	constexpr int k=8;

	using F2m = Fpk<p,k,AESPolynomial>;
	F2m f2m_elem;
	f2m_elem[0] = 1;
	f2m_elem[3] = 1;
	f2m_elem[7] = 1;

	for(auto enc : {ser::Encoding::Base10, ser::Encoding::Base16, ser::Encoding::Base64}){
		std::string s = ser::serialize(f2m_elem, enc);
		F2m restored = ser::deserialize_f2m<p,k,AESPolynomial>(s,enc);
		ASSERT_EQ(f2m_elem,restored);
	}
}

void test_fpk_serde(){
	constexpr BigInt<3> five(5);
	using Fpk = Fpk<five,3>;
	Fpk fpk_elem({1,4,2});

	for(auto enc : {ser::Encoding::Base10, ser::Encoding::Base16, ser::Encoding::Base64}){
		std::string s = ser::serialize(fpk_elem, enc);
		Fpk restored = ser::deserialize_fpk<five,3,AutoIrreducible<five,3>>(s,enc);
		ASSERT_EQ(fpk_elem,restored);
	}
}

void test_deser_field_general(){
	constexpr BigInt<5> p(19);
	using Fp19 = field::Fp<p>;
	Fp19 fp_elem(17);

	for(auto enc : {ser::Encoding::Base10, ser::Encoding::Base16, ser::Encoding::Base64}){
		std::string s = ser::serialize(fp_elem, enc);
		Fp19 restored = ser::deserialize<Fp19>(s,enc);
		ASSERT_EQ(fp_elem,restored);
	}

	constexpr BigInt<2> two(2);
	using F2m = Fpk<two,8,AESPolynomial>;
	F2m f2m_elem;
	f2m_elem[0] = 1;
	f2m_elem[3] = 1;
	f2m_elem[7] = 1;

	for(auto enc : {ser::Encoding::Base10, ser::Encoding::Base16, ser::Encoding::Base64}){
		std::string s = ser::serialize(f2m_elem, enc);
		F2m restored = ser::deserialize<F2m>(s,enc);
		ASSERT_EQ(f2m_elem,restored);
	}

	constexpr BigInt<3> five(5);
	using Fpk = Fpk<five,3>;
	Fpk fpk_elem({15,3,1});

	for(auto enc : {ser::Encoding::Base10, ser::Encoding::Base16, ser::Encoding::Base64}){
		std::string s = ser::serialize(fpk_elem, enc);
		Fpk restored = ser::deserialize<Fpk>(s,enc);
		ASSERT_EQ(fpk_elem,restored);
	}
}

void test_point_and_curve_serde(){
	constexpr BigInt<3> five(5);
	using Field = Fpk<five,2>;
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

void test_fp_large_serde(){
	static constexpr BigInt<128> p = (BigInt<128>(1) << 127) - BigInt<128>(1);
	using Fp128 = field::Fp<p>;

	BigInt<128> large_val = (BigInt<128>(1) << 80) + BigInt<128>(123456789ULL);
	Fp128 elem(large_val);

	for(auto enc : {ser::Encoding::Base10, ser::Encoding::Base16, ser::Encoding::Base64}){
		std::string s = ser::serialize(elem,enc);
		Fp128 restored = ser::deserialize<Fp128>(s,enc);

		ASSERT_EQ(elem,restored);
	}
}

void test_fp256_secp256k1_serde() {
    static constexpr BigInt<256> p = BigInt<256>::from_hex(
        "FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFEFFFFFC2F"
    );
    using Fp256 = field::Fp<p>;

    BigInt<256> val = BigInt<256>::from_hex(
        "A1B2C3D4E5F60718293A4B5C6D7E8F90123456789ABCDEF0123456789ABCDEF1"
    );
    Fp256 elem(val);

    for (auto enc : {ser::Encoding::Base10, ser::Encoding::Base16, ser::Encoding::Base64}) {
        std::string s = ser::serialize(elem, enc);
        Fp256 restored = ser::deserialize<Fp256>(s, enc);
        ASSERT_EQ(elem, restored);
    }
}

void test_fpk256_k2_serde() {
    static constexpr BigInt<256> p = BigInt<256>::from_hex(
        "FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFEFFFFFC2F"
    );
    using Field256_2 = field::Fpk<p, 2>;

    BigInt<256> c0_val = BigInt<256>::from_hex(
        "123456789ABCDEF0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF0"
    );
    BigInt<256> c1_val = BigInt<256>::from_hex(
        "FEDCBA9876543210FEDCBA9876543210FEDCBA9876543210FEDCBA9876543210"
    );

    Field256_2 elem({c0_val, c1_val});

    for (auto enc : {ser::Encoding::Base10, ser::Encoding::Base16, ser::Encoding::Base64}) {
        std::string s = ser::serialize(elem, enc);
        Field256_2 restored = ser::deserialize<Field256_2>(s, enc);
        ASSERT_EQ(elem, restored);
    }
}

void test_secp256k1_point_serde() {
    static constexpr BigInt<256> p = BigInt<256>::from_hex(
        "FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFEFFFFFC2F"
    );
    using Field = field::Fp<p>;

    Field a(0);
    Field b(7);
    crypto::EllipticCurve<Field> curve(a, b);

    Field gx(BigInt<256>::from_hex("79BE667EF9DCBBAC55A06295CE870B07029BFCDB2DCE28D959F2815B16F81798"));
    Field gy(BigInt<256>::from_hex("483ADA7726A3C4655DA4FBFC0E1108A8FD17B448A68554199C47D08FFB10D4B8"));

    crypto::Point<Field> G = curve.point(gx, gy);

    for (auto enc : {ser::Encoding::Base10, ser::Encoding::Base16, ser::Encoding::Base64}) {
        std::string pt_str = ser::serialize(G, enc);
        auto restored_pt = ser::deserialize_point<Field>(curve, pt_str, enc);
        ASSERT_EQ(G, restored_pt);

        std::string curve_str = ser::serialize<Field>(curve, enc);
        auto restored_curve = ser::deserialize_curve<Field>(curve_str, enc);
        ASSERT_EQ(curve, restored_curve);
    }
}

void test_fp512_extreme_serde() {
    static constexpr BigInt<512> p = (BigInt<512>(1) << 511) - BigInt<512>(187);
    using Fp512 = field::Fp<p>;

    BigInt<512> val = p - BigInt<512>(1);
    Fp512 elem(val);

    for (auto enc : {ser::Encoding::Base10, ser::Encoding::Base16, ser::Encoding::Base64}) {
        std::string s = ser::serialize(elem, enc);
        Fp512 restored = ser::deserialize<Fp512>(s, enc);
        ASSERT_EQ(elem, restored);
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
	test_fp_large_serde();
	test_fp256_secp256k1_serde();
	test_fpk256_k2_serde();
	test_secp256k1_point_serde();
	test_fp512_extreme_serde();
}
