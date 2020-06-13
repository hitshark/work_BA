/*
------------------------------------------------------------------------
--
-- File :                       traffic_package.h
-- Author:                      Tao Li
-- Date :                       $Date: 2014/2/15 13:03:45 $
-- Version      :               $Revision: 1.0 $
-- Abstract     :               traffic package model
--                              This file describes an adaptor which put the
--                              data passed from interface method call to
--                              generic payload and monitor the return status
--
-- Modification History:
-- Date                 By      Version     Change Description
------------------------------------------------------------------------
*/

#ifndef TRAFFIC_PACKAGE_H_INCLUDED
#define TRAFFIC_PACKAGE_H_INCLUDED
#include "systemc.h"
#include "tlm.h"
#include "amba_if.h"
#include <queue>

//#define MAX_TRANS  16
//#define DATA_LENGTH_W  4
//#define DATA_LENGTH_HW 2
//#define DATA_LENGTH_B  1

class traffic_package
:    public amba_if
,    public sc_core::sc_module
{
private:
    enum {DATA_LENGTH_B=1,DATA_LENGTH_HW=2,DATA_LENGTH_W=4,MAX_TRANS=16};
public:
    traffic_package(sc_core::sc_module_name name
                    , const unsigned int ID
                    , unsigned int  active_txn_count        //< Max number of active transactions
                    );
    void traffic_status_thread();

    virtual unsigned int read(unsigned int address);
    virtual unsigned short int read_half(unsigned int address);
    virtual unsigned char read_byte(unsigned int address);

    virtual void write(unsigned int address, unsigned int  data);
    virtual void write_half (unsigned int address, unsigned short int  data);
    virtual void write_byte (unsigned int address, unsigned char  data);

    virtual bool incr_burst_read(unsigned int address,unsigned char *data,unsigned int length,unsigned int m_size);
	virtual bool incr_burst_write(unsigned int address,unsigned char *data,unsigned int length,unsigned int m_size);
	virtual bool wrap_burst_read(unsigned int address,unsigned char *data,unsigned int length,unsigned int m_size);
	virtual bool wrap_burst_write(unsigned int address,unsigned char *data,unsigned int length,unsigned int m_size);

    class tg_queue_c                                            // memory managed queue class
    : public tlm::tlm_mm_interface                              // implements memory management IF
    {
    public:
        tg_queue_c(){}                                          // tg_queue_c constructor

        void enqueue()                                          // enqueue entry (create)
        {
            tlm::tlm_generic_payload  *transaction_ptr  = new tlm::tlm_generic_payload ( this );    // transaction pointer
            unsigned char             *data_buffer_ptr  = new unsigned char [ DATA_LENGTH_W ];      // data buffer pointer

            transaction_ptr->set_data_ptr ( data_buffer_ptr );
            m_queue.push ( transaction_ptr );
        }

        tlm::tlm_generic_payload *dequeue()                             // transaction pointer,dequeue entry
        {
            tlm::tlm_generic_payload *transaction_ptr = m_queue.front ();
            m_queue.pop();
            return transaction_ptr;
        }

        void release(tlm::tlm_generic_payload *transaction_ptr)         //release entry
        {
            transaction_ptr->release ();
        }

        bool is_empty()                                                  // queue empty
        {
            return m_queue.empty ();
        }

        size_t size()                                                   // queue size
        {
            return m_queue.size ();
        }

        void free(tlm::tlm_generic_payload *transaction_ptr)            // return transaction to the pool
        {
            transaction_ptr->reset();
            m_queue.push ( transaction_ptr );
        }

    private:
        std::queue<tlm::tlm_generic_payload*> m_queue;          // queue
    };

private:

    typedef tlm::tlm_generic_payload  *gp_ptr;
    tg_queue_c          m_transaction_queue;
    unsigned int   m_ID;
    unsigned int        m_active_txn_count;                     // active transaction count
    sc_core::sc_event read_ok;
	sc_core::sc_event write_ok;

public:
    // Port for requests to the initiator
    sc_core::sc_port<sc_core::sc_fifo_out_if <gp_ptr> > request_out_port;

    // Port for responses from the initiator
    sc_core::sc_port<sc_core::sc_fifo_in_if <gp_ptr> > response_in_port;
};


#endif // TRAFFIC_PACKAGE_H_INCLUDED
