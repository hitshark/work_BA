/*
------------------------------------------------------------------------
--
-- File :                       traffic_package.cpp
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

#include "traffic_package.h"

using namespace std;

// Constructor

SC_HAS_PROCESS(traffic_package);

//-----------------------------------------------------------------------------
//
traffic_package::traffic_package                // @todo keep me, lose other constructor
( sc_core::sc_module_name name                  // instance name
, const unsigned int    ID                      // initiator ID
, unsigned int        active_txn_count          //< Max number of active transactions
)

: sc_module           ( name              )     // instance name
, m_ID                ( ID                )     // initiator ID
, m_active_txn_count  ( active_txn_count  )     // number of active transactions

{
    SC_THREAD(traffic_status_thread);
    // build transaction pool
    for (unsigned int i = 0; i < MAX_TRANS; i++ )
    {
        m_transaction_queue.enqueue ();
    }
}

//package read and write transaction

unsigned int traffic_package::read(unsigned int address)
{
    tlm::tlm_generic_payload  *transaction_ptr;   //< transaction pointer
    unsigned char             *data_buffer_ptr;   //< data buffer pointer
    sc_dt::uint64 base_address;
    base_address=(sc_dt::uint64)address;

    if(!m_transaction_queue.is_empty())
    {
        transaction_ptr = m_transaction_queue.dequeue ();
        transaction_ptr->acquire();
	    ++m_active_txn_count;

        transaction_ptr->set_byte_enable_length(            4                );
        transaction_ptr->set_command          ( tlm::TLM_READ_COMMAND        );
        transaction_ptr->set_address          ( base_address                 );
        transaction_ptr->set_data_length      ( DATA_LENGTH_W                );
        transaction_ptr->set_streaming_width  ( DATA_LENGTH_W                );
        transaction_ptr->set_response_status  ( tlm::TLM_INCOMPLETE_RESPONSE );

        request_out_port->write (transaction_ptr);

        wait(read_ok);
        if(transaction_ptr->get_response_status() != tlm::TLM_OK_RESPONSE)
        {
            cout<<"read word failed!"<<endl;
            transaction_ptr->release();
            --m_active_txn_count;
            return 0;
        }
        else
        {
            data_buffer_ptr=transaction_ptr->get_data_ptr();
            unsigned int data_temp;
            // convert address of write data to an 32-bit value
            data_temp = *reinterpret_cast<unsigned int*>(data_buffer_ptr);

            transaction_ptr->release();
            --m_active_txn_count;
            return data_temp;
        }
    }
    else
    {
        cout<<"transaction queue is empty,transmit failed!"<<endl;
        return 0;
    }
}

unsigned short int traffic_package::read_half(unsigned int address)
{
    tlm::tlm_generic_payload  *transaction_ptr;   //< transaction pointer
    unsigned char             *data_buffer_ptr;   //< data buffer pointer
    sc_dt::uint64 base_address;
    base_address=(sc_dt::uint64)address;

    if(!m_transaction_queue.is_empty())
    {
        transaction_ptr = m_transaction_queue.dequeue ();
        transaction_ptr->acquire();
	    ++m_active_txn_count;

        transaction_ptr->set_byte_enable_length(            2                );
        transaction_ptr->set_command          ( tlm::TLM_READ_COMMAND        );
        transaction_ptr->set_address          ( base_address                 );
        transaction_ptr->set_data_length      ( DATA_LENGTH_HW               );
        transaction_ptr->set_streaming_width  ( DATA_LENGTH_HW               );
        transaction_ptr->set_response_status  ( tlm::TLM_INCOMPLETE_RESPONSE );

        request_out_port->write (transaction_ptr);

        wait(read_ok);
        if(transaction_ptr->get_response_status() != tlm::TLM_OK_RESPONSE)
        {
            cout<<"read half word failed!"<<endl;
            transaction_ptr->release();
            --m_active_txn_count;
            return 0;
        }
        else
        {
            data_buffer_ptr=transaction_ptr->get_data_ptr();
            unsigned short int data_temp;
            // convert address of write data to an 32-bit value
            data_temp = *reinterpret_cast<unsigned short int*>(data_buffer_ptr);

            transaction_ptr->release();
            --m_active_txn_count;
            return data_temp;
        }
    }
    else
    {
        cout<<"transaction queue is empty,transmit failed!"<<endl;
        return 0;
    }
}

unsigned char traffic_package::read_byte(unsigned int address)
{
    tlm::tlm_generic_payload  *transaction_ptr;   //< transaction pointer
    unsigned char             *data_buffer_ptr;   //< data buffer pointer
    sc_dt::uint64 base_address;
    base_address=(sc_dt::uint64)address;

    if(!m_transaction_queue.is_empty())
    {
        transaction_ptr = m_transaction_queue.dequeue ();
        transaction_ptr->acquire();
	    ++m_active_txn_count;

        transaction_ptr->set_byte_enable_length(            1                );
        transaction_ptr->set_command          ( tlm::TLM_READ_COMMAND        );
        transaction_ptr->set_address          ( base_address                 );
        transaction_ptr->set_data_length      ( DATA_LENGTH_B                );
        transaction_ptr->set_streaming_width  ( DATA_LENGTH_B                );
        transaction_ptr->set_response_status  ( tlm::TLM_INCOMPLETE_RESPONSE );

        request_out_port->write (transaction_ptr);

        wait(read_ok); //check the response status!
        if(transaction_ptr->get_response_status() != tlm::TLM_OK_RESPONSE)
        {
            cout<<"read byte failed!"<<endl;
            transaction_ptr->release();
            --m_active_txn_count;
            return 0;
        }
        else
        {
            data_buffer_ptr=transaction_ptr->get_data_ptr();
            unsigned char data_temp;
            // convert address of write data to an 32-bit value
            data_temp = *data_buffer_ptr;

            transaction_ptr->release();
            --m_active_txn_count;
            return data_temp;
        }
    }
    else
    {
        cout<<"transaction queue is empty,transmit failed!"<<endl;
        return 0;
    }
}

void traffic_package::write(unsigned int address, unsigned int  data)
{
    tlm::tlm_generic_payload  *transaction_ptr;   //< transaction pointer
    unsigned char             *data_buffer_ptr;   //< data buffer pointer
    sc_dt::uint64 base_address;
    base_address=(sc_dt::uint64)address;

    if(!m_transaction_queue.is_empty())
    {
        transaction_ptr = m_transaction_queue.dequeue ();
        transaction_ptr->acquire();
	    ++m_active_txn_count;

        data_buffer_ptr = transaction_ptr->get_data_ptr();

        // convert address of write data to an 32-bit value
        *reinterpret_cast<unsigned int*>(data_buffer_ptr) = data;

        transaction_ptr->set_byte_enable_length(            4                );
        transaction_ptr->set_command          ( tlm::TLM_WRITE_COMMAND       );
        transaction_ptr->set_address          ( base_address                 );
        transaction_ptr->set_data_length      ( DATA_LENGTH_W                );
        transaction_ptr->set_streaming_width  ( DATA_LENGTH_W                );
        transaction_ptr->set_response_status  ( tlm::TLM_INCOMPLETE_RESPONSE );

        request_out_port->write (transaction_ptr);
		sc_core::wait(write_ok);
		if(transaction_ptr->get_response_status() != tlm::TLM_OK_RESPONSE)
        {
            cout<<"write word failed!"<<endl;
        }
		transaction_ptr->release();
        --m_active_txn_count;
    }
    else
    {
        cout<<"transaction queue is empty,transmit failed!"<<endl;
    }
}

void traffic_package::write_half (unsigned int address, unsigned short int  data)
{
    tlm::tlm_generic_payload  *transaction_ptr;   //< transaction pointer
    unsigned char             *data_buffer_ptr;   //< data buffer pointer
    sc_dt::uint64 base_address;
    base_address=(sc_dt::uint64)address;

    if(!m_transaction_queue.is_empty())
    {
        transaction_ptr = m_transaction_queue.dequeue ();
        transaction_ptr->acquire();
	    ++m_active_txn_count;

        data_buffer_ptr = transaction_ptr->get_data_ptr();

        // convert address of write data to an 32-bit value
        *reinterpret_cast<unsigned short int*>(data_buffer_ptr) = data;

        transaction_ptr->set_byte_enable_length(            2                );
        transaction_ptr->set_command          ( tlm::TLM_WRITE_COMMAND       );
        transaction_ptr->set_address          ( base_address                 );
        transaction_ptr->set_data_length      ( DATA_LENGTH_HW               );
        transaction_ptr->set_streaming_width  ( DATA_LENGTH_HW               );
        transaction_ptr->set_response_status  ( tlm::TLM_INCOMPLETE_RESPONSE );

        request_out_port->write (transaction_ptr);
		sc_core::wait(write_ok);
		if(transaction_ptr->get_response_status() != tlm::TLM_OK_RESPONSE)
        {
            cout<<"write half word failed!"<<endl;
        }
		transaction_ptr->release();
        --m_active_txn_count;
    }
    else
    {
        cout<<"transaction queue is empty,transmit failed!"<<endl;
    }
}

void traffic_package::write_byte (unsigned int address, unsigned char  data)
{
    tlm::tlm_generic_payload  *transaction_ptr;   //< transaction pointer
    unsigned char             *data_buffer_ptr;   //< data buffer pointer
    sc_dt::uint64 base_address;
    base_address=(sc_dt::uint64)address;

    if(!m_transaction_queue.is_empty())
    {
        transaction_ptr = m_transaction_queue.dequeue ();
        transaction_ptr->acquire();
	    ++m_active_txn_count;

        data_buffer_ptr = transaction_ptr->get_data_ptr();

        // convert address of write data to an 32-bit value
        *data_buffer_ptr = data;

        transaction_ptr->set_byte_enable_length(            1                );
        transaction_ptr->set_command          ( tlm::TLM_WRITE_COMMAND       );
        transaction_ptr->set_address          ( base_address                 );
        transaction_ptr->set_data_length      ( DATA_LENGTH_B                );
        transaction_ptr->set_streaming_width  (				0                );
        transaction_ptr->set_response_status  ( tlm::TLM_INCOMPLETE_RESPONSE );

        request_out_port->write (transaction_ptr);
		sc_core::wait(write_ok);
		if(transaction_ptr->get_response_status() != tlm::TLM_OK_RESPONSE)
        {
            cout<<"write byte failed!"<<endl;
        }
		transaction_ptr->release();
        --m_active_txn_count;
    }
    else
    {
        cout<<"transaction queue is empty,transmit failed!"<<endl;
    }
}

bool traffic_package::incr_burst_read(unsigned int address,unsigned char *data,unsigned int length,unsigned int m_size)
{
    tlm::tlm_generic_payload  *transaction_ptr;   //< transaction pointer
    //unsigned char             *data_buffer_ptr;   ///< data buffer pointer
    sc_dt::uint64 base_address;
    base_address=(sc_dt::uint64)address;

    if(!m_transaction_queue.is_empty())
    {
        transaction_ptr = m_transaction_queue.dequeue ();
        transaction_ptr->acquire();
	    ++m_active_txn_count;

        transaction_ptr->set_data_ptr(data);
        transaction_ptr->set_byte_enable_length(            m_size           );
        transaction_ptr->set_command          ( tlm::TLM_READ_COMMAND        );
        transaction_ptr->set_address          ( base_address                 );
        transaction_ptr->set_data_length      ( length                       );
        transaction_ptr->set_streaming_width  (                 0            );
        transaction_ptr->set_response_status  ( tlm::TLM_INCOMPLETE_RESPONSE );

        request_out_port->write (transaction_ptr);
        wait(read_ok);
        if(transaction_ptr->get_response_status() != tlm::TLM_OK_RESPONSE)
        {
            cout<<"increase burst read failed!"<<endl;
            transaction_ptr->release();
            --m_active_txn_count;
            return false;
        }
        else
        {
            transaction_ptr->release();
            --m_active_txn_count;
            return true;
        }
    }
    else
    {
        cout<<"transaction queue is empty,transmit failed!"<<endl;
        return false;
    }
}

bool traffic_package::wrap_burst_read(unsigned int address,unsigned char *data,unsigned int length,unsigned int m_size)
{
    tlm::tlm_generic_payload  *transaction_ptr;   //< transaction pointer
    //unsigned char             *data_buffer_ptr;   ///< data buffer pointer
    sc_dt::uint64 base_address;
    base_address=(sc_dt::uint64)address;

    if(!m_transaction_queue.is_empty())
    {
        transaction_ptr = m_transaction_queue.dequeue ();
        transaction_ptr->acquire();
	    ++m_active_txn_count;

        transaction_ptr->set_data_ptr(data);
        transaction_ptr->set_byte_enable_length(            m_size           );
        transaction_ptr->set_command          ( tlm::TLM_READ_COMMAND        );
        transaction_ptr->set_address          ( base_address                 );
        transaction_ptr->set_data_length      ( length                       );
        transaction_ptr->set_streaming_width  (            length            );
        transaction_ptr->set_response_status  ( tlm::TLM_INCOMPLETE_RESPONSE );

        request_out_port->write (transaction_ptr);
        wait(read_ok);
        if(transaction_ptr->get_response_status() != tlm::TLM_OK_RESPONSE)
        {
            cout<<"wrap burst read failed!"<<endl;
            transaction_ptr->release();
            --m_active_txn_count;
            return false;
        }
        else
        {
            transaction_ptr->release();
            --m_active_txn_count;
            return true;
        }
    }
    else
    {
        cout<<"transaction queue is empty,transmit failed!"<<endl;
        return false;
    }
}

bool traffic_package::incr_burst_write(unsigned int address,unsigned char *data,unsigned int length,unsigned int m_size)
{
    tlm::tlm_generic_payload  *transaction_ptr;   //< transaction pointer
    sc_dt::uint64 base_address;
    base_address=(sc_dt::uint64)address;

    if(!m_transaction_queue.is_empty())
    {
        transaction_ptr = m_transaction_queue.dequeue ();
        transaction_ptr->acquire();
	    ++m_active_txn_count;

        transaction_ptr->set_data_ptr(data);

        transaction_ptr->set_byte_enable_length(            m_size           );
        transaction_ptr->set_command          ( tlm::TLM_WRITE_COMMAND       );
        transaction_ptr->set_address          ( base_address                 );
        transaction_ptr->set_data_length      ( length                       );
        transaction_ptr->set_streaming_width  ( 0                            );
        transaction_ptr->set_response_status  ( tlm::TLM_INCOMPLETE_RESPONSE );

        request_out_port->write (transaction_ptr);
		sc_core::wait(write_ok);
		if(transaction_ptr->get_response_status() != tlm::TLM_OK_RESPONSE)
        {
            cout<<"increase burst write failed!"<<endl;
            transaction_ptr->release();
            --m_active_txn_count;
            return false;
        }
        else
        {
            transaction_ptr->release();
            --m_active_txn_count;
            return true;
        }
    }
    else
    {
        cout<<"transaction queue is empty,transmit failed!"<<endl;
        return false;
    }
}

bool traffic_package::wrap_burst_write(unsigned int address,unsigned char *data,unsigned int length,unsigned int m_size)
{
    tlm::tlm_generic_payload  *transaction_ptr;   //< transaction pointer
    sc_dt::uint64 base_address;
    base_address=(sc_dt::uint64)address;

    if(!m_transaction_queue.is_empty())
    {
        transaction_ptr = m_transaction_queue.dequeue ();
        transaction_ptr->acquire();
	    ++m_active_txn_count;

        transaction_ptr->set_data_ptr(data);

        transaction_ptr->set_byte_enable_length(            m_size           );
        transaction_ptr->set_command          ( tlm::TLM_WRITE_COMMAND       );
        transaction_ptr->set_address          ( base_address                 );
        transaction_ptr->set_data_length      ( length                       );
        transaction_ptr->set_streaming_width  ( length                       );
        transaction_ptr->set_response_status  ( tlm::TLM_INCOMPLETE_RESPONSE );

        request_out_port->write (transaction_ptr);
		sc_core::wait(write_ok);
		if(transaction_ptr->get_response_status() != tlm::TLM_OK_RESPONSE)
        {
            cout<<"wrap burst write failed!"<<endl;
            transaction_ptr->release();
            --m_active_txn_count;
            return false;
        }
        else
        {
            transaction_ptr->release();
            --m_active_txn_count;
            return true;
        }
    }
    else
    {
        cout<<"transaction queue is empty,transmit failed!"<<endl;
        return false;
    }
}
// SystemC thread for generation of GP traffic

void traffic_package::traffic_status_thread()
{
	sc_core::wait(5,sc_core::SC_NS);
    tlm::tlm_generic_payload  *transaction_ptr;
    while(true)
    {
        while(response_in_port->num_available()>0)
        {
            response_in_port->read(transaction_ptr);
            if(transaction_ptr->get_command()==tlm::TLM_READ_COMMAND)
            {
                read_ok.notify(sc_core::SC_ZERO_TIME);
            }
            else if(transaction_ptr->get_command()==tlm::TLM_WRITE_COMMAND)
            {
				write_ok.notify(sc_core::SC_ZERO_TIME);
                //transaction_ptr->release();
                //--m_active_txn_count;
            }
        }
		sc_core::wait(5,sc_core::SC_NS);
    }
}
