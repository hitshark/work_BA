#include "my_ram.h"

using namespace sc_core;

SC_HAS_PROCESS(my_ram);
/// Constructor
my_ram::my_ram
(
 sc_core::sc_module_name nm
 ,unsigned int start_addr
 ,unsigned int end_addr
)
 :sc_module       (nm)
 ,target_socket   ("target_socket")
 ,start_address   (start_addr)
 ,end_address     (end_addr)
 {
     mem = new unsigned char[end_address - start_address+1];
	 for (unsigned int i=0; i<(end_address - start_address+1); i++)
	 {
	     mem[i] = 0;
	 }
     target_socket.bind(*this);
 }

tlm::tlm_sync_enum my_ram::nb_transport_fw
(tlm::tlm_generic_payload &trans
 ,tlm::tlm_phase &phase
 ,sc_core::sc_time &t)
{
    cout<<"not complement!"<<endl;
    return tlm::TLM_COMPLETED;
}

bool my_ram::get_direct_mem_ptr
(tlm::tlm_generic_payload &trans
 ,tlm::tlm_dmi &dmi_data)
{
    cout<<"not complement!"<<endl;
    return false;
}

unsigned int my_ram::transport_dbg
(tlm::tlm_generic_payload &trans)
{
    cout<<"not complement!"<<endl;
    return 0;
}

void my_ram::b_transport
(tlm::tlm_generic_payload   &gp                // ref to payload
 ,sc_core::sc_time          &delay_time             // delay time
)
{
  /// Access the required attributes from the payload
  sc_dt::uint64    address   = gp.get_address();     // memory address
  tlm::tlm_command command   = gp.get_command();     // memory command
  unsigned char    *data     = gp.get_data_ptr();    // data pointer
  unsigned  int     length   = gp.get_data_length(); // data length
  //unsigned int	trans_size	 = transaction_ptr->get_byte_enable_length();

  switch (command)
  {
    default:
    {
        cout<<"command is not exist!"<<endl;
        gp.set_response_status(tlm::TLM_COMMAND_ERROR_RESPONSE);
        delay_time = sc_core::SC_ZERO_TIME;
        break;
    }

    /// Setup a TLM_WRITE_COMMAND Informational Message and Write the Data from
    /// the Generic Payload Data pointer to Memory
    ///
    case tlm::TLM_WRITE_COMMAND:
    {
		for (unsigned int i = 0; i < length; i++)
        {
			mem[address-start_address] = data[i];     // move the data to memory
			address++;
        }
		delay_time = sc_core::SC_ZERO_TIME;
		gp.set_response_status(tlm::TLM_OK_RESPONSE);
		break;
		/*if(length==1)
			cout<<"write byte:	"<<*data<<endl;
		else if(length==2)
			cout<<"write half word:	"<<*reinterpret_cast<unsigned short *>(data)<<endl;
		else
			cout<<"write word:	"<<*reinterpret_cast<unsigned int *>(data)<<endl;
            delay_time = delay_time + m_write_delay;

        if (length == 4)
        {
            *((unsigned int*) (mem + address-start_address))=*reinterpret_cast<unsigned int *>(data);
			cout<<"write word:	"<<*reinterpret_cast<unsigned int *>(data)<<endl;
        }
        else if(length == 2)
        {
            *((unsigned short int*)(mem + address-start_address))=*reinterpret_cast<unsigned short int *>(data);
			cout<<"write half word:	"<<*reinterpret_cast<unsigned short int *>(data)<<endl;
        }
        else
        {
            mem[address-start_address]=*data;
			cout<<"write byte:	"<<*data<<endl;
        }
        gp.set_response_status(tlm::TLM_OK_RESPONSE);
        delay_time=sc_core::SC_ZERO_TIME;
        break;*/
    }

    case tlm::TLM_READ_COMMAND:
    {
		for (unsigned int i = 0; i < length; i++)
        {
			data[i] = mem[address-start_address];         // move the data to memory
			address++;
        }
		delay_time = sc_core::SC_ZERO_TIME;
		gp.set_response_status(tlm::TLM_OK_RESPONSE);
		break;
		/*
        if (length == 4)
        {
            unsigned int offset = address&0x00000003;
	        address = address - offset;
            unsigned int data_temp;
            data_temp=*((unsigned int*)(mem + address-start_address));
            *reinterpret_cast<unsigned int *>(data) = data_temp;
			cout<<"read word:	"<<data_temp<<endl;
        }
        else if(length == 2)
        {
            unsigned short int  data_temp;
	        data_temp = *((unsigned short int*)(mem + address-start_address));
            *reinterpret_cast<unsigned short int *>(data) = data_temp;
			cout<<"read half word:	"<<data_temp<<endl;
        }
        else
        {
            unsigned char data_temp;
            data_temp =  mem[address-start_address];
	        *data=data_temp;
			cout<<"read byte:	"<<data_temp<<endl;
        }
        gp.set_response_status(tlm::TLM_OK_RESPONSE);
        delay_time=sc_core::SC_ZERO_TIME;
        break;
		*/
    }
  } // end switch

  return;
} // end memory_operation
