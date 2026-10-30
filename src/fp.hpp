#pragma once

#include<iostream>
#include<cstdint>

#include"utils.hpp"

namespace field{

template<auto Modulus>
//requires(utils::is_prime<decltype(Modulus)::bits>(Modulus))
class Fp{
public:
	using bint_t = std::decay_t<decltype(Modulus)>;
	static constexpr bint_t characteristic = Modulus;
	static constexpr int bit_count = bint_t::bits;

	constexpr Fp() noexcept : value_(0) {}

	template<typename T>
	requires std::is_integral_v<T>
	explicit constexpr Fp(T val) noexcept{
		if constexpr(bint_t::bits < 64){
			uint64_t mod_val = Modulus.get_word(0);
			int64_t m = static_cast<int64_t>(mod_val);
			int64_t v = static_cast<int64_t>(val) % m;
			if(v < 0) v += m;
			value_ = bint_t(static_cast<uint64_t>(v));
		}
		else{
			if constexpr(std::is_signed_v<T>){
				if(val < 0){
					int64_t v = static_cast<int64_t>(val);
					bint_t abs_v(static_cast<uint64_t>(-v));
					if(abs_v >= Modulus) abs_v = abs_v % Modulus;
					value_ = Modulus - abs_v;
					if(value_ == Modulus) value_ = bint_t(0);
					return;
				}
			}
			bint_t v(static_cast<uint64_t>(val));
			if(v >= Modulus) v = v % Modulus;
			value_ = v;
		}
	}

	constexpr Fp(bint_t val) noexcept
		: value_(val % Modulus){}

	constexpr Fp(uint64_t val) noexcept 
		: value_(bint_t(val) % Modulus){}

	constexpr const bint_t& value() const noexcept {
        return value_;
	}

	constexpr Fp& operator[](std::size_t i) noexcept{
		return *this;
	}

	constexpr const Fp& operator[](std::size_t i) const noexcept{
		return *this;
	}

	constexpr Fp& operator+=(const Fp& rhs) noexcept {
		bint_t diff = Modulus - rhs.value_;
		if(value_ >= diff){
			value_ -= diff;
		}
		else{
			value_ += rhs.value_;
		}
		return *this;
	}

	constexpr Fp& operator-=(const Fp& rhs) noexcept {
		if(value_ < rhs.value_){
			value_ = Modulus - (rhs.value_ - value_);
		}
		else{
			value_ -= rhs.value_;
		}
		return*this;
	}

	constexpr Fp& operator*=(const Fp& rhs) noexcept {
		using WideInt = BigInt<bit_count*2>;
		WideInt a_wide = cast<bit_count*2, bit_count>(value_);
		WideInt b_wide = cast<bit_count*2, bit_count>(rhs.value_);
		WideInt m_wide = cast<bit_count*2, bit_count>(Modulus);
		value_ = cast<bit_count, bit_count*2>(a_wide*b_wide % m_wide);
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
		if(value_.is_zero()) return*this;

		Fp res;
		res.value_ = Modulus - value_;
		return res;
	}

	constexpr bool operator==(const Fp& rhs) const noexcept = default;

	constexpr Fp inv() const{
		if(value_.is_zero()) throw std::domain_error("0 has no inverse in Fp");

		// fermats little theorem
		// a^(-1) = a^(p - 2)
		bint_t exp = Modulus - 2;
		Fp base = *this;
		Fp result(1ULL);

		while(!exp.is_zero()){
			if(exp.is_odd()){
				result *= base;
			}

			base *= base;
			exp >>= 1;
		}

		return result;
	}

private:
	bint_t value_{};
};

template<auto p>
requires(utils::is_prime<decltype(p)::bits>(p))
inline std::ostream& operator<<(std::ostream& os, const Fp<p>& x){
	return os << x.value();
}

} // namespace field
