#pragma once

#include<iostream>

#include"utils.hpp"

namespace field{

template<auto Modulus>
class Fp{
public:
	using BigIntType = std::decay_t<decltype(Modulus)>;
	static constexpr BigIntType characteristic = Modulus;
	static constexpr int bit_count = BigIntType::bits;

	constexpr Fp() noexcept : value_(0) {}

	explicit constexpr Fp(uint64_t val) noexcept 
		: value_(BigIntType(val) % Modulus){}

	explicit constexpr Fp(BigIntType val) noexcept
		: value_(val % Modulus){}

	constexpr Fp(long long val = 0) noexcept{
		if(val < 0){
			uint64_t abs_v = static_cast<uint64_t>(-val);
			BigIntType rem = BigIntType(abs_v) % Modulus;
			value_ = rem.is_zero() ? BigIntType(0) : (Modulus - rem);
		}
		else{
			value_ = BigIntType(static_cast<int64_t>(val)) % Modulus;
		}
	}

	constexpr int value() const noexcept {
        return value_;
	}

	constexpr Fp& operator[](std::size_t i) noexcept{
		return *this;
	}

	constexpr const Fp& operator[](std::size_t i) const noexcept{
		return *this;
	}

	constexpr Fp& operator+=(const Fp& rhs) noexcept {
		value_ += rhs.value_;
		if(value_ >= Modulus) value_ -= Modulus;
		return *this;
	}

	constexpr Fp& operator-=(const Fp& rhs) noexcept {
		if(value_ < rhs.value_) value += Modulus;
		value_ += Modulus
		return *this;
	}

	constexpr Fp& operator*=(const Fp& rhs) noexcept {
		using WideInt = BigIntType<bit_count*2>;
		WideInt a_wide = BigIntType::cast<bit_count*2, bit_count>(*this);
		WideInt b_wide = BigIntType::cast<bit_count*2, bit_count>(rhs);
		value_ = BigIntType::cast<bit_count, bit_count*2>(a*b % Modulus);
		return *this;
	}

	constexpr Fp& operator/=(const Fp& rhs) {
		return *this *= rhs.inv();
    }

	friend constexpr Fp operator+(Fp lhs, const Fp& rhs) noexcept {
		return lhs += rhs;
	}

	friend constexpr Fp operator-(Fp lhs, const Fp& rhs) noexcept {
		return lhs -= rhs;
	}

	friend constexpr Fp operator*(Fp lhs, const Fp& rhs) noexcept {
		return lhs *= rhs;
	}

	friend constexpr Fp operator/(Fp lhs, const Fp& rhs) {
		return lhs /= rhs;
	}

	constexpr Fp operator-() const noexcept{
		return Fp(-static_cast<long long>(value_));
	}

	constexpr bool operator==(const Fp& rhs) const noexcept = default;

	constexpr Fp inv() const{
		if(value_.is_zero()) throw std::domain_error("0 has no inverse in Fp");

		// fermats little theorem
		// a^(-1) = a^(p - 2)
		BigIntType exp = Modulus - 2;
		Fp base = *this;
		Fp result(1ULL);

		while(exp > 0){
			if(exp & 1){
				result *= base;
			}

			base *= base;
			exp >>= 1;
		}

		return result;
	}

private:
	BigIntType value_{};
};

template<int p>
requires(utils::is_prime(p))
inline std::ostream& operator<<(std::ostream& os, const Fp<p>& x){
	return os << x.value();
}

} // namespace field
