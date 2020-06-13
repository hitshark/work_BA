#ifndef AHB_SLAVE_IF_H_INCLUDED
#define AHB_SLAVE_IF_H_INCLUDED
#include "systemc.h"
#include "amba_constants.h"

class ahb_slave_if : virtual public sc_interface
{
public:
//*************************************************************
//  bus request interface
//************************************************************/

    virtual bool bus_req() = 0;

//*************************************************************
// read interfaces
//************************************************************/

    virtual int nonseq_read(unsigned int haddr) = 0;
    virtual int seq_read(unsigned int haddr,int hsize,unsigned int &hrdata) = 0;
    virtual int last_read(int hsize,unsigned int &hrdata) = 0;

//*************************************************************
//  write interface
// ************************************************************/

    virtual int nonseq_write(unsigned int haddr) = 0;
    virtual int seq_write(unsigned int haddr,int hsize,unsigned int hwdata) = 0;
    virtual int last_write(int hsize,unsigned int hwdata) = 0;
};

#endif // AHB_SLAVE_IF_H_INCLUDED
