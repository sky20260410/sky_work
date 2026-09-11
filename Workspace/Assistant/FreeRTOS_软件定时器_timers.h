/*
 * FreeRTOS Kernel Software Timer Header
 * Source: https://raw.githubusercontent.com/FreeRTOS/FreeRTOS-Kernel/main/include/timers.h
 * License: MIT
 * Branch: main (latest)
 */

#ifndef TIMERS_H
#define TIMERS_H

#include "FreeRTOS.h"
#include "task.h"

/* ========== Timer Command IDs ========== */
/* Task-level commands */
#define tmrCOMMAND_EXECUTE_CALLBACK_FROM_ISR    ( ( BaseType_t ) -2 )
#define tmrCOMMAND_EXECUTE_CALLBACK             ( ( BaseType_t ) -1 )
#define tmrCOMMAND_START_DONT_TRACE             ( ( BaseType_t ) 0 )
#define tmrCOMMAND_START                        ( ( BaseType_t ) 1 )
#define tmrCOMMAND_RESET                        ( ( BaseType_t ) 2 )
#define tmrCOMMAND_STOP                         ( ( BaseType_t ) 3 )
#define tmrCOMMAND_CHANGE_PERIOD                ( ( BaseType_t ) 4 )
#define tmrCOMMAND_DELETE                       ( ( BaseType_t ) 5 )

/* ISR-level commands (must use >= tmrFIRST_FROM_ISR_COMMAND) */
#define tmrFIRST_FROM_ISR_COMMAND               ( ( BaseType_t ) 6 )
#define tmrCOMMAND_START_FROM_ISR               ( ( BaseType_t ) 6 )
#define tmrCOMMAND_RESET_FROM_ISR               ( ( BaseType_t ) 7 )
#define tmrCOMMAND_STOP_FROM_ISR                ( ( BaseType_t ) 8 )
#define tmrCOMMAND_CHANGE_PERIOD_FROM_ISR       ( ( BaseType_t ) 9 )

/* ========== Type Definitions ========== */
struct tmrTimerControl;
typedef struct tmrTimerControl * TimerHandle_t;

/* Callback function prototype - called when timer expires */
typedef void (* TimerCallbackFunction_t)( TimerHandle_t xTimer );

/* Pended function prototype (for xTimerPendFunctionCallFromISR) */
typedef void (* PendedFunction_t)( void * pvParameter1, uint32_t ulParameter2 );

/* ========== Timer Creation APIs ========== */
/* Dynamic allocation - requires configSUPPORT_DYNAMIC_ALLOCATION = 1 */
#if ( configSUPPORT_DYNAMIC_ALLOCATION == 1 )
TimerHandle_t xTimerCreate(
    const char * const pcTimerName,         /* Debug name (for debugger) */
    const TickType_t xTimerPeriodInTicks,   /* Period in ticks (must be > 0) */
    const BaseType_t xAutoReload,            /* pdTRUE = auto-reload, pdFALSE = one-shot */
    void * const pvTimerID,                /* User identifier passed to callback */
    TimerCallbackFunction_t pxCallbackFunction /* Callback function pointer */
);
/* Returns: Handle on success, NULL if heap exhausted */
#endif

/* Static allocation - requires configSUPPORT_STATIC_ALLOCATION = 1 */
#if ( configSUPPORT_STATIC_ALLOCATION == 1 )
typedef struct xTIMER {
    uint8_t ucDummy;
} StaticTimer_t;

TimerHandle_t xTimerCreateStatic(
    const char * const pcTimerName,
    const TickType_t xTimerPeriodInTicks,
    const BaseType_t xAutoReload,
    void * const pvTimerID,
    TimerCallbackFunction_t pxCallbackFunction,
    StaticTimer_t * pxTimerBuffer  /* Caller-provided storage buffer */
);
/* Returns: Handle if pxTimerBuffer non-NULL, else NULL */
#endif

/* ========== Timer Control Functions ========== */
/* Task context (blocking) */
BaseType_t xTimerStart( TimerHandle_t xTimer, TickType_t xTicksToWait );
BaseType_t xTimerStop( TimerHandle_t xTimer, TickType_t xTicksToWait );
BaseType_t xTimerReset( TimerHandle_t xTimer, TickType_t xTicksToWait );
BaseType_t xTimerChangePeriod( TimerHandle_t xTimer, TickType_t xNewPeriod, TickType_t xTicksToWait );
BaseType_t xTimerDelete( TimerHandle_t xTimer, TickType_t xTicksToWait );

/* ISR context (non-blocking, uses pxHigherPriorityTaskWoken) */
BaseType_t xTimerStartFromISR( TimerHandle_t xTimer, BaseType_t * pxHigherPriorityTaskWoken );
BaseType_t xTimerStopFromISR( TimerHandle_t xTimer, BaseType_t * pxHigherPriorityTaskWoken );
BaseType_t xTimerResetFromISR( TimerHandle_t xTimer, BaseType_t * pxHigherPriorityTaskWoken );
BaseType_t xTimerChangePeriodFromISR( TimerHandle_t xTimer, TickType_t xNewPeriod, BaseType_t * pxHigherPriorityTaskWoken );

/* ========== Timer ID Management ========== */
/* Distinguish which timer triggered when multiple timers share one callback */
void * pvTimerGetTimerID( TimerHandle_t xTimer );
void vTimerSetTimerID( TimerHandle_t xTimer, void * pvNewID );

/* ========== Timer Query Functions ========== */
BaseType_t xTimerIsTimerActive( TimerHandle_t xTimer );         /* pdFALSE if dormant */
TickType_t xTimerGetPeriod( TimerHandle_t xTimer );             /* Period in ticks */
TickType_t xTimerGetExpiryTime( TimerHandle_t xTimer );         /* Next expiry tick */
const char * pcTimerGetName( TimerHandle_t xTimer );            /* Timer name */
TaskHandle_t xTimerGetTimerDaemonTaskHandle( void );            /* Daemon task handle */
BaseType_t xTimerGetReloadMode( TimerHandle_t xTimer );          /* pdTRUE if auto-reload */
void vTimerSetReloadMode( TimerHandle_t xTimer, BaseType_t xAutoReload );

/* ========== Pended Function Call (ISR to Task) ========== */
/* Execute a function in task context from ISR */
#if ( INCLUDE_xTimerPendFunctionCall == 1 )
BaseType_t xTimerPendFunctionCallFromISR( PendedFunction_t xFunction,
                                          void * pvParameter1,
                                          uint32_t ulParameter2,
                                          BaseType_t * pxHigherPriorityTaskWoken );
/* Task-level version */
BaseType_t xTimerPendFunctionCall( PendedFunction_t xFunction,
                                   void * pvParameter1,
                                   uint32_t ulParameter2,
                                   TickType_t xTicksToWait );
#endif

/* ========== Internal Helper (used by macros) ========== */
/* Low-level command sender - all timer functions call this internally */
BaseType_t xTimerGenericCommand( TimerHandle_t xTimer,
                                  BaseType_t xCommandID,
                                  TickType_t xOptionalValue,
                                  BaseType_t * pxHigherPriorityTaskWoken,
                                  TickType_t xTicksToWait );

#endif /* TIMERS_H */
