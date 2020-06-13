#ifndef TOP_H_INCLUDED
#define TOP_H_INCLUDED
#include "ahb_at.h"
#include "my_target.h"
#include "amba_if.h"
#include "generator.h"

class top:public sc_module
{
public:
    top(sc_core::sc_module_name nm);

private:
    generator gen;
    ahb_at prom_trans_at;
    my_target    prom_at;
};

#endif // TOP_H_INCLUDED
