#include <stdio.h>
#include <fstream>
#include <sstream>
#include "rtp_test.h"

int main(char** args, int argc)
{
	litertp_global_init();

	rtp_test test;
	test.init();
	getchar();

	test.cleanup();

	litertp_global_cleanup();
}

