#ifndef GENERATOR_A_H_INCLUDED
#define GENERATOR_A_H_INCLUDED
#include "systemc.h"
#include "amba_if.h"

class generator_a:public sc_module
{
public:
    generator_a(sc_core::sc_module_name nm
              ,unsigned int start_address
              ,unsigned int end_address
              );
    void generate_thread();
	sc_core::sc_port<amba_if>wr_port;
private:
    unsigned int start_addr;
    unsigned int end_addr;
    unsigned char *data;
};


#endif // GENERATOR_A_H_INCLUDED
