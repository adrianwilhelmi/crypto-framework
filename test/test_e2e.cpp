#include"fp.hpp"
#include"fpk.hpp"
#include"elliptic.hpp"
#include"bigint.hpp"
#include"test_framework.hpp"
#include"serializer.hpp"

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

void test_e2e(){
	constexpr BigInt<2> two(2);
	using Field163 = Fpk<two,163,Polynomial163>;

	Field163 a{1};
	Field163 b{1};
	crypto::EllipticCurve<Field163> curve(a,b);

	Field163 x;
	x[1] = 1;
	x[2] = 1;

	Field163 y = ser::deserialize<Field163>(
		"0x182d615946272a2282b6c436afce2ccf272a307803000000", 
		ser::Encoding::Base16
	);

	crypto::Point<Field163> point = curve.point(x,y);

    for (auto enc : {ser::Encoding::Base10, ser::Encoding::Base16, ser::Encoding::Base64}) {
		std::string point_str = ser::serialize(point, enc);
		std::cout << point_str << "\n";

		auto restored = ser::deserialize_point<Field163>(curve, point_str, enc);
		ASSERT_EQ(restored, point);
	}
}
