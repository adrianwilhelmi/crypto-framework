#pragma once

#include<array>
#include<iostream>
#include<cstddef>
#include<cstdint>
#include<bit>
#include<compare>
#include<string_view>
#include<iomanip>

template<uint64_t N>
class BigInt{
public:
	static constexpr int bits = N;

	constexpr BigInt() noexcept{
		words_.fill(0);
	}

	constexpr BigInt(const uint64_t val) noexcept{
		words_.fill(0);
		words_[0] = val;
		words_[word_count-1] &= top_mask_;
	}

	constexpr BigInt(const std::array<uint64_t, (N+63)/64>& words) noexcept
		: words_(words){
		words_[word_count-1] &= top_mask_;
	}

	static constexpr BigInt from_hex(std::string_view hex) noexcept{
		BigInt res;
		if(hex.starts_with("0x") || hex.starts_with("0X")){
			hex.remove_prefix(2);
		}
		for(char c : hex){
			res <<= 4;
			uint64_t val = 0;
			if(c >= '0' && c <= '9')	val = c - '0';
			else if(c >= 'a' && c <= 'f') val = c - 'a' + 10;
			else if(c >= 'A' && c <= 'F') val = c - 'A' + 10;
			res += BigInt(val);
		}
		return res;
	}

	constexpr uint64_t get_word(const std::size_t i) const noexcept{
		return words_[i];
	}

	constexpr void set_word(const std::size_t i, const uint64_t val) noexcept{
		this->words_[i] = val;
		words_[word_count - 1] &= top_mask_;
	}

	constexpr bool get_bit(std::size_t bit_idx) const noexcept{
		if(bit_idx >= static_cast<std::size_t>(N)) return false;
		return (words_[bit_idx / 64] >> (bit_idx % 64)) & 1ULL;
	}

	constexpr void set_bit(std::size_t bit_idx, bool val) noexcept{
		if(bit_idx >= static_cast<std::size_t>(N)) return;
		if(val){
			words_[bit_idx / 64] |= (1ULL << (bit_idx % 64));
		}
		else{
			words_[bit_idx / 64] &= ~(1ULL << (bit_idx % 64));
		}
		words_[word_count-1] &= top_mask_;
	}

	constexpr std::size_t bit_length() const noexcept{
		for(std::size_t i = word_count; i-- > 0;){
			if(words_[i] != 0) return i*64 + (64 - std::countl_zero(words_[i]));
		}
		return 0;
	}

	constexpr bool is_zero() const noexcept{
		for(std::size_t i = 0; i < word_count; ++i){
			if(words_[i] != 0) return false;
		}
		return true;
	}

	constexpr bool is_even() const noexcept {return (words_[0] & 1ULL) == 0;}
	constexpr bool is_odd() const noexcept {return (words_[0] & 1ULL) != 0;}

	constexpr bool operator==(const BigInt&) const = default;
	constexpr auto operator<=>(const BigInt& rhs) const noexcept{
		for(std::size_t i = word_count; i-- > 0;){
			if(words_[i] != rhs.words_[i]){
				return words_[i] <=> rhs.words_[i];
			}
		}
		return std::strong_ordering::equal;
	}

	constexpr BigInt operator~() const noexcept{
		BigInt res;
		for(std::size_t i = 0; i < word_count; ++i){
			res.words_[i] = ~words_[i];
		}
		res.words_[word_count - 1] &= top_mask_;
		return res;
	}

	constexpr BigInt& operator&=(const BigInt& rhs) noexcept{
		for(std::size_t i = 0; i < word_count; ++i) words_[i] &= rhs.words_[i];
		return *this;
	}

	constexpr BigInt& operator|=(const BigInt& rhs) noexcept{
		for(std::size_t i = 0; i < word_count; ++i) words_[i] |= rhs.words_[i];
		return *this;
	}

	constexpr BigInt& operator^=(const BigInt& rhs) noexcept{
		for(std::size_t i = 0; i < word_count; ++i) words_[i] ^= rhs.words_[i];
		return *this;
	}

	constexpr BigInt& operator<<=(std::size_t shift) noexcept{
		if(shift == 0) return*this;
		if(shift >= static_cast<std::size_t>(N)){
			words_.fill(0);
			return*this;
		}

		std::size_t word_shift = shift / 64;
		std::size_t bit_shift = shift % 64;

		if(word_shift > 0){
			for(std::size_t i = word_count; i-- > word_shift;){
				words_[i] = words_[i-word_shift];
			}
			for(std::size_t i = 0; i < word_shift; ++i){
				words_[i] = 0;
			}
		}

		if(bit_shift > 0){
			uint64_t carry = 0;
			for(std::size_t i = 0; i < word_count; ++i){
				uint64_t next_carry = words_[i] >> (64 - bit_shift);
				words_[i] = (words_[i] << bit_shift) | carry;
				carry = next_carry;
			}
		}

		words_[word_count - 1] &= top_mask_;
		return*this;
	}

