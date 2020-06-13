#ifndef MY_RAM_H_INCLUDED
#define MY_RAM_H_INCLUDED
#include "systemc.h"
#include "tlm.h"     // TLM headers
//#include "tlm_core\tlm_2\tlm_2_interfaces\tlm_fw_bw_ifs.h"

class my_ram:virtual public tlm::tlm_fw_transport_if<>,public sc_module
{
public:
    my_ram(sc_core::sc_module_name nm
           ,unsigned int start_addr
           ,unsigned int end_addr
           );

	~my_ram(){delete [] mem;}

/// b_transport() - Blocking Transport
  virtual void b_transport
  ( tlm::tlm_generic_payload  &gp                // ref to payload
  , sc_core::sc_time          &delay_time             // delay time
  );

/// nb_transport_fw() - Nonblockig Transport
  virtual tlm::tlm_sync_enum nb_transport_fw
  ( tlm::tlm_generic_payload &trans
    ,tlm::tlm_phase &phase
    ,sc_core::sc_time &t);

/// direct memory
  virtual bool get_direct_mem_ptr
  (tlm::tlm_generic_payload &trans
   ,tlm::tlm_dmi &dmi_data);

/// debug
  virtual unsigned int transport_dbg
  (tlm::tlm_generic_payload &trans);

// Member Variables ===================================================
  tlm::tlm_target_socket<>target_socket; ///<  target socket

private:
  unsigned char *mem;
  unsigned int start_address;
  unsigned int end_address;
};

#endif // MY_RAM_H_INCLUDED
