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

#endif /* INC_SCHEDULER_EVAL_H_ */
