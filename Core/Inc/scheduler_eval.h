/*
 * scheduler_eval.h
 *
 *  Created on: 22 Aug 2026
 *      Author: batuhanozturk
 */

#ifndef INC_SCHEDULER_EVAL_H_
#define INC_SCHEDULER_EVAL_H_

#include "stm32f4xx_hal.h"
#include <stdint.h>

/* Core clock frequency, used to convert raw DWT cycle counts into
 * microseconds. Matches SystemCoreClock after SystemClock_Config()
 * has run (180 MHz on this STM32F429 project). Read at runtime via
 * SystemCoreClock instead of a hardcoded constant, so this stays
 * correct even if the clock configuration changes later. */
#define SCHED_EVAL_CYCLES_TO_US(cycles)  \
    ((uint32_t)(((uint64_t)(cycles) * 1000000ULL) / SystemCoreClock))

/* One measurement record: what was measured and how long it took */
typedef struct
{
    const char *label;        /* human-readable name, e.g. "IMU_ReadAccel" */
    uint32_t    cycles;       /* raw DWT cycle count for this measurement */
    uint32_t    duration_us;  /* cycles converted to microseconds */
} SchedEval_Measurement_t;

/* Public API */
void     SchedEval_InitDWT(void);
uint32_t SchedEval_GetCycles(void);
void     SchedEval_Record(SchedEval_Measurement_t *m, const char *label,
                           uint32_t start_cycles, uint32_t end_cycles);
void     SchedEval_PrintMeasurement(const SchedEval_Measurement_t *m);

/* Convenience macros for wrapping a block of code to measure.
 * Usage:
 *   uint32_t _start = SCHED_EVAL_START();
 *   ... code to measure ...
 *   SCHED_EVAL_STOP_AND_PRINT(_start, "IMU_ReadAccel");
 */
#define SCHED_EVAL_START()   SchedEval_GetCycles()
#define SCHED_EVAL_STOP_AND_PRINT(start_var, label_str)                 \
    do {                                                                \
        uint32_t _end = SchedEval_GetCycles();                          \
        SchedEval_Measurement_t _m;                                     \
        SchedEval_Record(&_m, (label_str), (start_var), _end);          \
        SchedEval_PrintMeasurement(&_m);                                \
    } while (0)

/* ===================================================================
 * Accumulating latency statistics (for scheduling config evaluation)
 * =================================================================== */

/* Deadlines, derived from the effective DSP window period (~35.55 ms,
 * ~28.1 Hz — see thesis §4.9) multiplied by each DSP function's window
 * size (128 samples for FIR/IIR/RMS/Peak, 256 for FFT). ... */
#define SCHED_DEADLINE_US_FFT256   9101000U   /* 256 * 35.55 ms */
#define SCHED_DEADLINE_US_FFT512   18202000U  /* 512 * 35.55 ms (not used in continuous eval) */
#define SCHED_DEADLINE_US_FFT1024  36403000U  /* 1024 * 35.55 ms (not used in continuous eval) */
#define SCHED_DEADLINE_US_FIR      4550000U   /* 128 * 35.55 ms */
#define SCHED_DEADLINE_US_IIR      4550000U   /* 128 * 35.55 ms */
#define SCHED_DEADLINE_US_RMS      4550000U   /* 128 * 35.55 ms */
#define SCHED_DEADLINE_US_PEAK     4550000U   /* 128 * 35.55 ms */


/* Running statistics for one DSP operation type over many samples.
 * Tracks min/max/avg latency and deadline miss count without storing
 * every individual measurement (constant memory, O(1) update). */
typedef struct
{
    const char *label;             /* e.g. "FFT256", "FIR", "RMS" */
    uint32_t    deadline_us;       /* deadline for this operation type */
    uint32_t    count;             /* number of samples accumulated */
    uint32_t    min_us;
    uint32_t    max_us;
    uint64_t    sum_us;            /* for computing average; 64-bit to avoid overflow */
    uint32_t    deadline_miss_count;
} SchedEval_LatencyStats_t;

void SchedEval_StatsInit(SchedEval_LatencyStats_t *stats, const char *label,
                          uint32_t deadline_us);
void SchedEval_StatsUpdate(SchedEval_LatencyStats_t *stats, uint32_t duration_us);
void SchedEval_StatsPrint(const SchedEval_LatencyStats_t *stats);


/* Like SCHED_EVAL_STOP_AND_PRINT, but accumulates into a running
 * SchedEval_LatencyStats_t instead of printing every single sample.
 * Usage:
 *   uint32_t _start = SCHED_EVAL_START();
 *   ... code to measure ...
 *   SCHED_EVAL_STOP_AND_ACCUMULATE(_start, &g_stats_fft256);
 */
#define SCHED_EVAL_STOP_AND_ACCUMULATE(start_var, stats_ptr)             \
    do {                                                                 \
        uint32_t _end_acc = SchedEval_GetCycles();                       \
        uint32_t _cycles_acc = _end_acc - (start_var);                   \
        uint32_t _dur_us_acc = SCHED_EVAL_CYCLES_TO_US(_cycles_acc);     \
        SchedEval_StatsUpdate((stats_ptr), _dur_us_acc);                 \
    } while (0)
#endif /* INC_SCHEDULER_EVAL_H_ */
