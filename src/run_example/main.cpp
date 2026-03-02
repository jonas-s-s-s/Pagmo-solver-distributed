#include "distributed_controller.h"
#include "distributed_worker.h"
#include "islandTest.h"
int main(int argc, char* argv[])
{
	std::string address = "tcp://localhost:5000";

	if (argc >= 2 && argv[1] == std::string("-run-controller"))
	{
		distributed_controller controller{address};
		for (;;)
		{
			controller.server_loop();
		}
	}
	else
	{
		distributed_worker worker{address};
		for (;;)
		{
			worker.client_loop();
		}
	}

	// ZDT test suite is used in the NSGA II paper as well
	//islandTest::run_nsga2(islandTest::run_zdt);

    return 0;
}