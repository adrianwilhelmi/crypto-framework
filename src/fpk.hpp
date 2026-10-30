#pragma once

#include<array>
#include<cstddef>
#include<cstdint>
#include<stdexcept>
#include<vector>

#include"utils.hpp"
#include"polynomial_utils.hpp"
#include"fp.hpp"

namespace field{

template<int p, int k>
struct AutoIrreducible{
	static constexpr std::array<Fp<p>, k+1> value =
		utils::find_first_irreducible<p, static_cast<std::size_t>(k)>();
};


template<int p, int k, typename IrreduciblePolynomial>
requires(utils::is_prime(p))
class FpkImpl;

template<int k, typename IrreduciblePolynomial>
requires(k>1)
class FpkBinaryImpl;


template<int p, int k, typename IP>
struct FpkSelector{
	using type = FpkImpl<p,k,IP>;
};

template<int p, typename IP>
struct FpkSelector<p,1,IP>{
	using type = Fp<p>;
};

template<int k, typename IP>
struct FpkSelector<2,k,IP>{
	using type = FpkBinaryImpl<k,IP>;
};

template<typename IP>
struct FpkSelector<2,1,IP>{
	using type = Fp<2>;
};

template<int p, int k, typename IP = AutoIrreducible<p,k>>
requires(utils::is_prime(p) && k >= 1)
using Fpk = typename FpkSelector<p,k,IP>::type;



template<int p, int k, typename IrreduciblePolynomial = AutoIrreducible<p,k>>
requires(utils::is_prime(p))
class FpkImpl{
public:
	static constexpr int characteristic = p;

	using BaseFp = Fp<p>;

	constexpr FpkImpl() noexcept {
		element_.fill(BaseFp(0));
	}

	constexpr FpkImpl(const std::array<BaseFp, k>& coeffs) noexcept : element_(coeffs) {}

	constexpr FpkImpl(BaseFp val) noexcept{
		element_.fill(BaseFp(0));
		element_[0] = val;
	}

	constexpr BaseFp& operator[](std::size_t i) noexcept {
		return element_[i];
	}

	constexpr const BaseFp& operator[](std::size_t i) const noexcept {
		return element_[i];
	}

	constexpr FpkImpl operator-() const noexcept{
		FpkImpl res;
		for(std::size_t i = 0; i < k; ++i){
			res[i] = -element_[i];
		}
		return res;
	}

	constexpr FpkImpl& operator+=(const FpkImpl& rhs) noexcept{
		for(std::size_t i = 0; i < k; ++i){
			element_[i] += rhs[i];
		}
		return *this;
	}

	constexpr FpkImpl& operator-=(const FpkImpl& rhs) noexcept{
		for(std::size_t i = 0; i < k; ++i){
			element_[i] -= rhs[i];
		}
		return *this;
	}

	constexpr FpkImpl& operator*=(const FpkImpl& rhs) noexcept{
		// assuming that irreducible polynomial is monic
		std::array<BaseFp, 2*k-1> product{};

		for(std::size_t i = 0; i < k; ++i){
			for(std::size_t j = 0; j < k; ++j){
				product[i+j] += element_[i] * rhs[j];
			}
		}

		for(int d = 2 * k - 2; d >= k; --d){
			const BaseFp factor = product[d];
			if(factor == BaseFp(0)) continue;

			for(int i = 0; i <= k; ++i){
				product[d-k+i] -= factor * irred_[i];
			}
		}

		for(std::size_t i = 0; i < k; ++i){
			element_[i] = product[i];
		}

		return *this;
	}

	constexpr FpkImpl pow(std::uint64_t exponent) const{
		FpkImpl base = *this;
		FpkImpl result(BaseFp(1));

		while(exponent > 0){
			if(exponent & 1ULL) result *= base;
			exponent >>= 1ULL;
			if(exponent > 0) base *= base;
		}

		return result;
	}

	constexpr FpkImpl inv() const{
		if(is_zero()){
			throw std::domain_error("div by 0 in FpkImpl");
		}

		// a^(-1) = a^(p^k - 2)
		FpkImpl result = pow(static_cast<std::uint64_t>(p-2));
		FpkImpl power = pow(static_cast<std::uint64_t>(p));

		for(int i = 1; i < k; ++i){
			result *= power.pow(static_cast<std::uint64_t>(p-1));

			if(i+1 < k){
				power = power.pow(static_cast<std::uint64_t>(p));
			}
		}

		return result;
	}

	constexpr FpkImpl& operator/=(const FpkImpl& rhs){
		return *this *= rhs.inv();
	}

