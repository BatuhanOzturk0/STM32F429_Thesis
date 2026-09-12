/*
 * scheduler_eval.c
 *
 *  Created on: 22 Aug 2026
 *      Author: batuhanozturk
 */

#include <stdio.h>
#include "scheduler_eval.h"

/**
 * @brief  Enables the Cortex-M4 DWT cycle counter (DWT->CYCCNT), which
 *         increments once per CPU clock cycle while the core runs.
 *         Must be called once, early in main(), before any
 *         SchedEval_GetCycles() calls are used for measurement.
 * @retval None
 */
void SchedEval_InitDWT(void)
{
    /* Enable the core's general trace/debug subsystem - CYCCNT lives
     * inside it and does not count unless this is set first. */
    CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;

    /* Reset the counter to a known state before starting to count. */
    DWT->CYCCNT = 0;

    /* Start the cycle counter itself. */
    DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;
}

/**
 * @brief  Returns the current raw DWT cycle count. Call once before and
 *         once after the code block being measured; the difference is
 *         the number of CPU cycles that block took to execute.
 * @retval Current value of DWT->CYCCNT
 */
uint32_t SchedEval_GetCycles(void)
{
    return DWT->CYCCNT;
}

/**
 * @brief  Fills a measurement record from a start/end cycle pair,
 *         converting the raw cycle count into microseconds.
 * @param  m: pointer to the measurement struct to fill
 * @param  label: human-readable name for what was measured
 * @param  start_cycles: DWT->CYCCNT value captured before the code ran
 * @param  end_cycles: DWT->CYCCNT value captured after the code ran
 * @retval None
 * @note   DWT->CYCCNT is a free-running 32-bit counter and wraps around
 *         after it overflows. At 180 MHz, this happens roughly every
 *         ~23.8 seconds. Simple unsigned subtraction (end - start)
 *         still gives the correct elapsed cycle count even across a
 *         single wraparound, thanks to unsigned integer arithmetic -
 *         no special handling is needed here as long as the measured
 *         block itself takes far less than ~23.8 seconds, which holds
 *         for every measurement in this project.
 */
void SchedEval_Record(SchedEval_Measurement_t *m, const char *label,
                       uint32_t start_cycles, uint32_t end_cycles)
{
    m->label       = label;
    m->cycles      = end_cycles - start_cycles;
    m->duration_us = SCHED_EVAL_CYCLES_TO_US(m->cycles);
}

/**
 * @brief  Prints one measurement record to the terminal in a fixed,
 *         table-friendly format (easy to copy into a results table
 *         later, one row per printed line).
 * @param  m: pointer to the measurement record to print
 * @retval None
 */
void SchedEval_PrintMeasurement(const SchedEval_Measurement_t *m)
{
    printf("[LATENCY] %-20s cycles=%8lu  duration_us=%6lu\r\n",
           m->label,
           (unsigned long)m->cycles,
           (unsigned long)m->duration_us);
}
/* ===================================================================
 * Accumulating latency statistics implementation
 * =================================================================== */

void SchedEval_StatsInit(SchedEval_LatencyStats_t *stats, const char *label,
                          uint32_t deadline_us)
{
    stats->label = label;
    stats->deadline_us = deadline_us;
    stats->count = 0;
    stats->min_us = UINT32_MAX;
    stats->max_us = 0;
    stats->sum_us = 0;
    stats->deadline_miss_count = 0;
}

void SchedEval_StatsUpdate(SchedEval_LatencyStats_t *stats, uint32_t duration_us)
{
    stats->count++;
    stats->sum_us += duration_us;

    if (duration_us < stats->min_us)
    {
        stats->min_us = duration_us;
    }
    if (duration_us > stats->max_us)
    {
        stats->max_us = duration_us;
    }
    if (duration_us > stats->deadline_us)
    {
        stats->deadline_miss_count++;
    }
}

void SchedEval_StatsPrint(const SchedEval_LatencyStats_t *stats)
{
    uint32_t avg_us = (stats->count > 0)
                       ? (uint32_t)(stats->sum_us / stats->count)
                       : 0;
    float miss_rate = (stats->count > 0)
                       ? (100.0f * (float)stats->deadline_miss_count / (float)stats->count)
                       : 0.0f;

    printf("[STATS] %-8s count=%lu min=%luus max=%luus avg=%luus deadline=%luus misses=%lu (%.2f%%)\r\n",
           stats->label,
           (unsigned long)stats->count,
           (unsigned long)stats->min_us,
           (unsigned long)stats->max_us,
           (unsigned long)avg_us,
           (unsigned long)stats->deadline_us,
           (unsigned long)stats->deadline_miss_count,
           miss_rate);
}
