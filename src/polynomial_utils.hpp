#pragma once

#include<vector>
#include<array>
#include<cstddef>
#include<utility>

#include"fp.hpp"

using namespace field;

namespace utils{

template<int p>
constexpr void trim(std::vector<Fp<p>>& poly) noexcept{
	while(!poly.empty() && poly.back().value() == 0){
		poly.pop_back();
	}
}

template<int p>
constexpr std::vector<Fp<p>> poly_sub(std::vector<Fp<p>> a, const std::vector<Fp<p>>& b){
	if(b.size() > a.size()) a.resize(b.size(), Fp<p>(0));
	for(std::size_t i = 0; i < b.size(); ++i){
		a[i] -= b[i];
	}
	trim<p>(a);
	return a;
}

template<int p>
constexpr std::vector<Fp<p>> poly_mul(
		const std::vector<Fp<p>>& a,
		const std::vector<Fp<p>>& b){
	if(a.empty() || b.empty()) return {};
	std::vector<Fp<p>> res(a.size() + b.size() - 1, Fp<p>(0));

	for(std::size_t i = 0; i < a.size(); ++i){
		for(std::size_t j = 0; j < b.size(); ++j){
			res[i+j] += a[i] * b[j];
		}
	}

	trim<p>(res);
	return res;
}

template<int p>
constexpr std::vector<Fp<p>> poly_mod(std::vector<Fp<p>> a, std::vector<Fp<p>> b){
	trim<p>(a);
	trim<p>(b);

	if(b.empty()) throw std::invalid_argument("div by 0");

	int deg_b = static_cast<int>(b.size()) - 1;
	Fp<p> b_lead_inv = b.back().inv();

	while(static_cast<int>(a.size()) - 1 >= deg_b){
		trim<p>(a);
		if(static_cast<int>(a.size()) - 1 < deg_b) break;

		Fp<p> factor = a.back() * b_lead_inv;
		std::size_t deg_diff = a.size() - b.size();

		for(std::size_t i = 0; i <= static_cast<std::size_t>(deg_b); ++i){
			a[i + deg_diff] -= factor * b[i];
		}

		trim<p>(a);
	}

	return a;
}

template<int p>
constexpr std::vector<Fp<p>> poly_pow_mod(
		std::vector<Fp<p>> base, long long exp, const std::vector<Fp<p>>& mod_poly){
	std::vector<Fp<p>> result = { Fp<p>(1) };
	base = poly_mod(base, mod_poly);

	while(exp > 0){
		if(exp & 1){
			result = poly_mod(poly_mul(result, base), mod_poly);
		}
		base = poly_mod(poly_mul(base, base), mod_poly);
		exp >>= 1;
	}
	return result;
}

template<int p>
constexpr std::vector<Fp<p>> poly_gcd(std::vector<Fp<p>> a, std::vector<Fp<p>> b){
	trim<p>(a);
	trim<p>(b);

	while(!b.empty()){
		a = poly_mod(a,b);
		std::swap(a,b);
	}

	if(!a.empty()){
		Fp<p> lead_inv = a.back().inv();
		for(auto& coeff : a) coeff *= lead_inv;
	}

	return a;
}

template<int p>
constexpr bool is_irreducible(std::vector<Fp<p>> f){
	trim<p>(f);

	if(f.size() < 2) return false;

	// test requires polynomial to be monic
	//if(f.back() != Fp<p>(1)) return false;
	if(f.back() != Fp<p>(1)){
		const Fp<p> inv_lead = f.back().inv();
		for(auto& coeff : f){
			coeff *= inv_lead;
		}
	}

	const int deg = static_cast<int>(f.size()) - 1;

	if(deg == 1) return true;

	const std::vector<Fp<p>> x_poly = {Fp<p>(0), Fp<p>(1)};

	// h = x mod f
	std::vector<Fp<p>> h = poly_mod(x_poly, f);

	for(int k = 1; k <= deg / 2; ++k){
		// h = h^p mod f
		h = poly_pow_mod(std::move(h),p,f);

		const std::vector<Fp<p>> diff = poly_sub(h, x_poly);
		const std::vector<Fp<p>> g = poly_gcd(diff, f);

		if(g.size() != 1) return false;
	}

	return true;
}

template<int p, int k>
constexpr std::array<Fp<p>,k+1> find_first_irreducible(){
	static_assert(k > 0, "degree must be positive");

	// x^k
	std::vector<Fp<p>> f(k+1, Fp<p>(0));
	f[k] = 1;

	while(true){
		if(k == 1 || f[0].value() != 0){
			Fp<p> sum_coeffs(0);
			for(const auto& coeff : f){
				sum_coeffs += coeff;
			}

			if(sum_coeffs.value() != 0){
				if(is_irreducible(f)){
					std::array<Fp<p>, k+1> result{};
					for(std::size_t i = 0; i <= k; ++i){
						result[i] = f[i];
					}
					return result;
				}
			}
		}

		// lex increment
		std::size_t pos = 0;
		while(pos < k){
			int next_val = f[pos].value() + 1;
			if(next_val < p){
				f[pos] = Fp<p>(next_val);
				break;
			}
			else{
				f[pos] = Fp<p>(0);
				++pos;
			}
		}
	}
}

} // namespace utils
