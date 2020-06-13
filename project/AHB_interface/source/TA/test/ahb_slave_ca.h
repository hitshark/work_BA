#ifndef AHB_SLAVE_CA_H_INCLUDED
#define AHB_SLAVE_CA_H_INCLUDED
#include "systemc.h"
#include "ahb_slave_if.h"
#include "amba_constants.h"

class ahb_slave_ca : public sc_module,public ahb_slave_if
{
public:
    ahb_slave_ca(sc_core::sc_module_name name,unsigned int st_addr,unsigned int ed_addr);
//*************************************************************
//  bus request interface
//************************************************************/

    bool bus_req();

//*************************************************************
// read interfaces
//************************************************************/

    int nonseq_read(unsigned int haddr);
    int seq_read(unsigned int haddr,int hsize,unsigned int &hrdata);
    int last_read(int hsize,unsigned int &hrdata);

//*************************************************************
//  write interface
// ************************************************************/

    int nonseq_write(unsigned int haddr);
    int seq_write(unsigned int haddr,int hsize,unsigned int hwdata);
    int last_write(int hsize,unsigned int hwdata);

    ~ahb_slave_ca(){delete [] mem;}

private:
    unsigned int start_addr;
    unsigned int end_addr;
    unsigned char *mem;
	unsigned int previous_addr;
	unsigned int current_addr;
};


#endif // AHB_SLAVE_CA_H_INCLUDED
