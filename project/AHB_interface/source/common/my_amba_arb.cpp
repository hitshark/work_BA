#include "my_amba_arb.h"

SC_HAS_PROCESS(my_amba_arb);
my_amba_arb::my_amba_arb(sc_core::sc_module_name nm)
:sc_module(nm)
{}

unsigned int my_amba_arb::read(unsigned int address)
{
    unsigned int data;
    if(address<1024)
    {
        data=prom_port->read(address);
    }
    else if(address>=1024)
    {
        data=sram_port->read(address);
    }
    else
    {
        cout<<"wrong address!"<<endl;
    }
    return data;
}

unsigned short int my_amba_arb::read_half(unsigned int address)
{
    unsigned short int data;
    if(address<1024)
    {
        data=prom_port->read_half(address);
    }
    else if(address>=1024)
    {
        data=sram_port->read_half(address);
    }
    else
    {
        cout<<"wrong address!"<<endl;
    }
    return data;
}

unsigned char my_amba_arb::read_byte(unsigned int address)
{
    unsigned char data;
    if(address<1024)
    {
        data=prom_port->read_byte(address);
    }
    else if(address>=1024)
    {
        data=sram_port->read_byte(address);
    }
    else
    {
        cout<<"wrong address!"<<endl;
    }
    return data;
}

void my_amba_arb::write(unsigned int address,unsigned int data)
{
    if(address<1024)
    {
        prom_port->write(address,data);
    }
    else if(address>=1024)
    {
        sram_port->write(address,data);
    }
    else
    {
        cout<<"wrong address!"<<endl;
    }
    return;
}

void my_amba_arb::write_half(unsigned int address,unsigned short int data)
{
    if(address<1024)
    {
        prom_port->write_half(address,data);
    }
    else if(address>=1024)
    {
        sram_port->write_half(address,data);
    }
    else
    {
        cout<<"wrong address!"<<endl;
    }
    return;
}

void my_amba_arb::write_byte(unsigned int address,unsigned char data)
{
    if(address<1024)
    {
        prom_port->write_byte(address,data);
    }
    else if(address>=1024)
    {
        sram_port->write_byte(address,data);
    }
    else
    {
        cout<<"wrong address!"<<endl;
    }
    return;
}

bool my_amba_arb::incr_burst_read(unsigned int address,unsigned char *data,unsigned int length,unsigned int m_size)
{
	bool state;
    if(address<1024)
    {
        state=prom_port->incr_burst_read(address,data,length,m_size);
    }
    else if(address>=1024)
    {
        state=sram_port->incr_burst_read(address,data,length,m_size);
    }
    else
    {
        cout<<"wrong address!"<<endl;
    }
    return state;
}

bool my_amba_arb::wrap_burst_read(unsigned int address,unsigned char *data,unsigned int length,unsigned int m_size)
{
	bool state;
    if(address<1024)
    {
        state=prom_port->wrap_burst_read(address,data,length,m_size);
    }
    else if(address>=1024)
    {
        state=sram_port->wrap_burst_read(address,data,length,m_size);
    }
    else
    {
        cout<<"wrong address!"<<endl;
    }
    return state;
}

void my_amba_arb::incr_burst_write(unsigned int address,unsigned char *data,unsigned int length,unsigned int m_size)
{
	if(address<1024)
    {
        prom_port->incr_burst_write(address,data,length,m_size);
    }
    else if(address>=1024)
    {
        sram_port->incr_burst_write(address,data,length,m_size);
    }
    else
    {
        cout<<"wrong address!"<<endl;
    }
    return;
}

void my_amba_arb::wrap_burst_write(unsigned int address,unsigned char *data,unsigned int length,unsigned int m_size)
{
	if(address<1024)
    {
        prom_port->wrap_burst_write(address,data,length,m_size);
    }
    else if(address>=1024)
    {
        sram_port->wrap_burst_write(address,data,length,m_size);
    }
    else
    {
        cout<<"wrong address!"<<endl;
    }
    return;
}