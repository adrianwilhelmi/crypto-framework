#pragma once

#include<string>
#include<vector>
#include<sstream>
#include<iomanip>
#include<stdexcept>
#include<cstdint>
#include<array>
#include<algorithm>

#include"fp.hpp"
#include"fpk.hpp"
#include"elliptic.hpp"

namespace ser{

//detail

enum class Encoding{
	Base10,
	Base16,
	Base64
};

static constexpr char base64_chars[] = 
	"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

inline std::string bytes_to_base64(const std::vector<std::uint8_t>& data){
	std::string out;
	int val = 0, valb = -6;
	for(std::uint8_t c : data){
		val = (val << 8) + c;
		valb += 8;
		while(valb>=0){
			out.push_back(base64_chars[(val>>valb) & 0x3F]);
			valb -= 6;
		}
	}
	if(valb > -6) out.push_back(base64_chars[((val<<8) >> (valb+8)) & 0x3F]);
	while(out.size() % 4) out.push_back('=');
	return out;
}

inline std::vector<std::uint8_t> base64_to_bytes(const std::string& in){
	std::vector<std::uint8_t> out;
	std::vector<int> T(256,-1);
	for(int i = 0; i < 64; ++i)
		T[static_cast<unsigned char>(base64_chars[i])] = i;

	int val = 0; 
	int valb = -8;
	for(unsigned char c : in){
		if(c=='=') break;
		if(T[c] == -1) continue;
		val = (val << 6) + T[c];
		valb += 6;
		if(valb >= 0){
			out.push_back(static_cast<std::uint8_t>((val>>valb)&0xFF));
			valb -= 8;
		}
	}
	return out;
}

inline std::string bytes_to_hex(const std::vector<uint8_t>& bytes){
	std::ostringstream oss;
	for(auto b : bytes){
		oss << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(b);
	}
	return oss.str();
}

inline std::vector<std::uint8_t> hex_to_bytes(const std::string& hex){
	std::vector<std::uint8_t> bytes;
	std::string clean_hex = (hex.rfind("0x", 0) == 0) ? hex.substr(2) : hex;
	if(clean_hex.size() % 2 != 0) clean_hex = "0" + clean_hex;
	for(std::size_t i = 0; i < clean_hex.length(); i+=2){
		std::string byte_string = clean_hex.substr(i,2);
		std::uint8_t byte = static_cast<std::uint8_t>(
			std::strtoul(byte_string.c_str(), nullptr, 16)
		);
		bytes.push_back(byte);
	}
	return bytes;
}

// fields
template<int p>
std::string serialize(const field::Fp<p>& elem, Encoding enc){
	std::uint64_t val = elem.value();
	if(enc==Encoding::Base10) return std::to_string(val);
	else if(enc==Encoding::Base16){
		std::ostringstream oss;
		oss << "0x" << std::hex << val;
		return oss.str();
	}
	else if(enc==Encoding::Base64){
		std::vector<std::uint8_t> bytes;
		while(val>0){
			bytes.push_back(static_cast<std::uint8_t>(val&0xFF));
			val >>= 8;
		}
		if(bytes.empty()) bytes.push_back(0);
		std::reverse(bytes.begin(), bytes.end());
		return bytes_to_base64(bytes);
	}
	return "";
}

template<typename TargetType>
TargetType deserialize(const std::string& str, Encoding enc);

template<int p>
field::Fp<p> deserialize_fp(const std::string& str, Encoding enc){
	if(enc==Encoding::Base10){
		return field::Fp<p>(static_cast<int>(std::stoull(str,nullptr,10)));
	}
	else if(enc==Encoding::Base16){
		return field::Fp<p>(static_cast<int>(std::stoull(str,nullptr,16)));
	}
	else if(enc==Encoding::Base64){
		auto bytes = base64_to_bytes(str);
		std::uint64_t val = 0;
		for(auto b : bytes) val = (val << 8) | b;
		return field::Fp<p>(static_cast<int>(val));
	}
	return 0;
}

template<int k, typename IP>
std::string serialize(const field::FpkBinaryImpl<k,IP>& elem, Encoding enc){
	// format:
	//	 <serialize(f[k-1..k-8])><serialize(f[k-9..k-16])>...<serialize(f[8...0])>
	constexpr std::size_t word_count = (k+63) / 64;
	std::vector<std::uint8_t> bytes(word_count*8,0);

	// uint64_t to big endian bytes
	for(std::size_t i = 0; i < static_cast<std::size_t>(k); ++i){
		if(elem[i]==1){
			std::size_t byte_idx = (k-1-i)/8;
			std::size_t bit_idx = i%8;
			bytes[byte_idx] |= (1<<bit_idx);
		}
	}

	if(enc == Encoding::Base16){
		return "0x" + bytes_to_hex(bytes);
	}
	else if(enc == Encoding::Base64){
		return bytes_to_base64(bytes);
	}
	else{
		std::ostringstream oss;
		for(std::size_t i = 0; i < static_cast<std::size_t>(k);++i)
			oss << (elem[k-1-i] == 1 ? '1' : '0');
		return oss.str();
	}
}

template<int k, typename IP>
field::FpkBinaryImpl<k,IP> deserialize_f2m(const std::string& str, Encoding enc){
	field::FpkBinaryImpl<k,IP> elem;
	if(enc == Encoding::Base10){
		for(std::size_t i = 0; i < str.length() && i < static_cast<std::size_t>(k);++i){
			if(str[str.length()-1-i] == '1'){
				elem[i] = 1;
			}
		}
	}
	else{
		std::vector<std::uint8_t> bytes = (enc == Encoding::Base16)
			? hex_to_bytes(str) : base64_to_bytes(str);

		for(std::size_t i = 0; i < static_cast<std::size_t>(k); ++i){
			std::size_t byte_idx = (k-1-i) / 8;
			std::size_t bit_idx = i%8;
			if(byte_idx < bytes.size() && ((bytes[byte_idx] >> bit_idx) & 1))
				elem[i]=1;
		}
	}
	return elem;
}

template<int p, int k, typename IP>
std::string serialize(const field::FpkImpl<p,k,IP>& elem, Encoding enc){
	// format: <serialize(f[0])>;<serialize(f[1])>;...;<serialize(f[k-1])>
	std::ostringstream oss;
	for(std::size_t i = 0; i < static_cast<std::size_t>(k); ++i){
		if(i>0) oss << ";";
		oss << serialize(elem[i],enc);
	}
	return oss.str();
}

template<int p, int k, typename IP>
field::FpkImpl<p,k,IP> deserialize_fpk(const std::string& str, Encoding enc){
	field::FpkImpl<p,k,IP> elem;
	std::stringstream ss(str);
	std::string token;
	std::size_t idx = 0;
	while(std::getline(ss,token,';') && idx<static_cast<std::size_t>(k)){
		elem[idx++] = deserialize_fp<p>(token,enc);
	}
	return elem;
}

template<typename Field>
struct FieldDeserializer;

template<int p>
struct FieldDeserializer<field::Fp<p>>{
	static field::Fp<p> parse(const std::string& str, Encoding enc){
		return deserialize_fp<p>(str,enc);
	}
};

template<int k, typename IP>
struct FieldDeserializer<field::FpkBinaryImpl<k,IP>>{
	static field::FpkBinaryImpl<k,IP> parse(const std::string& str, Encoding enc){
		return deserialize_f2m<k,IP>(str,enc);
	}
};

template<int p, int k, typename IP>
struct FieldDeserializer<field::FpkImpl<p,k,IP>>{
	static field::FpkImpl<p,k,IP> parse(const std::string& str, Encoding enc){
		return deserialize_fpk<p,k,IP>(str,enc);
	}
};

template<typename Field>
Field deserialize(const std::string& str, Encoding enc){
	return FieldDeserializer<Field>::parse(str,enc);
}


// elliptic
template<typename Field>
std::string serialize(const crypto::Point<Field>& pt, Encoding enc){
	// format: <pt.x>:<pt.y>
	if(pt.is_infinity()) return "inf";
	return serialize(pt.x(), enc) + ":" + serialize(pt.y(), enc);
}

template<typename Field>
crypto::Point<Field> deserialize_point(const crypto::EllipticCurve<Field>& curve,
		const std::string& str,
		Encoding enc){
	if(str == "inf"){
		return curve.infinity();
	}

	auto pos = str.find(':');
	if(pos == std::string::npos) throw std::invalid_argument("bad format");

	std::string x_str = str.substr(0,pos);
	std::string y_str = str.substr(pos+1);

	Field x = deserialize<Field>(x_str, enc);
	Field y = deserialize<Field>(y_str, enc);

	return curve.point(x,y);
}

template<typename Field>
std::string serialize(const crypto::EllipticCurve<Field>& curve, Encoding enc){
	// format: <a.x>:<a.b>|<b.x>:<b.y>
	return serialize(curve.a(), enc) + "|" + serialize(curve.b(), enc);
}

template<typename Field>
crypto::EllipticCurve<Field> deserialize_curve(const std::string& str, Encoding enc){
	auto pos = str.find('|');
	if(pos == std::string::npos) throw std::invalid_argument("bad curve format");

	std::string a_str = str.substr(0,pos);
	std::string b_str = str.substr(pos+1);

	Field a = deserialize<Field>(a_str, enc);
	Field b = deserialize<Field>(b_str, enc);

	return crypto::EllipticCurve<Field>(a,b);
}

} // namespace ser
