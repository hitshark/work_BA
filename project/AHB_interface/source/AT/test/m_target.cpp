#include "my_target.h"                        // our header

using namespace std;

SC_HAS_PROCESS(my_target);

///Constructor

my_target::my_target
( sc_core::sc_module_name           module_name             /// instance name
, const unsigned int                ID                      /// target ID
, sc_dt::uint64                     memory_size             /// memory size (bytes)
, unsigned int                      memory_width            /// memory width (bytes)
, const sc_core::sc_time            accept_delay            /// accept delay (SC_TIME)
, const sc_core::sc_time            read_response_delay     /// read response delay (SC_TIME)
, const sc_core::sc_time            write_response_delay    /// write response delay (SC_TIME)
)
: sc_module                         (module_name)           /// instance name
, m_ID                              (ID)                    /// init target ID
//, m_memory_size                     (memory_size)           /// init memory size (bytes)
//, m_memory_width                    (memory_width)          /// init memory width (bytes)
, m_accept_delay                    (accept_delay)          /// init accept delay
//, m_read_response_delay             (read_response_delay)   /// init read response delay
//, m_write_response_delay            (write_response_delay)  /// init write response delay

, target_socket                     ("target_socket")
, m_end_request_PEQ                 ("end_request_PEQ")
, m_response_PEQ                    ("response_PEQ")

, m_target_memory                 /// init target's memory
  ( m_ID                          // initiator ID for messaging
  , read_response_delay         // delay for reads
  , write_response_delay        // delay for writes
  , memory_size                 // memory size (bytes)
  , memory_width                // memory width (bytes)
  )
{
    /// Bind the socket's export to the interface
    target_socket(*this);

    /// Register begin_reponse as an SC_METHOD
    SC_METHOD(end_request_method);
    sensitive << m_end_request_PEQ.get_event();
    dont_initialize();

    /// Register begin_reponse as an SC_METHOD
    SC_METHOD(begin_response_method);
    sensitive << m_response_PEQ.get_event();
    dont_initialize();
}

//==============================================================================
//  b_transport implementation calls from initiators
//
//=============================================================================
void
my_target::b_transport
( tlm::tlm_generic_payload  &payload                // ref to  Generic Payload
, sc_core::sc_time          &delay_time             // delay time
)
{
    sc_core::sc_time      mem_op_time;
    m_target_memory.operation(payload, mem_op_time);
    delay_time = delay_time + m_accept_delay + mem_op_time;
    return;
}

//=============================================================================
// nb_transport_fw implementation calls from initiators
//
//=============================================================================
tlm::tlm_sync_enum                                  // synchronization state
my_target::nb_transport_fw                  // non-blocking transport call through Bus
( tlm::tlm_generic_payload &gp                      // generic payoad pointer
, tlm::tlm_phase           &phase                   // transaction phase
, sc_core::sc_time         &delay_time)             // time it should take for transport
{
    tlm::tlm_sync_enum  return_status = tlm::TLM_COMPLETED;

//-----------------------------------------------------------------------------
// decode phase argument
//-----------------------------------------------------------------------------
    switch (phase)
    {
//=============================================================================
        case tlm::BEGIN_REQ:
        {
            sc_core::sc_time PEQ_delay_time = delay_time + m_accept_delay;
            m_end_request_PEQ.notify(gp, PEQ_delay_time); // put transaction in the PEQ
            return_status = tlm::TLM_ACCEPTED;
            break;
        } // end BEGIN_REQ

//=============================================================================
        case tlm::END_RESP:
        {
            m_end_resp_rcvd_event.notify (delay_time);
            return_status = tlm::TLM_COMPLETED;         // indicate end of transaction
            break;
        }

//=============================================================================
        case tlm::END_REQ:
        case tlm::BEGIN_RESP:
        {
            cout<<"illegal phase received by target -- end_req or begin_resp!"<<endl;
            return_status = tlm::TLM_ACCEPTED;
            break;
        }

//=============================================================================
        default:
        {
            cout<<"unknown phase received!"<<endl;
            return_status = tlm::TLM_ACCEPTED;
            break;
        }
    }
    return return_status;
} //end nb_transport_fw

