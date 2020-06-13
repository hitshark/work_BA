#ifndef MY_AMBA_ARB_H_INCLUDED
#define MY_AMBA_ARB_H_INCLUDED
#include "amba_if.h"
#include "systemc.h"

class my_amba_arb:public amba_if,public sc_module
{
public:
    my_amba_arb(sc_core::sc_module_name nm);

    virtual unsigned int read(unsigned int address);
	virtual unsigned short int read_half(unsigned int address);
	virtual unsigned char read_byte(unsigned int address);
	virtual void write(unsigned int address, unsigned int  data);
	virtual void write_half (unsigned int address, unsigned short int  data);
	virtual void write_byte (unsigned int address, unsigned char  data);
	virtual bool incr_burst_read(unsigned int address,unsigned char *data,unsigned int length,unsigned int m_size);
	virtual void incr_burst_write(unsigned int address,unsigned char *data,unsigned int length,unsigned int m_size);
	virtual bool wrap_burst_read(unsigned int address,unsigned char *data,unsigned int length,unsigned int m_size);
	virtual void wrap_burst_write(unsigned int address,unsigned char *data,unsigned int length,unsigned int m_size);

	sc_core::sc_port<amba_if>prom_port;
	sc_core::sc_port<amba_if>sram_port;
};


#endif // MY_AMBA_ARB_H_INCLUDED