	friend constexpr FpkImpl operator+(FpkImpl lhs, const FpkImpl& rhs){
		lhs += rhs;
		return lhs;
	}

	friend constexpr FpkImpl operator-(FpkImpl lhs, const FpkImpl& rhs){
		lhs -= rhs;
		return lhs;
	}

	friend constexpr FpkImpl operator*(FpkImpl lhs, const FpkImpl& rhs){
		lhs *= rhs;
		return lhs;
	}

	friend constexpr FpkImpl operator/(FpkImpl lhs, const FpkImpl& rhs){
		lhs /= rhs;
		return lhs;
	}

	constexpr bool operator==(const FpkImpl& rhs) const noexcept{
		for(std::size_t i = 0; i < k; ++i){
			if(element_[i] != rhs[i]) return false;
		}
		return true;
	}

	constexpr bool operator!=(const FpkImpl& rhs) const noexcept{return !(*this==rhs);}


	constexpr bool is_zero() const noexcept{
		for(const auto& coeff : element_){
			if(coeff != BaseFp(0)){
				return false;
			}
		}
		return true;
	}

	static constexpr const std::array<BaseFp,k+1>& modulus() noexcept{
		return irred_;
	}

private:
	std::array<BaseFp, k> element_;
	inline static constexpr std::array<BaseFp,k+1> irred_ = IrreduciblePolynomial::value;
};



// partial specialization for k = 2
template<int k, typename IrreduciblePolynomial = AutoIrreducible<2,k>>
requires(k>1)
class FpkBinaryImpl{
public:
	static constexpr int characteristic = 2;

	using BaseFp = Fp<2>;

	static constexpr std::size_t word_count = static_cast<std::size_t>((k+63)/64);

	struct BitRef{
		std::uint64_t& word;
		std::size_t bit_idx;

		constexpr BitRef& operator=(const BitRef&rhs) noexcept{
			return *this = static_cast<BaseFp>(rhs);
		}

		constexpr BitRef& operator=(BaseFp val) noexcept{
			if(val.value() == 1) word |= (1ULL << bit_idx);
			else word &= ~(1ULL << bit_idx);
			return *this;
		}

		constexpr BitRef& operator=(int val) noexcept{
			return *this = BaseFp(val);
		}

		constexpr operator BaseFp() const noexcept{
			return BaseFp(static_cast<int>((word >> bit_idx) & 1ULL));
		}

		constexpr bool operator==(int val) const noexcept{
			return static_cast<BaseFp>(*this) == BaseFp(val);
		}

		constexpr bool operator!=(int val) const noexcept{
			return !(*this == val);
		}
	};

	constexpr FpkBinaryImpl() noexcept{
		words_.fill(0);
	}

	constexpr FpkBinaryImpl(const std::array<BaseFp,k>& coeffs) noexcept{
		for(std::size_t i = 0; i < k; ++i){
			if(coeffs[i].value() == 1){
				words_[i/64] |= (1ULL << (i%64));
			}
		}
	}

	constexpr FpkBinaryImpl(BaseFp val) noexcept{
		words_.fill(0);
		if(val.value() == 1) words_[0] = 1ULL;
	}

	constexpr BitRef operator[](std::size_t i) noexcept {
		return BitRef{words_[i/64], i%64};
	}
	constexpr BaseFp operator[](std::size_t i) const noexcept {
		return BaseFp(
			static_cast<int>((words_[i/64] >> (i%64)) & 1ULL)
		);
	}

	// -a = a
	constexpr FpkBinaryImpl operator-() const noexcept {return *this;}

	constexpr FpkBinaryImpl& operator+=(const FpkBinaryImpl& rhs) noexcept{
		for(std::size_t i = 0; i < word_count; ++i){
			words_[i] ^= rhs.words_[i];
		}
		return *this;
	}

	constexpr FpkBinaryImpl& operator-=(const FpkBinaryImpl& rhs) noexcept{
		return *this += rhs;
	}

	// shift and xor
	constexpr FpkBinaryImpl& operator*=(const FpkBinaryImpl& rhs) noexcept{
		FpkBinaryImpl res;
		FpkBinaryImpl a = *this;

		// A,B - polynomials
		// A(x) B(x) = sum A(x)*b_i*x^i

		for(std::size_t i = 0; i < static_cast<std::size_t>(k); ++i){
			if(rhs[i] == 1) res += a;
			a.shift_left_and_reduce();
		}

		*this=res;
		return *this;
	}

