#include "top.h"

SC_HAS_PROCESS(top);
top::top(sc_core::sc_module_name nm)
:sc_module(nm)
,gen("gen",0,2096)
,prom_trans_at
( "prom_trans_at"
  ,1
  ,sc_core::sc_time(10,sc_core::SC_NS)
  ,0
)
,prom_at
( "prom_at"
  ,101
  ,2096
  ,4
  ,sc_core::sc_time(10,sc_core::SC_NS)
  ,sc_core::sc_time(10,sc_core::SC_NS)
  ,sc_core::sc_time(10,sc_core::SC_NS)
)
{
    gen.wr_port(prom_trans_at.ahb_export);
    prom_trans_at.initiator_socket(prom_at.target_socket);
}


