//==================================================================================
//==================================================================================
// File: cMonitor.cpp
// Description: Performance/Load Monitor implementation for execution time monitoring
//
// Copyright (c) 2025 Dad Design.
//==================================================================================
//==================================================================================

#include "cMonitor.h"

namespace DadUtilities {

//**********************************************************************************
// Class cMonitor: encapsulates execution time monitoring routines
//**********************************************************************************

// =============================================================================
// Static member initialization
// =============================================================================

bool cMonitor::m_dwt_initialized = false;  // DWT initialization flag

// =============================================================================
// Constructor
// =============================================================================

// -----------------------------------------------------------------------------
cMonitor::cMonitor()
{
    m_call_count = 0;                    // Total number of function calls
    m_total_execution_cycles = 0;        // Sum of all execution cycles
    m_min_execution_cycles = UINT32_MAX; // Minimum execution cycles
    m_max_execution_cycles = 0;          // Maximum execution cycles
    m_start_cycles = 0;                  // Cycle count at monitoring start

    // Frequency statistics
    m_last_call_cycles = 0;              // Cycle count at last call
    m_total_period_cycles = 0;           // Sum of all period cycles between calls
    m_min_period_cycles = UINT32_MAX;    // Minimum period between calls
    m_max_period_cycles = 0;             // Maximum period between calls

    // Configuration
    m_cpu_frequency = SystemCoreClock;   // CPU frequency in Hz
    m_monitoring_active = false;         // Monitoring state flag
}

// =============================================================================
// Public methods
// =============================================================================

// -----------------------------------------------------------------------------
// Initialize monitoring system
void cMonitor::Init()
{
    m_start_cycles = 0;                  // Reset start cycle counter
    m_last_call_cycles = 0;              // Reset last call cycle counter
    SystemCoreClockUpdate();             // Update system core clock
    m_cpu_frequency = SystemCoreClock;   // Store current CPU frequency
    m_monitoring_active = false;         // Set monitoring as inactive
    reset();                             // Reset all statistics
    initDWT();                           // Initialize DWT cycle counter
}

// -----------------------------------------------------------------------------
// Reset all statistics to initial values
void cMonitor::reset()
{
    m_call_count = 0;                    // Reset call counter
    m_total_execution_cycles = 0;        // Reset total execution cycles
    m_min_execution_cycles = UINT32_MAX; // Reset minimum execution cycles
    m_max_execution_cycles = 0;          // Reset maximum execution cycles
    m_total_period_cycles = 0;           // Reset total period cycles
    m_min_period_cycles = UINT32_MAX;    // Reset minimum period cycles
    m_max_period_cycles = 0;             // Reset maximum period cycles
    m_last_call_cycles = 0;              // Reset last call cycle counter
}

// -----------------------------------------------------------------------------
// Get average execution time in microseconds
float cMonitor::getAverageExecutionTime_us() const
{
    if (m_call_count == 0) return 0.0f;  // Handle zero calls case

    // Calculate average cycles and convert to microseconds
    float avg_cycles = (float)m_total_execution_cycles / m_call_count;
    return (avg_cycles * 1000000.0f) / m_cpu_frequency;
}

// -----------------------------------------------------------------------------
// Get minimum execution time in microseconds
float cMonitor::getMinExecutionTime_us() const
{
    if (m_min_execution_cycles == UINT32_MAX) return 0.0f;  // Handle no measurements

    // Convert minimum cycles to microseconds
    return ((float)m_min_execution_cycles * 1000000.0f) / m_cpu_frequency;
}

// -----------------------------------------------------------------------------
// Get maximum execution time in microseconds
float cMonitor::getMaxExecutionTime_us() const
{
    // Convert maximum cycles to microseconds
    return ((float)m_max_execution_cycles * 1000000.0f) / m_cpu_frequency;
}

// -----------------------------------------------------------------------------
// Get average call frequency in Hertz
float cMonitor::getAverageFrequency_Hz() const
{
    if (m_call_count <= 1) return 0.0f;  // Need at least 2 calls for frequency

    // Calculate average period and convert to frequency
    float m_avg_period_cycles = (float)m_total_period_cycles / (m_call_count - 1);
    return (float)m_cpu_frequency / m_avg_period_cycles;
}

// -----------------------------------------------------------------------------
// Get minimum call frequency in Hertz
float cMonitor::getMinFrequency_Hz() const
{
    if (m_max_period_cycles == 0) return 0.0f;  // Handle no period measurements

    // Maximum period gives minimum frequency
    return (float)m_cpu_frequency / m_max_period_cycles;
}

// -----------------------------------------------------------------------------
// Get maximum call frequency in Hertz
float cMonitor::getMaxFrequency_Hz() const
{
    if (m_min_period_cycles == UINT32_MAX) return 0.0f;  // Handle no period measurements

    // Minimum period gives maximum frequency
    return (float)m_cpu_frequency / m_min_period_cycles;
}

// -----------------------------------------------------------------------------
// Estimate CPU load percentage based on execution time and frequency
float cMonitor::getCPULoad_percent() const
{
    if (m_call_count <= 1) return 0.0f;  // Need multiple calls for load calculation

    // Calculate CPU load as (execution time * frequency) converted to percentage
    float avg_exec_time = getAverageExecutionTime_us();
    float avg_frequency = getAverageFrequency_Hz();
    return (avg_exec_time * avg_frequency) / 10000.0f;  // Convert to percentage
}

// -----------------------------------------------------------------------------
// Get average execution cycles (raw data)
uint32_t cMonitor::getAverageExecutionCycles() const
{
    if (m_call_count == 0) return 0;     // Handle zero calls case

    return m_total_execution_cycles / m_call_count;  // Calculate average cycles
}

// -----------------------------------------------------------------------------
// Get minimum execution cycles (raw data)
uint32_t cMonitor::getMinExecutionCycles() const
{
    return m_min_execution_cycles == UINT32_MAX ? 0 : m_min_execution_cycles;
}

// =============================================================================
// Protected methods
// =============================================================================

// -----------------------------------------------------------------------------
// Initialize DWT (Data Watchpoint and Trace) cycle counter (static method)
void cMonitor::initDWT()
{
    if (!m_dwt_initialized) {
        CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;   // Enable trace system
        DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;              // Enable cycle counter
        DWT->CYCCNT = 0;                                  // Reset cycle counter
        m_dwt_initialized = true;                         // Mark as initialized
    }
}

} // namespace DadUtilities

//***End of file**************************************************************
