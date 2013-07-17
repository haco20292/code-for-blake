#include <sys/wait.h>
#include <stdio.h>
#include <errno.h>
#include <sstream>
#include <iostream>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <errno.h>
#include <sstream>
#include <iostream>
#include <string>
#include <set>
#include <map>
#include <vector>
#include <list>
using namespace std;
#define DIE_LOG(fmt, args...) do{fprintf(stderr,"[pid:%d]" fmt "\n", getpid(), ##args); exit(0);}while(0)
#define ERRNO_LOG(fmt, args...) fprintf(stderr,"[pid:%d]" fmt "errno:%m\n", getpid(), ##args)
#define ERR_LOG(fmt, args...) fprintf(stderr, "[pid:%d]" fmt "\n", getpid(), ##args)
#define DBG_LOG(fmt, args...) fprintf(stderr,"[pid:%d]" fmt "\n", getpid(), ##args)
#define INFO(fmt, args...)    printf(fmt "\n", ##args)
#define die(fmt, args...) DIE_LOG(fmt, ##args)
struct InputData
{
    char* program;
    InputData()
    {}
}g_userinput;
/*--------------*/
void DoJob()
{
}
//
void Usage()
{
	fprintf(stderr, "%s [-f fork num]", g_userinput.program);
	exit(1);
}
int main(int argc, char** argv)
{
	g_userinput.program = argv[0];
	int _result = 0;
	while((_result = getopt(argc, argv, "f:h")) != -1)
	{
		switch(_result)
		{
			case 'h':
				Usage();
				break;
			case '?':
				ERR_LOG("undefine paramers:%d", optopt);
				break;
			default:
				ERR_LOG("default switch reached, optopt:%d optarg:%s", optopt, optarg);
				break;
		}
	}
	DoJob();
	return 0;
}

