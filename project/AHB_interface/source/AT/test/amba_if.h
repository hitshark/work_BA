#ifndef  _AMBA_IF_H
#define  _AMBA_IF_H

#include "systemc.h"

class amba_if : virtual public sc_interface
{
public:
	virtual unsigned int read(unsigned int address)=0;
	virtual unsigned short int read_half(unsigned int address)=0;
	virtual unsigned char read_byte(unsigned int address)=0;
	virtual void write(unsigned int address, unsigned int  data)=0;
	virtual void write_half (unsigned int address, unsigned short int  data)=0;
	virtual void write_byte (unsigned int address, unsigned char  data)=0;

	virtual bool incr_burst_read(unsigned int address,unsigned char *data,unsigned int length,unsigned int m_size)=0;
	virtual bool incr_burst_write(unsigned int address,unsigned char *data,unsigned int length,unsigned int m_size)=0;
	virtual bool wrap_burst_read(unsigned int address,unsigned char *data,unsigned int length,unsigned int m_size)=0;
	virtual bool wrap_burst_write(unsigned int address,unsigned char *data,unsigned int length,unsigned int m_size)=0;
};
#endif  //_AMBA_IF_H
