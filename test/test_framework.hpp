#pragma once

#include<iostream>
#include<source_location>
#include<string_view>

namespace test{

inline int total_tests = 0;
inline int failed_tests = 0;

inline void check(bool condition, std::string_view expr,
		std::source_location loc = std::source_location::current()){
	++total_tests;
	if(!condition){
		++failed_tests;
		std::cerr << "test failed " << loc.file_name() << ":" << loc.line()
				<< " -> condition: (" << expr << ")\n";
	}
}

#define ASSERT_TRUE(expr)	::test::check((expr),#expr)
#define ASSERT_EQ(a,b)		::test::check((a) == (b), #a " == " #b)

inline int print_summary(){
	std::cout << "test summary: " << (total_tests - failed_tests)
		<< "/" << total_tests << " assertions passed\n";

	if(failed_tests > 0){
		std::cout << total_tests << "failed tests\n";
	}
	else{
		std::cout << "ok\n";
	}

	return failed_tests == 0 ? 0 : 1;
}

} // namespace test
