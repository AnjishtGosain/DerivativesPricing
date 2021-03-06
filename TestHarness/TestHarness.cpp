// TestHarness.cpp : Defines the entry point for the console application.
//

#include <iostream>
#include <vector>
#include <memory>
#include <random>
#include <stdio.h>
#include "BlackScholesModelTests.h"
#include "BlackScholesWithDividendModelTests.h"
#include "BlackScholesSingleNormalJumpTests.h"
#include "BlackScholesDoubleNormalJumpTests.h"
#include "OptimisersTests.h"

using namespace std;
using namespace tests;

#define TEST(test) test == false ? std::cout << "Failed: " #test "\n" : std::cout << "Passed: " #test "\n"  

void PressEnterToContinue()
{
	int c;
	printf("Press ENTER to continue... ");
	fflush(stdout);
	do c = getchar(); while ((c != '\n') && (c != EOF));
}

int main()
{
	TEST(BlackScholesModelTest1());
	TEST(BlackScholesModelTest2());
	TEST(BlackScholesModelTest3());
	TEST(BlackScholesModelTest4());
	//TEST(BlackScholesModelPerformanceTest()); // this test does not perform any result validations, but compares performance of the pricing methods.

	TEST(BlackScholesWithDividendModelTest1());

	TEST(BlackScholesSingleNormalJumpTest1());

 	TEST(BlackScholesDoubleNormalJumpTest1());

	TEST(OptimisersTest1());


	PressEnterToContinue();
	return 0;
}



