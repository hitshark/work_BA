#define SC_INCLUDE_DYNAMIC_PROCESSES
#include "top.h"

int sc_main(int argc,char *argv[])
{
    top top1("top1");
    sc_core::sc_start(2000,SC_NS);
    return 0;
}
