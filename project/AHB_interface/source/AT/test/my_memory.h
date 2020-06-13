#ifndef MY_MEMORY_H_INCLUDED
#define MY_MEMORY_H_INCLUDED
#include "tlm.h"                                // TLM headers

class my_memory
{
// Member Methods  ====================================================
public:
//=====================================================================
/// @fn my_memory.h
//
///  @brief memory Constructor
//
///  @details
//       Initialize member variables, include allocating and initializing
//       the actual memory
//
//=====================================================================
    my_memory
    (
    const unsigned int ID                 ///< initiator ID for messaging
    , sc_core::sc_time   read_delay         ///< delay for reads
    , sc_core::sc_time   write_delay        ///< delay for writes
    , sc_dt::uint64      memory_size        ///< memory size (bytes)
    , unsigned int       memory_width       ///< memory width (bytes)
    );

    ~my_memory(){delete [] m_memory;}

 //====================================================================
 /// @fn operation
 ///
 ///  @brief Performs the Memory Operation specified in the GP
 ///
 ///  @details
 ///    Performs the operation specified by the GP
 ///    Returns after updating the status of the GP (if required)
 ///    and updating the time based upon initialization parameters
 ///
 ///===================================================================
    void
    operation(
            tlm::tlm_generic_payload  &gp             ///< TLM2 GP reference
            , sc_core::sc_time          &delay_time   ///< transaction delay
            );

 //====================================================================
 /// @fn get_delay
 ///
 ///  @brief Looks at GP and returns delay without doing GP Operation
 ///
 ///  @details
 ///    Performs the operation specified by the GP
 ///    Returns after updating the status of the GP (if required)
 ///    and updating the time based upon initialization parameters
 ///
 ///===================================================================
    void
    get_delay(
            tlm::tlm_generic_payload  &gp           ///< TLM2 GP reference
            , sc_core::sc_time          &delay_time   ///< time to be updated
            );

    unsigned char* get_mem_ptr(void);

private:
    /// Check the address vs. range passed at construction
    tlm::tlm_response_status
    check_address(tlm::tlm_generic_payload  &gp);

// Member Variables/Objects  ===================================================

private:
    unsigned int          m_ID;                    ///< initiator ID
    sc_core::sc_time      m_read_delay;            ///< read delay
    sc_core::sc_time      m_write_delay;           ///< write delay
    sc_dt::uint64         m_memory_size;           ///< memory size (bytes)
    unsigned int          m_memory_width;          ///< memory width (bytes)
    unsigned char         *m_memory;               ///< memory
};

#endif // MY_MEMORY_H_INCLUDED
