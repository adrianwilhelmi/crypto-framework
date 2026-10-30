#pragma once

#include<stdexcept>
#include<utility>
#include<iostream>

using namespace field;

namespace crypto{

template<typename Field> class EllipticCurveGeneral;
template<typename Field> class EllipticCurveBinary;
template<typename Field> class EllipticCurveChar3;

template<typename Field>
class Point{
public:
	constexpr Point() noexcept = default;

	constexpr bool is_infinity() const noexcept{
		return infinity_;
	}

	constexpr const Field& x() const{
		if(infinity_) 
			throw std::logic_error("point at inf has not affine x-coord");
		return x_;
	}
	constexpr const Field& y() const{
		if(infinity_) 
			throw std::logic_error("point at inf has not affine y-coord");
		return y_;
	}

	friend constexpr bool operator==(
			const Point& lhs,
			const Point& rhs) noexcept{
		if(lhs.infinity_ || rhs.infinity_){
			return lhs.infinity_ == rhs.infinity_;
		}
		return lhs.x_ == rhs.x_ && lhs.y_ == rhs.y_;
	}

	friend constexpr bool operator!=(
			const Point& lhs,
			const Point& rhs) noexcept{
		return !(lhs==rhs);
	}

private:
	Field x_{};
	Field y_{};
	bool infinity_{true};

	constexpr Point(Field x, Field y)
		: x_(std::move(x)), y_(std::move(y)), infinity_(false) {}

	friend class EllipticCurveGeneral<Field>;
	friend class EllipticCurveBinary<Field>;
	friend class EllipticCurveChar3<Field>;
};

template<typename Field>
inline std::ostream& operator<<(std::ostream& os, const Point<Field>& p){
	if(p.is_infinity()) return os << "infinity";
	return os << "(" << p.x() << ", " << p.y() << ")";
}

template<typename Field, auto Char = Field::characteristic>
struct EllipticCurveSelector{
	using type = EllipticCurveGeneral<Field>;
};

template<typename Field, auto Char>
requires(Char==2)
struct EllipticCurveSelector<Field,Char>{
	using type = EllipticCurveBinary<Field>;
};

template<typename Field>
using EllipticCurve = typename EllipticCurveSelector<Field>::type;


// implementation for p > 3
template<typename Field>
class EllipticCurveGeneral{
	static_assert(Field::characteristic > 3, "general short Weistrass requires characteristic p > 3");
public:
	constexpr EllipticCurveGeneral(Field a, Field b) : a_(std::move(a)), b_(std::move(b)){
		if(Field(4) * a_ * a_ * a_ + Field(27) * b_ * b_ == Field(0))
			throw std::invalid_argument("singular elliptic curve: 4a^3 + 27b^2 = 0");
	}

	constexpr const Field& a() const noexcept {return a_;}
	constexpr const Field& b() const noexcept {return b_;}

	constexpr Point<Field> infinity() const noexcept {return Point<Field>{};}

	constexpr Point<Field> point(Field x, Field y) const{
		Point<Field> result(std::move(x), std::move(y));
		if(!is_on_curve(result))
			throw std::invalid_argument("point does not belong to this curve");
		return result;
	}

	constexpr bool is_on_curve(const Point<Field>& p) const noexcept{
		if(p.infinity_) return true;
		return p.y_ * p.y_ == p.x_ * p.x_ * p.x_ + a_ * p.x_ + b_;
	}

	constexpr Point<Field> neg(const Point<Field>& p) const{
		if(p.infinity_) return p;
		return Point<Field>(p.x_, -p.y_);
	}

