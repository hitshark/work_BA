#ifndef MY_TARGET_H_INCLUDED
#define MY_TARGET_H_INCLUDED
#include "tlm.h"                          		        // TLM headers
#include "C:\systemc-2.3.0\src\tlm_utils/peq_with_get.h"                     // Payload event queue FIFO
#include "my_memory.h"                                     // memory storage

class my_target                                       /// at_target_4_phase
: public sc_core::sc_module           	              /// inherit from SC module base clase
, virtual public tlm::tlm_fw_transport_if<>   	      /// inherit from TLM "forward interface"
{
// Member Methods =====================================================
public:
//=====================================================================
///	@fn my_target
///
///	@brief Constructor for Four Phase AT target
///
///	@details
///		Generic Four Phase target used in several examples.
///		Constructor offers several parameters for customization
///
//=====================================================================

  my_target
  ( sc_core::sc_module_name   module_name           ///< SC module name
  , const unsigned int        ID                    ///< target ID
  , sc_dt::uint64             memory_size           ///< memory size (bytes)
  , unsigned int              memory_width          ///< memory width (bytes)
  , const sc_core::sc_time    accept_delay          ///< accept delay (SC_TIME, SC_NS)
  , const sc_core::sc_time    read_response_delay   ///< read response delay (SC_TIME, SC_NS)
  , const sc_core::sc_time    write_response_delay  ///< write response delay (SC_TIME, SC_NS)
  );

//=====================================================================
///	@brief Implementation of call from Initiator.
//
///	@details
///		This is the ultimate destination of the nb_transport_fw call from
///		the initiator after being routed trough a Bus
//
//=====================================================================

  tlm::tlm_sync_enum                                // sync status
  nb_transport_fw
  ( tlm::tlm_generic_payload &gp                    ///< generic payoad pointer
  , tlm::tlm_phase           &phase                 ///< transaction phase
  , sc_core::sc_time         &delay_time            ///< time taken for transport
  );

  //=====================================================================
  ///  @fn my_target::end_request_method
  ///
  ///  @brief End Request Processing
  ///
  ///  @details
  ///                   ????
  //=====================================================================
  void end_request_method();

  //=====================================================================
  ///  @fn my_target::begin_response_method
  ///
  ///  @brief Response Processing
  ///
  ///  @details
  ///    This routine takes transaction responses from the m_response_PEQ.
  ///    It contains the state machine to manage the communication path
  ///    back to the initiator. This method is registered as an SC_METHOD
  ///    with the SystemC kernal and is sensitive to m_response_PEQ.get_event()
  //=====================================================================
   void begin_response_method();

private:
//==============================================================================
// Methods not Implemented for this Example

/// b_transport() - Blocking Transport
  void b_transport                                    // returns nothing
  ( tlm::tlm_generic_payload  &payload                // ref to payload
  , sc_core::sc_time          &delay_time             // delay time
  );

/// Not implemented for this example but required by interface
  bool get_direct_mem_ptr
  ( tlm::tlm_generic_payload   &payload,            // address + extensions
    tlm::tlm_dmi               &dmi_data            // DMI data
  );

/// Not implemented for this example but required by interface
  unsigned int transport_dbg
  ( tlm::tlm_generic_payload  &payload              // debug payload
  );

// Member Variables ===================================================
public:
  typedef tlm::tlm_generic_payload  *gp_ptr;		///< generic payload pointer

  /// @todo Should this be the convenience scoket from utilities?
  /// for clarity and templated types with "con" in the title
  tlm::tlm_target_socket<>  target_socket;          ///<  target socket

private:
  const unsigned int        m_ID;                   ///< target ID
        //sc_dt::uint64       m_memory_size;          ///< memory size (bytes)
        //unsigned int        m_memory_width;         ///< word size (bytes)
  const sc_core::sc_time    m_accept_delay;         ///< accept delay
  //const sc_core::sc_time    m_read_response_delay;  ///< read response delay
  //const sc_core::sc_time    m_write_response_delay; ///< write response delays

        tlm_utils::peq_with_get<tlm::tlm_generic_payload>
                            m_end_request_PEQ;      ///< response payload event queue
        sc_core::sc_event   m_end_resp_rcvd_event;
        tlm_utils::peq_with_get<tlm::tlm_generic_payload>
                            m_response_PEQ;         ///< response payload event queue
        my_memory           m_target_memory;
};


#endif // MY_TARGET_H_INCLUDED
