#include "my_memory.h"                         // Header for this class
#include "systemc.h"
#include <iostream>

using namespace sc_core;
using std::cout;
using std::endl;

/// Constructor
my_memory::my_memory
(
  const unsigned int ID                  // Target ID
, sc_core::sc_time   read_delay          // read delay
, sc_core::sc_time   write_delay         // write delay
, sc_dt::uint64      memory_size         // memory size (bytes)
, unsigned int       memory_width        // memory width (bytes)
)
: m_ID              (ID)
, m_read_delay      (read_delay)
, m_write_delay     (write_delay)
, m_memory_size     (memory_size)
, m_memory_width    (memory_width)
{
    /// Allocate and initalize an array for the target's memory
    m_memory = new unsigned char[size_t(m_memory_size)];

    /// clear memory
    memset(m_memory, 0, size_t(m_memory_size));
} // end Constructor

//==============================================================================
///  @fn my_memory::operation
//
///  @brief performs read and write
//
///  @details
///    This routine implements the read and  write operations. including
///    checking for byte_enable and streaming that are not implemented
//
//==============================================================================
void
my_memory::operation
( tlm::tlm_generic_payload  &gp
, sc_core::sc_time          &delay_time   ///< transaction delay
)
{
    /// Access the required attributes from the payload
    sc_dt::uint64    address   = gp.get_address();     // memory address
    tlm::tlm_command command   = gp.get_command();     // memory command
    unsigned char    *data     = gp.get_data_ptr();    // data pointer
    unsigned  int     length   = gp.get_data_length(); // data length

    tlm::tlm_response_status response_status = check_address(gp);
    if (gp.get_byte_enable_ptr())
    {
        gp.set_response_status(tlm::TLM_BYTE_ENABLE_ERROR_RESPONSE);
    }
    else if (gp.get_streaming_width() != gp.get_data_length())
    {
        gp.set_response_status(tlm::TLM_BURST_ERROR_RESPONSE);
    }

    switch (command)
    {
        default:
        {
            gp.set_response_status(tlm::TLM_COMMAND_ERROR_RESPONSE);
            delay_time = sc_core::SC_ZERO_TIME;
            break;
        }

    /// Setup a TLM_WRITE_COMMAND Informational Message and Write the Data from
    /// the Generic Payload Data pointer to Memory
    ///
        case tlm::TLM_WRITE_COMMAND:
        {
            if (response_status == tlm::TLM_OK_RESPONSE)
            {
                for (unsigned int i = 0; i < length; i++)
                {
                    m_memory[address++] = data[i];     // move the data to memory
                }
				/*
				if(length==1)
					cout<<"write byte:	"<<*data<<endl;
				else if(length==2)
					cout<<"write half word:	"<<*reinterpret_cast<unsigned short *>(data)<<endl;
				else
					cout<<"write word:	"<<*reinterpret_cast<unsigned int *>(data)<<endl;
				*/
                delay_time = delay_time + m_write_delay;
            }
            break;
        }

        case tlm::TLM_READ_COMMAND:
        {
            if (response_status == tlm::TLM_OK_RESPONSE)
            {
                for (unsigned int i = 0; i < length; i++)
                {
                    data[i] = m_memory[address++];         // move the data to memory
                }
				/*
				if(length==1)
					cout<<"read byte:	"<<*data<<endl;
				else if(length==2)
					cout<<"read  half word:	"<<*reinterpret_cast<unsigned short *>(data)<<endl;
				else
					cout<<"read word:	"<<*reinterpret_cast<unsigned int *>(data)<<endl;
				*/
                delay_time = delay_time + m_read_delay;
            }
            break;
        }
    } // end switch

    gp.set_response_status(response_status);
    return;
} // end memory_operation

//==============================================================================
///  @fn my_memory::get_mem_ptr
//
///  @brief Method to return pointer to memory in this object
//
///  @details
///    This routine used during dmi example
//
//==============================================================================
unsigned char*
my_memory::get_mem_ptr(void)
{
    return m_memory;
}

//==============================================================================
///  @fn my_memory::get_delay
//
///  @brief Method to "pull" appropriate delay from the gp
//
///  @details
///    This routine used during several at examples
//
//==============================================================================
void
my_memory::get_delay
( tlm::tlm_generic_payload  &gp           ///< TLM2 GP reference
, sc_core::sc_time          &delay_time   ///< time to be updated
)
{
    /// Access the required attributes from the payload
    tlm::tlm_command command = gp.get_command();     // memory command
    switch (command)
    {
        default:
        {
            cout<<"unsupport command!"<<endl;
            break;
        }

        /// Setup a TLM_WRITE_COMMAND Informational Message and Write the Data from
        /// the Generic Payload Data pointer to Memory
        case tlm::TLM_WRITE_COMMAND:
        {
            delay_time = delay_time + m_write_delay;
            break;
        }
        case tlm::TLM_READ_COMMAND:
        {
            delay_time = delay_time + m_read_delay;
            break;
        }
    } // end switch
    return;
}

//==============================================================================
///  @fn my_memory::check_address
//
///  @brief Method to check if the gp is in the address range of this memory
//
///  @details
///    This routine used to check for errors in address space
//
//==============================================================================
tlm::tlm_response_status
my_memory::check_address
( tlm::tlm_generic_payload  &gp
)
{
    sc_dt::uint64    address   = gp.get_address();     // memory address
    unsigned  int     length   = gp.get_data_length(); // data length
    if ( address >= m_memory_size )
    {
        cout<<"address goes out of bounds!"<<endl;
        return tlm::TLM_ADDRESS_ERROR_RESPONSE; // operation response
    }
    else
    {
        if ( (address + length) >= m_memory_size )
        {
            cout<<"address will go out of bounds!"<<endl;
            return tlm::TLM_ADDRESS_ERROR_RESPONSE; // operation response
        }
        return tlm::TLM_OK_RESPONSE;
    }
} // end check address