	constexpr Point<Field> add(const Point<Field>& p, const Point<Field>& q) const{
		if(p.infinity_) return q;
		if(q.infinity_) return p;

		const Field zero(0);
		Field lambda(0);

		if(p.x_ == q.x_){
			if(p.y_ + q.y_ == zero) return infinity();
			lambda = (Field(3) * p.x_ * p.x_ + a_) / (Field(2) * p.y_);
		}
		else{
			lambda = (q.y_ - p.y_) / (q.x_ - p.x_);
		}

		const Field rx = lambda * lambda - p.x_ - q.x_;
		const Field ry = lambda * (p.x_ - rx) - p.y_;

		return Point<Field>(rx,ry);
	}

	constexpr Point<Field> sub(const Point<Field>& p, const Point<Field>& q) const{
		return add(p,neg(q));
	}

	// double and add method
	constexpr Point<Field> mul(const Point<Field>& p, std::uint64_t scalar) const{
		Point<Field> result = infinity();
		Point<Field> base = p;

		while(scalar > 0){
			if(scalar & 1ULL) result = add(result, base);
			scalar >>= 1ULL;
			if(scalar > 0) base = add(base, base);
		}

		return result;
	}

	constexpr bool operator==(const EllipticCurve<Field>& rhs) const noexcept{
		return (a_==rhs.a_ && b_==rhs.b_);
	}

private:
	Field a_;
	Field b_;

};

template<typename Field>
class EllipticCurveBinary{
	static_assert(Field::characteristic==2, "binary field should have char=2");
public:
	constexpr EllipticCurveBinary(Field a, Field b) 
			: a_(std::move(a)), b_(std::move(b)){
		if(b_ == Field(0)) throw std::invalid_argument("b cannot be 0");
	}

	constexpr const Field& a() const noexcept {return a_;}
	constexpr const Field& b() const noexcept {return b_;}

	constexpr Point<Field> infinity() const noexcept {return Point<Field>{};}

	constexpr Point<Field> point(Field x, Field y) const{
		Point<Field> result(std::move(x), std::move(y));
		if(!is_on_curve(result)) throw std::invalid_argument("point not on the curve");
		return result;
	}

	constexpr bool is_on_curve(const Point<Field>& p) const noexcept{
		if(p.infinity_) return true;
		return p.y_ * p.y_ + p.x_ * p.y_ == p.x_ * p.x_ * p.x_ + a_ * p.x_ * p.x_ + b_;
	}

	constexpr Point<Field> neg(const Point<Field>& p) const{
		if(p.infinity_) return p;
		return Point<Field>(p.x_, p.x_ + p.y_);
	}

	constexpr Point<Field> add(const Point<Field>& p, const Point<Field>& q) const{
		if(p.infinity_) return q;
		if(q.infinity_) return p;

		if(p.x_ == q.x_){
			if(p.y_ != q.y_ || p.x_ == Field(0)) return infinity();
		}

		Field lambda(0);
		Field rx, ry;

		if(p == q){
			lambda = p.x_ + p.y_ / p.x_;
			rx = lambda * lambda + lambda + a_;
			ry = p.x_ * p.x_ + (lambda + Field(1)) * rx;
		}
		else{
			lambda = (p.y_ + q.y_) / (p.x_ + q.x_);
			rx = lambda * lambda + lambda + p.x_ + q.x_ + a_;
			ry = lambda * (p.x_ + rx) + rx + p.y_;
		}

		return Point<Field>(rx,ry);
	}

	constexpr Point<Field> sub(const Point<Field>& p, const Point<Field>& q) const{
		return add(p, neg(q));
	}

	constexpr Point<Field> mul(const Point<Field>& p, std::uint64_t scalar) const{
		Point<Field> result = infinity();
		Point<Field> base = p;

		while(scalar > 0){
			if(scalar & 1ULL) result = add(result, base);
			scalar >>= 1ULL;
			if(scalar > 0) base = add(base,base);
		}

		return result;
	}

	constexpr bool operator==(const EllipticCurve<Field>& rhs) const noexcept{
		return (a_==rhs.a_ && b_==rhs.b_);
	}


private:
	Field a_;
	Field b_;

};


} // namespace crypto
