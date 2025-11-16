//==================================================================================
//==================================================================================
// File: cMonitor.h
// Description: Performance/Load Monitor header for execution time monitoring
//
// Copyright (c) 2025 Dad Design.
//==================================================================================
//==================================================================================

#pragma once

#include "main.h"
#include <cstdint>
#include <limits>

namespace DadUtilities {

//**********************************************************************************
// Class cMonitor: encapsulates execution time monitoring routines
//**********************************************************************************

class cMonitor {

public:

    // =============================================================================
    // Constructor
    // =============================================================================

    // -----------------------------------------------------------------------------
    cMonitor();

    // =============================================================================
    // Public methods
    // =============================================================================

    // -----------------------------------------------------------------------------
    // Initialize monitoring system
    void Init();

    // -----------------------------------------------------------------------------
    // DWT initialization (static, done once)
    static void initDWT();

    // -----------------------------------------------------------------------------
    // Start monitoring (to be called at the beginning of the function to monitor)
    inline void startMonitoring() {
        if (!m_monitoring_active) {
            uint32_t current_cycles = DWT->CYCCNT;  // Get current cycle count

            // Compute call period between consecutive calls
            if (m_last_call_cycles != 0 && m_call_count > 0) {
                uint32_t period = current_cycles - m_last_call_cycles;  // Calculate time since last call
                m_total_period_cycles += period;                        // Accumulate total period

                // Update minimum and maximum period statistics
                if (period < m_min_period_cycles) {
                    m_min_period_cycles = period;
                }
                if (period > m_max_period_cycles) {
                    m_max_period_cycles = period;
                }
            }

            m_last_call_cycles = current_cycles;  // Store current cycle for next period calculation
            m_start_cycles = current_cycles;      // Set start point for execution time measurement
            m_monitoring_active = true;           // Mark monitoring as active
        }
    }

    // -----------------------------------------------------------------------------
    // Stop monitoring (to be called at the end of the function to monitor)
    inline void stopMonitoring() {
        if (m_monitoring_active) {
            uint32_t execution_cycles = DWT->CYCCNT - m_start_cycles;  // Calculate elapsed cycles

            // Update execution statistics
            m_total_execution_cycles += execution_cycles;  // Accumulate total execution time
            m_call_count++;                                // Increment call counter

            // Update minimum and maximum execution time statistics
            if (execution_cycles < m_min_execution_cycles) {
                m_min_execution_cycles = execution_cycles;
            }
            if (execution_cycles > m_max_execution_cycles) {
                m_max_execution_cycles = execution_cycles;
            }

            m_monitoring_active = false;  // Mark monitoring as inactive
        }
    }

    // -----------------------------------------------------------------------------
    // Reset all statistics to initial values
    void reset();

    // -----------------------------------------------------------------------------
    // Getters for execution time in microseconds
    float getAverageExecutionTime_us() const;
    float getMinExecutionTime_us() const;
    float getMaxExecutionTime_us() const;

    // -----------------------------------------------------------------------------
    // Getters for call frequency in Hertz
    float getAverageFrequency_Hz() const;
    float getMinFrequency_Hz() const;
    float getMaxFrequency_Hz() const;

    // -----------------------------------------------------------------------------
    // CPU load estimation based on execution time and call frequency
    float getCPULoad_percent() const;

    // -----------------------------------------------------------------------------
    // Getters for raw data
    inline uint32_t getCallCount() const { return m_call_count; }  // Get total number of calls
    uint32_t getAverageExecutionCycles() const;                    // Get average execution cycles
    uint32_t getMinExecutionCycles() const;                        // Get minimum execution cycles
    inline uint32_t getMaxExecutionCycles() const {
        return m_max_execution_cycles;                             // Get maximum execution cycles
    }

private:

    // =============================================================================
    // Execution time statistics
    // =============================================================================

    volatile uint32_t m_call_count;              // Total number of function calls
    volatile uint32_t m_total_execution_cycles;  // Sum of all execution cycles
    volatile uint32_t m_min_execution_cycles;    // Minimum execution cycles
    volatile uint32_t m_max_execution_cycles;    // Maximum execution cycles
    volatile uint32_t m_start_cycles;            // Cycle count at monitoring start

    // =============================================================================
    // Frequency statistics
    // =============================================================================

    volatile uint32_t m_last_call_cycles;        // Cycle count at last call
    volatile uint32_t m_total_period_cycles;     // Sum of all period cycles between calls
    volatile uint32_t m_min_period_cycles;       // Minimum period between calls
    volatile uint32_t m_max_period_cycles;       // Maximum period between calls

    // =============================================================================
    // Configuration
    // =============================================================================

    uint32_t m_cpu_frequency;                    // CPU frequency in Hz
    bool m_monitoring_active;                    // Current monitoring state

    // =============================================================================
    // Static members
    // =============================================================================

    static bool m_dwt_initialized;               // DWT initialization flag
};

} // namespace DadUtilities

//***End of file**************************************************************
