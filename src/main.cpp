#include "islandTest.h"
int main(int argc, char* argv[])
{
	// ZDT test suite is used in the NSGA II paper as well
	islandTest::run_nsga2(islandTest::run_zdt);

	// "Deb, Thiele, Laumanns and Zitzler" - "box-constrained continuous n-dimensional multi-objective problems"
	//nsga2Test::run_nsga2(nsga2Test::run_dtlz);

    return 0;
}