	constexpr FpkBinaryImpl inv() const{
		if(is_zero()) throw std::domain_error("div by 0 in FpkBinaryImpl<2,k>");

		FpkBinaryImpl result(BaseFp(1));
		FpkBinaryImpl power = *this;

		for(int i = 1; i < k; ++i){
			power *= power;
			result *= power;
		}

		return result;
	}

	constexpr FpkBinaryImpl pow(std::uint64_t exponent) const{
		FpkBinaryImpl base = *this;
		FpkBinaryImpl result(BaseFp(1));

		while(exponent > 0){
			if(exponent & 1ULL) result *= base;
			exponent >>= 1ULL;
			if(exponent > 0) base *= base;
		}

		return result;
	}

	constexpr FpkBinaryImpl& operator/=(const FpkBinaryImpl& rhs) {
		return *this *= rhs.inv();
	}

	friend constexpr FpkBinaryImpl operator+(
			FpkBinaryImpl lhs,
			const FpkBinaryImpl& rhs){
		return lhs+=rhs;
	}

	friend constexpr FpkBinaryImpl operator-(
			FpkBinaryImpl lhs,
			const FpkBinaryImpl& rhs){
		return lhs+=rhs;
	}

	friend constexpr FpkBinaryImpl operator*(
			FpkBinaryImpl lhs,
			const FpkBinaryImpl& rhs){
		return lhs*=rhs;
	}

	friend constexpr FpkBinaryImpl operator/(
			FpkBinaryImpl lhs,
			const FpkBinaryImpl& rhs){
		return lhs/=rhs;
	}

	constexpr bool operator==(const FpkBinaryImpl& rhs) const noexcept{
		return words_ == rhs.words_;
	}
	constexpr bool operator!=(const FpkBinaryImpl& rhs) const noexcept{
		return words_ != rhs.words_;
	}

	constexpr bool is_zero() const noexcept{
		for(std::size_t i = 0; i < word_count; ++i){
			if(words_[i] != 0) return false;
		}
		return true;
	}

	static constexpr const std::array<BaseFp,k+1>& modulus() noexcept{
		return IrreduciblePolynomial::value;
	}

private:
	std::array<std::uint64_t, word_count> words_{};

	static constexpr std::uint64_t top_word_mask() noexcept{
		constexpr std::size_t rem = k % 64;
		if constexpr(rem == 0) return ~0ULL;
		else return (1ULL << rem) - 1ULL;
	}

	static constexpr std::array<std::uint64_t, word_count> get_irred_words() noexcept{
		std::array<std::uint64_t, word_count> res{};
		const auto& irred = IrreduciblePolynomial::value;
		for(int i = 0; i < k; ++i){
			if(irred[i].value() == 1){
				res[i / 64] |= (1ULL << (i % 64));
			}
		}
		return res;
	}

	// 1 bitshift into left and reduce modulo irreducible polynomial
	constexpr void shift_left_and_reduce() noexcept{
		bool msb = ((*this)[k-1] == 1);
		std::uint64_t carry = 0;

		for(std::size_t i = 0; i < word_count; ++i){
			std::uint64_t next_carry = words_[i] >> 63;
			words_[i] = (words_[i] << 1) | carry;
			carry = next_carry;
		}

		words_[word_count - 1] &= top_mask_;

		if(msb){
			for(std::size_t i = 0; i < word_count; ++i){
				words_[i] ^= irred_words_[i];
			}
		}
	}

	static constexpr std::uint64_t top_mask_ = top_word_mask();
	static constexpr std::array<std::uint64_t, word_count> irred_words_ = 
		get_irred_words();
};

} // namespace field

template<int p, int k, typename IP = AutoIrreducible<p,k>>
requires(utils::is_prime(p))
inline std::ostream& operator<<(std::ostream& os, const field::FpkImpl<p,k,IP>& x){
	for(std::size_t i = 0; i < k; ++i){
		if(i > 0) os << ' ';
		os << x[i];
	}
	return os;
}

template<int k, typename IP>
requires(k > 1)
inline std::ostream& operator<<(std::ostream& os, const field::FpkBinaryImpl<k,IP>& x){
	for(std::size_t i = 0; i < k; ++i){
		if(i > 0) os << ' ';
		os << x[i];
	}
	return os;
}

template<int p, std::size_t N>
requires(utils::is_prime(p))
inline std::ostream& operator<<(std::ostream& os, const std::array<Fp<p>,N>& x){
	os << '[';
	for(std::size_t i = 0; i < N; ++i){
		if(i > 0) os << ", ";
		os << x[i];
	}
	return os << ']';
}

