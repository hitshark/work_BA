#define SC_INCLUDE_DYNAMIC_PROCESSES
#include "ahb_ca.h"
#include "ahb_slave_ca.h"
#include "amba_if.h"
#include "generator.h"
//#include <fstream>
//#include "driver.h"

int sc_main(int argc,char *argv[])
{
    generator gen("gen",0,2096);
    ahb_ca prom_trans_ca( "prom_trans_ca",1,16);
    ahb_slave_ca    prom_ca( "prom_ca",0,2096);

    // pin connection
    gen.wr_port(prom_trans_ca.ahb_export);
	prom_trans_ca.ca_initiator.trans_port(prom_ca);

    sc_start(3000,SC_NS);
    return 0;
}
