#include "islandTest.h"
int main(int argc, char* argv[])
{
	// ZDT test suite is used in the NSGA II paper as well
	islandTest::run_nsga2(islandTest::run_zdt);


    return 0;
}