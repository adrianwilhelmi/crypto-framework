#include"bigint.hpp"
#include"fp.hpp"
#include"fpk.hpp"
#include"elliptic.hpp"

#include"test_framework.hpp"

void test_general_curve_fp(){
	constexpr BigInt<3> five(5);
	using F5 = field::Fp<five>;

	crypto::EllipticCurve<F5> curve(F5(2), F5(1));

	crypto::Point<F5> p = curve.point(F5(0), F5(1));
	crypto::Point<F5> inf = curve.infinity();

	// p + O = p
	ASSERT_EQ(curve.add(p, inf), p);
	ASSERT_EQ(curve.add(inf, p), p);

	// p + (-p) = 0
	crypto::Point<F5> neg_p = curve.neg(p);
	ASSERT_EQ(neg_p, curve.point(F5(0), F5(4))); // -1 mod 5 = 4
	ASSERT_EQ(curve.add(p, neg_p), inf);

	crypto::Point<F5> p2 = curve.add(p,p);
	crypto::Point<F5> p3 = curve.add(p2, p);
	ASSERT_EQ(curve.mul(p,3), p3);
	ASSERT_EQ(p3, curve.point(F5(3), F5(3)));

	ASSERT_EQ(curve.mul(p,7), inf);

	crypto::Point<F5> sub_res = curve.sub(p, p2);
	crypto::Point<F5> expected_sub = curve.add(p, curve.neg(p2));
	ASSERT_EQ(sub_res, expected_sub);
}

void test_general_curve_fpk(){
	constexpr BigInt<3> five(5);
	using Field = field::Fpk<five,2>;

	crypto::EllipticCurve<Field> curve(Field(1), Field(2));

	Field x({1,0});
	Field y({2,0});

	crypto::Point<Field> p = curve.point(x,y);

	crypto::Point<Field> p2 = curve.add(p,p);
	crypto::Point<Field> p3 = curve.add(p2,p);
	crypto::Point<Field> p6 = curve.mul(p,6);

	ASSERT_TRUE(curve.is_on_curve(p3));
	ASSERT_TRUE(curve.is_on_curve(p6));
	ASSERT_EQ(curve.add(p3,p3), p6);
}

void test_binary_curve(){
	constexpr BigInt<2> two(2);
	using Field = Fpk<two,4>;

	Field a;
	a[0] = 1;

	Field b;
	b[0] = 1;

	crypto::EllipticCurve<Field> curve(a,b);

	// x = 1
	Field x;
	x[0] = 1;

	// y = x + x^2
	Field y;
	y[1] = 1;
	y[2] = 1;

	crypto::Point<Field> p = curve.point(x,y);
	crypto::Point<Field> inf = curve.infinity();

	ASSERT_EQ(curve.add(p,inf),p);

	crypto::Point<Field> neg_p = curve.neg(p);
	ASSERT_EQ(neg_p.x(), x);
	ASSERT_EQ(neg_p.y(), x+y);
	ASSERT_EQ(curve.add(p,neg_p), inf);

	crypto::Point<Field> p2 = curve.add(p,p);

	ASSERT_TRUE(curve.is_on_curve(p2));
	ASSERT_EQ(curve.mul(p,2), p2);

	crypto::Point<Field> p10 = curve.mul(p,10);
	ASSERT_TRUE(curve.is_on_curve(p10));

	// ord p is 4 -> 10p = 2p
	ASSERT_EQ(p10, p2);
}

void test_curve_exceptions_and_edge_cases(){
	constexpr BigInt<3> five(5);
	using Field = Fp<five>;

	bool caught_singular = false;
	try{
		[[maybe_unused]] crypto::EllipticCurve<Field> singular_curve(Field(0),Field(0));
	} catch(const std::invalid_argument&){
		caught_singular = true;
	}
	ASSERT_TRUE(caught_singular);

	crypto::EllipticCurve<Field> curve(2,1);
	bool caught_invalid_point = false;
	try{
		[[maybe_unused]] auto invalid_p = curve.point(1,1);
	} catch(const std::invalid_argument&){
		caught_invalid_point = true;
	}
	ASSERT_TRUE(caught_invalid_point);

	bool caught_inf_access = false;
	try{
		auto inf = curve.infinity();
		[[maybe_unused]] auto x_val = inf.x();
	} catch(const std::logic_error&){
		caught_inf_access = true;
	}
	ASSERT_TRUE(caught_inf_access);
}

void test_general_curve_group_laws_fp(){
	constexpr BigInt<5> p(17);
	using F = field::Fp<p>;

	crypto::EllipticCurve<F> curve(2,1);
	const auto O = curve.infinity();

	std::vector<crypto::Point<F>> points;
	points.push_back(O);

	for(int xi = 0; xi < p; ++xi){
		for(int yi = 0; yi < p; ++yi){
			const F x(xi);
			const F y(yi);

			if(y*y == x*x*x + F(2)*x + F(1)){
				points.push_back(curve.point(x,y));
			}
		}
	}

	for(const auto& p : points){
		ASSERT_TRUE(curve.is_on_curve(p));
		ASSERT_EQ(curve.add(p, O), p);
		ASSERT_EQ(curve.add(O, p), p);
		ASSERT_EQ(curve.add(p, curve.neg(p)), O);

		ASSERT_EQ(curve.mul(p,0), O);
		ASSERT_EQ(curve.mul(p,1), p);
		ASSERT_EQ(curve.mul(p,2), curve.add(p,p));
	}

	for(const auto& p : points){
		for(const auto& q : points){
			const auto sum = curve.add(p,q);

			ASSERT_EQ(sum, curve.add(q,p));
			ASSERT_TRUE(curve.is_on_curve(sum));

			for(const auto& r : points){
				ASSERT_EQ(
					curve.add(curve.add(p,q), r),
					curve.add(p, curve.add(q,r))
				);
			}
		}
	}
}

void test_binary_curve_exceptions(){
	constexpr BigInt<2> two(2);
	using F = field::Fpk<two,4>;

	F a;
	a[0] = 1;

	F zero;
	bool caught_singular = false;

	try{
		[[maybe_unused]] crypto::EllipticCurve<F> invalid_curve(a,zero);
	} catch(const std::invalid_argument&){
		caught_singular = true;
	}

	ASSERT_TRUE(caught_singular);

	F b;
	b[0] = 1;

	crypto::EllipticCurve<F> curve(a,b);

	bool caught_invalid_point = false;

	try{
		[[maybe_unused]] auto invalid = curve.point(zero,zero);
	} catch(const std::invalid_argument&){
		caught_invalid_point = true;
	}

	ASSERT_TRUE(caught_invalid_point);

	bool caught_inf_x = false;
	try{
		[[maybe_unused]] auto x = curve.infinity().x();
	} catch(const std::logic_error&){
		caught_inf_x = true;
	}
	ASSERT_TRUE(caught_inf_x);

	bool caught_inf_y = false;
	try{
		[[maybe_unused]] auto y = curve.infinity().y();
	} catch(const std::logic_error&){
		caught_inf_y = true;
	}
	ASSERT_TRUE(caught_inf_y);
}

void tests_elliptic(){
	test_general_curve_fp();
	test_general_curve_fpk();
	test_binary_curve();
	test_curve_exceptions_and_edge_cases();
	test_general_curve_group_laws_fp();
	test_binary_curve_exceptions();
}