	constexpr BigInt& operator>>=(std::size_t shift) noexcept{
		if(shift == 0) return*this;
		if(shift >= static_cast<std::size_t>(N)){
			words_.fill(0);
			return*this;
		}

		std::size_t word_shift = shift / 64;
		std::size_t bit_shift = shift % 64;

		if(word_shift > 0){
			for(std::size_t i = 0; i + word_shift < word_count; ++i){
				words_[i] = words_[i+word_shift];
			}
			for(std::size_t i = word_count - word_shift; i < word_count; ++i){
				words_[i] = 0;
			}
		}

		if(bit_shift > 0){
			uint64_t carry = 0;
			for(std::size_t i = word_count; i-- > 0;){
				uint64_t next_carry = words_[i] << (64-bit_shift);
				words_[i] = (words_[i] >> bit_shift) | carry;
				carry = next_carry;
			}
		}

		words_[word_count - 1] &= top_mask_;
		return*this;
	}


	constexpr BigInt& operator+=(const BigInt& rhs) noexcept{
		uint64_t carry = 0;
		for(std::size_t i = 0; i < word_count; ++i){
			uint64_t a = words_[i];
			uint64_t b = rhs.words_[i];
			uint64_t sum = a + b + carry;
			carry = (sum < a) || (sum == a && carry > 0) ? 1 : 0;
			words_[i] = sum;
		}
		words_[word_count - 1] &= top_mask_;
		return*this;
	}

	constexpr BigInt& operator-=(const BigInt& rhs) noexcept{
		uint64_t borrow = 0;
		for(std::size_t i = 0; i < word_count; ++i){
			uint64_t a = words_[i];
			uint64_t b = rhs.words_[i];
			uint64_t diff = a - b - borrow;
			borrow = (a < b) || (a == b && borrow > 0) ? 1 : 0;
			words_[i] = diff;
		}
		words_[word_count - 1] &= top_mask_;
		return*this;
	}

	constexpr BigInt& operator*=(const BigInt& rhs) noexcept{
		BigInt res;
		for(std::size_t i = 0; i < word_count; ++i){
			if(words_[i] == 0) continue;
			uint64_t carry = 0;
			for(std::size_t j = 0; i + j < word_count; ++j){
				uint64_t a = words_[i], b = rhs.words_[j];
				uint64_t a_lo = a & 0xFFFFFFFFULL, a_hi = a >> 32;
				uint64_t b_lo = b & 0xFFFFFFFFULL, b_hi = b >> 32;

				uint64_t p0 = a_lo * b_lo;
				uint64_t p1 = a_lo * b_hi;
				uint64_t p2 = a_hi * b_lo;
				uint64_t p3 = a_hi * b_hi;

				uint64_t low_mid = (p0>>32) 
					+ (p1&0xFFFFFFFFULL) 
					+ (p2&0xFFFFFFFFULL);
				uint64_t prod_lo = (p0 & 0xFFFFFFFFULL) | (low_mid << 32);
				uint64_t prod_hi = p3 + (p1 >> 32) + (p2 >> 32) + (low_mid >> 32);

				uint64_t cur = res.words_[i+j];
				uint64_t sum1 = cur + prod_lo;
				uint64_t c1 = (sum1 < cur) ? 1 : 0;

				uint64_t sum2 = sum1+carry;
				uint64_t c2 = (sum2 < sum1) ? 1 : 0;

				res.words_[i+j] = sum2;
				carry = prod_hi + c1 + c2;
			}
		}
		res.words_[word_count - 1] &= top_mask_;
		*this = res;
		return *this;
	}

	static constexpr void div_rem(
			BigInt num, const BigInt& den, BigInt& quo, BigInt& rem) noexcept{
		quo = BigInt(0);
		rem = BigInt(0);
		if(den.is_zero()) return;

		std::size_t len = num.bit_length();
		for(std::size_t i = len; i-- > 0;){
			rem <<= 1;
			if(num.get_bit(i)) rem.set_bit(0,true);
			if(rem >= den){
				rem -= den;
				quo.set_bit(i,true);
			}
		}
	}

	constexpr BigInt operator/(const BigInt& rhs) const noexcept{
		BigInt quo, rem;
		div_rem(*this, rhs, quo, rem);
		return quo;
	}

	constexpr BigInt operator%(const BigInt rhs) const noexcept{
		BigInt quo, rem;
		div_rem(*this, rhs, quo, rem);
		return rem;
	}

	friend constexpr BigInt operator+(BigInt a, const BigInt& b) noexcept {return a += b;}
	friend constexpr BigInt operator-(BigInt a, const BigInt& b) noexcept {return a -= b;}
	friend constexpr BigInt operator*(BigInt a, const BigInt& b) noexcept {return a *= b;}
	friend constexpr BigInt operator&(BigInt a, const BigInt& b) noexcept {return a &= b;}
	friend constexpr BigInt operator|(BigInt a, const BigInt& b) noexcept {return a |= b;}
	friend constexpr BigInt operator^(BigInt a, const BigInt& b) noexcept {return a ^= b;}
	friend constexpr BigInt operator<<(BigInt a, std::size_t s) noexcept {return a <<= s;}
	friend constexpr BigInt operator>>(BigInt a, std::size_t s) noexcept {return a >>= s;}

