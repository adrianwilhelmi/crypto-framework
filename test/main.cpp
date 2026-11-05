#include"test_framework.hpp"

void tests_fp();
void tests_fpk();
void tests_elliptic();
void tests_serial();
void tests_bigint();
void test_e2e();

int main(){
	tests_fp();
	tests_fpk();
	tests_elliptic();
	tests_serial();
	tests_bigint();
	test_e2e();

	return test::print_summary();
}
