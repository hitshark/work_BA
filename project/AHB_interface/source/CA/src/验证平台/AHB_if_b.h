#ifndef AHB_IF_B_H_INCLUDED
#define AHB_IF_B_H_INCLUDED
#define SC_INCLUDE_DYNAMIC_PROCESSES
//#include "my_amba_arb.h"
#include "ahb_ca.h"
//#include "ahb_slave.h"
#include "amba_if.h"
#include "generator_b.h"
//#include <fstream>
//#include "driver.h"

SC_MODULE(AHB_if_b)
{
    sc_in<bool> clk;
    sc_in<bool> reset;

    sc_in<bool> hready;
    sc_in<sc_uint<HRESP_WIDTH> > hresp;
    sc_in<bool> hgrant;
    sc_in<sc_uint<BUSWIDTH> > hrdata;

    sc_out<bool> hbusreq;
    sc_out<bool> hwrite;
    sc_out<sc_uint<HTRANS_WIDTH> > htrans;
    sc_out<sc_uint<HSIZE_WIDTH> > hsize;
    sc_out<sc_uint<HBURST_WIDTH> > hburst;
    sc_out<sc_uint<BUSWIDTH> > haddr;
    sc_out<sc_uint<BUSWIDTH> > hwdata;

    generator_b gen;
    //my_amba_arb amba_arb;
    ahb_ca prom_trans_ca;



    SC_CTOR(AHB_if_b)
    ://amba_arb("amba_arb"),
    gen("gen",0xa000000,0xa00ffff),
    prom_trans_ca( "prom_trans_ca",1,16)
    {
        //gen.wr_port(amba_arb);
        //amba_arb.prom_port(prom_trans_ca.ahb_export);
        gen.wr_port(prom_trans_ca.ahb_export);

        prom_trans_ca.ca_initiator.clk(clk);
        prom_trans_ca.ca_initiator.hresetn(reset);
        prom_trans_ca.ca_initiator.hready_i(hready);
        prom_trans_ca.ca_initiator.hresp_i(hresp);
        prom_trans_ca.ca_initiator.hgrant_i(hgrant);
        prom_trans_ca.ca_initiator.hbusreq_o(hbusreq);
        prom_trans_ca.ca_initiator.htrans_o(htrans);
        prom_trans_ca.ca_initiator.hwrite_o(hwrite);
        prom_trans_ca.ca_initiator.hsize_o(hsize);
        prom_trans_ca.ca_initiator.hburst_o(hburst);
        prom_trans_ca.ca_initiator.haddr_o(haddr);
        prom_trans_ca.ca_initiator.hwdata_o(hwdata);
        prom_trans_ca.ca_initiator.hrdata_i(hrdata);
    }
};



#endif // AHB_IF_B_H_INCLUDED
