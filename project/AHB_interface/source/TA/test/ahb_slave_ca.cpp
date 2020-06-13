#include "ahb_slave_ca.h"

ahb_slave_ca::ahb_slave_ca(sc_core::sc_module_name name,unsigned int st_addr,unsigned int ed_addr)
: sc_module (name)
, start_addr(st_addr)
, end_addr(ed_addr)
{
    mem=new unsigned char[end_addr-start_addr];
    for(unsigned int i=0;i<(end_addr-start_addr);i++)
    {
        *(mem+i)=0;
    }
}

bool ahb_slave_ca::bus_req()
{
    sc_core::wait(10,SC_NS);
    return true;
}

int ahb_slave_ca::nonseq_write(unsigned int haddr)
{
    previous_addr=haddr;
    return OKAY;
}

int ahb_slave_ca::seq_write(unsigned int haddr,int hsize,unsigned int hwdata)
{
    current_addr=haddr;
    if(hsize==WORD)
        *reinterpret_cast<unsigned int *>(mem+previous_addr)=hwdata;
    else if(hsize==HWORD)
        *reinterpret_cast<unsigned short *>(mem+previous_addr)=unsigned short(hwdata & 0x0000ffff);
    else if(hsize==BYTE)
        *(mem+previous_addr)=unsigned char(hwdata & 0x000000ff);
    previous_addr=current_addr;
    return OKAY;
}

int ahb_slave_ca::last_write(int hsize,unsigned int hwdata)
{
    if(hsize==WORD)
        *reinterpret_cast<unsigned int *>(mem+previous_addr)=hwdata;
    else if(hsize==HWORD)
        *reinterpret_cast<unsigned short *>(mem+previous_addr)=unsigned short(hwdata & 0x0000ffff);
    else if(hsize==BYTE)
        *(mem+previous_addr)=unsigned char(hwdata & 0x000000ff);
    return OKAY;
}


int ahb_slave_ca::nonseq_read(unsigned int haddr)
{
    previous_addr=haddr;
    return OKAY;
}

int ahb_slave_ca::seq_read(unsigned int haddr,int hsize,unsigned int& hrdata)
{
    current_addr=haddr;
    if(hsize==WORD)
        hrdata=*reinterpret_cast<unsigned int *>(mem+previous_addr);
    else if(hsize==HWORD)
        hrdata=*reinterpret_cast<unsigned short *>(mem+previous_addr);
    else if(hsize==BYTE)
        hrdata=*(mem+previous_addr);
    previous_addr=current_addr;
    return OKAY;
}

int ahb_slave_ca::last_read(int hsize,unsigned int& hrdata)
{
    if(hsize==WORD)
        hrdata=*reinterpret_cast<unsigned int *>(mem+previous_addr);
    else if(hsize==HWORD)
        hrdata=*reinterpret_cast<unsigned short *>(mem+previous_addr);
    else if(hsize==BYTE)
        hrdata=*(mem+previous_addr);
    return OKAY;
}