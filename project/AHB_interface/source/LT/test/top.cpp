#include "top.h"

SC_HAS_PROCESS(top);
top::top(sc_core::sc_module_name nm)
:sc_module(nm)
,gen("gen",0,2096)
,prom_trans("prom_trans",1,0)
,prom("prom",0,2096)
{
    gen.wr_port(prom_trans.ahb_export);
    prom_trans.initiator_socket(prom.target_socket);
}