//=============================================================================
/// end_request_method function implementation
//
// This method is statically sensitive to m_end_request_PEQ.get_event
//
//=============================================================================
void my_target::end_request_method ()
{
    tlm::tlm_generic_payload  *transaction_ptr;       // generic payload pointer
    tlm::tlm_sync_enum        status = tlm::TLM_COMPLETED;

//-----------------------------------------------------------------------------
//  Process all transactions scheduled for current time a return value of NULL
//  indicates that the PEQ is empty at this time
//-----------------------------------------------------------------------------
    while ((transaction_ptr = m_end_request_PEQ.get_next_transaction()) != NULL)
    {
        sc_core::sc_time delay  = sc_core::SC_ZERO_TIME;
        m_target_memory.get_delay(*transaction_ptr, delay); // get memory operation delay
        m_response_PEQ.notify(*transaction_ptr, delay);     // put transaction in the PEQ

        tlm::tlm_phase phase    = tlm::END_REQ;
        delay                   = sc_core::SC_ZERO_TIME;

//-----------------------------------------------------------------------------
// Call nb_transport_bw with phase END_REQ check the returned status
//-----------------------------------------------------------------------------
        status = target_socket->nb_transport_bw(*transaction_ptr, phase, delay);
        switch (status)
        {
//=============================================================================
            case tlm::TLM_ACCEPTED:
            {
                // more phases will follow
                break;
            }
//=============================================================================
            case tlm::TLM_COMPLETED:
            case tlm::TLM_UPDATED:
            {
                cout<<"tlm_completed is invalid response to end_req!"<<endl;
                break;
            }
//=============================================================================
            default:
            {
                cout<<"illegal return status!"<<endl;
                break;
            }
        }// end switch
    } // end while
} //end end_request_method

//=============================================================================
/// begin_response_method function implementation
//
// This method is statically sensitive to m_response_PEQ.get_event
//
//=============================================================================
void my_target::begin_response_method (void)
{
    tlm::tlm_generic_payload  *transaction_ptr;       // generic payload pointer
    tlm::tlm_sync_enum        status = tlm::TLM_COMPLETED;

//-----------------------------------------------------------------------------
//  Process all transactions scheduled for current time a return value of NULL
//  indicates that the PEQ is empty at this time
//-----------------------------------------------------------------------------
    // set default trigger, will be overridden by TLM_COMPLETED or TLM_UPDATED
    next_trigger (m_response_PEQ.get_event());
    while ((transaction_ptr = m_response_PEQ.get_next_transaction()) != NULL)
    {
        sc_core::sc_time delay  = sc_core::SC_ZERO_TIME;
        m_target_memory.operation(*transaction_ptr, delay); /// perform memory operation

        tlm::tlm_phase  phase = tlm::BEGIN_RESP;
        delay = sc_core::SC_ZERO_TIME;

//-----------------------------------------------------------------------------
// Call nb_transport_bw with phase BEGIN_RESP check the returned status
//-----------------------------------------------------------------------------
        status = target_socket->nb_transport_bw(*transaction_ptr, phase, delay);
        switch (status)
        {
//=============================================================================
            case tlm::TLM_COMPLETED:
            {
                next_trigger (delay);               // honor the annotated delay
                break;
            }
//=============================================================================
            case tlm::TLM_ACCEPTED:
            {
                next_trigger (m_end_resp_rcvd_event); // honor end-response rule
                break;
            }
//=============================================================================
            case tlm::TLM_UPDATED:
            {
                cout<<"invalid response to begin_resp!"<<endl;
                break;
            }
//=============================================================================
            default:
            {
                cout<<"illegal return status!"<<endl;
                break;
            }
        }// end switch
    } // end while
} //end begin_response_queue_active

//==============================================================================
// Methods Required by Target Interface but not Implemented for this Example

// Not implemented for this example but required by interface
bool
my_target::get_direct_mem_ptr
  (tlm::tlm_generic_payload   &payload,             ///< address + extensions
   tlm::tlm_dmi               &data                 ///< dmi data
  )
{
    cout<<"dmi is not implemented!"<<endl;
    return false;
}

// Not implemented for this example but required by interface
unsigned int                                        // result
my_target::transport_dbg
( tlm::tlm_generic_payload   &payload               ///< debug payload
)
{
    cout<<"transport debug is not implemented!"<<endl;
    return false;
}
