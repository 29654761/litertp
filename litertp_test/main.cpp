#include <stdio.h>
#include <fstream>
#include <sstream>
#include "rtp_test.h"

#include <shared_mutex>
std::shared_mutex mutex;

int main(char** args, int argc)
{

	litertp_global_init();

	rtp_test test;
	test.init();
	getchar();

	test.cleanup();

	litertp_global_cleanup();
}

