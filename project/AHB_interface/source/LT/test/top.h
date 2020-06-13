#ifndef TOP_H_INCLUDED
#define TOP_H_INCLUDED
#include "ahb_lt.h"
#include "my_ram.h"
#include "amba_if.h"
#include "generator.h"

class top:public sc_module
{
public:
    top(sc_core::sc_module_name nm);

private:
    generator gen;
	ahb_lt prom_trans;
    my_ram    prom;
};

#endif // TOP_H_INCLUDED