	//scalar arithmetic
	constexpr BigInt& operator+=(uint64_t rhs) noexcept{
		if(rhs==0) return*this;
		uint64_t carry = rhs;
		for(std::size_t i = 0; i < word_count && carry > 0; ++i){
			uint64_t sum = words_[i] + carry;
			carry = (sum < words_[i]) ? 1 : 0;
			words_[i] = sum;
		}
		words_[word_count-1] &= top_mask_;
		return*this;
	}

	constexpr BigInt& operator-=(uint64_t rhs) noexcept{
		if(rhs==0) return*this;
		uint64_t borrow = rhs;
		for(std::size_t i = 0; i < word_count && borrow > 0; ++i){
			uint64_t prev = words_[i];
			words_[i] = prev - borrow;
			borrow = (prev < borrow) ? 1 : 0;
		}
		words_[word_count-1] &= top_mask_;
		return*this;
	}

	constexpr BigInt& operator*=(uint64_t val) noexcept{
		if(val==0){
			words_.fill(0);
			return*this;
		}
		if(val==1) return*this;

		uint64_t carry = 0;
		for(std::size_t i = 0; i < word_count; ++i){
			uint64_t a = words_[i];
			uint64_t a_lo = a & 0xFFFFFFFFULL, a_hi = a >> 32;
			uint64_t b_lo = val & 0xFFFFFFFFULL, b_hi = val >> 32;

			uint64_t p0 = a_lo * b_lo, p1 = a_lo * b_hi;
			uint64_t p2 = a_hi * b_lo, p3 = a_hi * b_hi;

			uint64_t low_mid = (p0 >> 32) + 
				(p1 & 0xFFFFFFFFULL) + (p2 & 0xFFFFFFFFULL);
			uint64_t prod_lo = (p0 & 0xFFFFFFFFULL) | (low_mid << 32);
			uint64_t prod_hi = p3 + (p1 >> 32) + (p2 >> 32) + (low_mid >> 32);

			uint64_t sum1 = prod_lo + carry;
			uint64_t c1 = (sum1 < prod_lo) ? 1 : 0;

			words_[i] = sum1;
			carry = prod_hi + c1;
		}
		words_[word_count-1] &= top_mask_;
		return*this;
	}

	static constexpr void div_rem_scalar(
			const BigInt& num, uint64_t den, BigInt& quo, uint64_t& rem) noexcept{
		quo = BigInt(0);
		rem = 0;
		if(den == 0) return;

		for(std::size_t i = word_count; i-- > 0;){
			uint64_t word = num.words_[i];
			for(int b = 63; b >= 0; --b){
				rem = (rem << 1) | ((word >> b) & 1ULL);
				if(rem >= den){
					rem -= den;
					quo.words_[i] |= (1ULL << b);
				}
			}
		}
		quo.words_[word_count-1] &= top_mask_;
	}

	constexpr BigInt& operator/=(uint64_t rhs) noexcept{
		BigInt quo;
		uint64_t rem;
		div_rem_scalar(*this, rhs, quo, rem);
		*this = quo;
		return*this;
	}

	constexpr uint64_t operator%(uint64_t rhs) const noexcept{
		BigInt quo;
		uint64_t rem;
		div_rem_scalar(*this, rhs, quo, rem);
		return rem;
	}

	friend constexpr BigInt operator+(BigInt a, uint64_t b) noexcept {return a += b;}
	friend constexpr BigInt operator+(uint64_t a, BigInt b) noexcept {return b += a;}
	friend constexpr BigInt operator-(BigInt a, uint64_t b) noexcept {return a -= b;}
	friend constexpr BigInt operator*(BigInt a, uint64_t b) noexcept {return a *= b;}
	friend constexpr BigInt operator*(uint64_t a, BigInt b) noexcept {return b *= a;}
	friend constexpr BigInt operator/(BigInt a, uint64_t b) noexcept {return a /= b;}

	template<uint64_t TargetBits, uint64_t SourceBits>
	static constexpr BigInt<TargetBits> cast(const BigInt<SourceBits>& src) noexcept{
		BigInt<TargetBits> dst;
		constexpr std::size_t words_to_copy = std::min((SourceBits+63)/64, (TargetBits+63)/64);
		for(std::size_t i = 0; i < words_to_copy; ++i)
			dst.set_word(i,src.get_word(i));
		return dst;
	}

private:
	static constexpr std::size_t word_count = static_cast<std::size_t>((N+63)/64);
	std::array<std::uint64_t, word_count> words_{};

	static constexpr std::uint64_t top_word_mask() noexcept{
		constexpr std::size_t rem = N % 64;
		if constexpr(rem == 0) return ~0ULL;
		else return (1ULL << rem) - 1ULL;
	}

	static constexpr std::uint64_t top_mask_ = top_word_mask();
};

template<int N>
inline std::ostream& operator<<(std::ostream& os, BigInt<N>& x){
	std::size_t word_count = static_cast<std::size_t>((N+63)/64);
	for(std::size_t i = word_count; i-- > 0;){
		uint64_t word = x.get_word(i);
		os << word;
	}
	return os;
}

