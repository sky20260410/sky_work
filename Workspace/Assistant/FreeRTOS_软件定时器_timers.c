/*
 * FreeRTOS Kernel Software Timer Implementation
 * Source: https://raw.githubusercontent.com/FreeRTOS/FreeRTOS-Kernel/main/timers.c
 * License: MIT
 * Branch: main (latest)
 */

/*
 * Configuration Requirements (in FreeRTOSConfig.h):
 *   configUSE_TIMERS = 1
 *   configTIMER_SERVICE_TASK_NAME = "Tmr Svc"
 *   configTIMER_QUEUE_LENGTH
 *   configSUPPORT_DYNAMIC_ALLOCATION (for xTimerCreate)
 *   configSUPPORT_STATIC_ALLOCATION (for xTimerCreateStatic)
 */

/* ========== Core Constants & Macros ========== */
#define tmrNO_DELAY                    ( ( TickType_t ) 0U )
#define tmrMAX_TIME_BEFORE_OVERFLOW    ( ( TickType_t ) -1 )

/* Timer Status Bits */
#define tmrSTATUS_IS_ACTIVE                   ( 0x01U )
#define tmrSTATUS_IS_STATICALLY_ALLOCATED     ( 0x02U )
#define tmrSTATUS_IS_AUTORELOAD               ( 0x04U )

/* Timer Commands (Message IDs) */
#define tmrCOMMAND_START_DONT_TRACE             ( ( BaseType_t ) 0 )
#define tmrCOMMAND_START                        ( ( BaseType_t ) 1 )
#define tmrCOMMAND_RESET                        ( ( BaseType_t ) 2 )
#define tmrCOMMAND_STOP                         ( ( BaseType_t ) 3 )
#define tmrCOMMAND_CHANGE_PERIOD                ( ( BaseType_t ) 4 )
#define tmrCOMMAND_DELETE                       ( ( BaseType_t ) 5 )
#define tmrFIRST_FROM_ISR_COMMAND               ( ( BaseType_t ) 6 )
#define tmrCOMMAND_START_FROM_ISR               ( ( BaseType_t ) 6 )
#define tmrCOMMAND_RESET_FROM_ISR               ( ( BaseType_t ) 7 )
#define tmrCOMMAND_STOP_FROM_ISR                ( ( BaseType_t ) 8 )
#define tmrCOMMAND_CHANGE_PERIOD_FROM_ISR        ( ( BaseType_t ) 9 )

/* ========== Timer Control Structure ========== */
typedef struct tmrTimerControl {
    const char * pcTimerName;                                   /* Debug name */
    ListItem_t xTimerListItem;                                 /* Linked list item */
    TickType_t xTimerPeriodInTicks;                            /* Timer period */
    void * pvTimerID;                                          /* User identifier */
    portTIMER_CALLBACK_ATTRIBUTE
    TimerCallbackFunction_t pxCallbackFunction;                 /* Callback function */
    #if ( configUSE_TRACE_FACILITY == 1 )
        UBaseType_t uxTimerNumber;                              /* Trace tool ID */
    #endif
    uint8_t ucStatus;                                           /* Status flags */
} xTIMER;

typedef xTIMER Timer_t;

/* ========== Timer Queue Message ========== */
typedef struct tmrTimerQueueMessage {
    BaseType_t xMessageID;
    union {
        TimerParameter_t xTimerParameters;
        #if ( INCLUDE_xTimerPendFunctionCall == 1 )
            CallbackParameters_t xCallbackParameters;
        #endif
    } u;
} DaemonTaskMessage_t;

/* ========== Core API Implementation ========== */

/* xTimerCreate - Dynamic allocation */
TimerHandle_t xTimerCreate( const char * const pcTimerName,
                            const TickType_t xTimerPeriodInTicks,
                            const BaseType_t xAutoReload,
                            void * const pvTimerID,
                            TimerCallbackFunction_t pxCallbackFunction )
{
    Timer_t * pxTimer;

    /* Allocate timer structure */
    pxTimer = ( Timer_t * ) pvPortMalloc( sizeof( Timer_t ) );
    if( pxTimer != NULL ) {
        pxTimer->pcTimerName = pcTimerName;
        pxTimer->xTimerPeriodInTicks = xTimerPeriodInTicks;
        pxTimer->pvTimerID = pvTimerID;
        pxTimer->pxCallbackFunction = pxCallbackFunction;
        pxTimer->ucStatus = tmrSTATUS_IS_ACTIVE;
        if( xAutoReload == pdTRUE ) {
            pxTimer->ucStatus |= tmrSTATUS_IS_AUTORELOAD;
        }
        vListInitialiseItem( &( pxTimer->xTimerListItem ) );
        listSET_LIST_ITEM_OWNER( &( pxTimer->xTimerListItem ), pxTimer );
    }

    return ( TimerHandle_t ) pxTimer;
}

/* xTimerCreateStatic - Static allocation */
TimerHandle_t xTimerCreateStatic( const char * const pcTimerName,
                                  const TickType_t xTimerPeriodInTicks,
                                  const BaseType_t xAutoReload,
                                  void * const pvTimerID,
                                  TimerCallbackFunction_t pxCallbackFunction,
                                  StaticTimer_t * pxTimerBuffer )
{
    Timer_t * pxTimer = ( Timer_t * ) pxTimerBuffer;

    if( pxTimerBuffer != NULL ) {
        pxTimer->pcTimerName = pcTimerName;
        pxTimer->xTimerPeriodInTicks = xTimerPeriodInTicks;
        pxTimer->pvTimerID = pvTimerID;
        pxTimer->pxCallbackFunction = pxCallbackFunction;
        pxTimer->ucStatus = tmrSTATUS_IS_ACTIVE | tmrSTATUS_IS_STATICALLY_ALLOCATED;
        if( xAutoReload == pdTRUE ) {
            pxTimer->ucStatus |= tmrSTATUS_IS_AUTORELOAD;
        }
        vListInitialiseItem( &( pxTimer->xTimerListItem ) );
        listSET_LIST_ITEM_OWNER( &( pxTimer->xTimerListItem ), pxTimer );
    }

    return ( TimerHandle_t ) pxTimer;
}

/* xTimerGenericCommand - Send command to daemon task */
BaseType_t xTimerGenericCommand( TimerHandle_t xTimer,
                                  BaseType_t xCommandID,
                                  TickType_t xOptionalValue,
                                  BaseType_t * pxHigherPriorityTaskWoken,
                                  TickType_t xTicksToWait )
{
    DaemonTaskMessage_t xMessage;
    BaseType_t xReturn;

    xMessage.xMessageID = xCommandID;
    xMessage.u.xTimerParameters.xMessageValue = xOptionalValue;
    xMessage.u.xTimerParameters.pxTimer = ( Timer_t * ) xTimer;

    xReturn = xQueueSend( xTimerQueue, &xMessage, xTicksToWait );

    return ( xReturn == pdPASS ) ? pdPASS : pdFAIL;
}

/* Timer Control Functions (macros) */
#define xTimerStart( xTimer, xTicksToWait ) \
    xTimerGenericCommand( ( xTimer ), tmrCOMMAND_START, ( TickType_t ) 0, NULL, ( xTicksToWait ) )

#define xTimerStop( xTimer, xTicksToWait ) \
    xTimerGenericCommand( ( xTimer ), tmrCOMMAND_STOP, ( TickType_t ) 0, NULL, ( xTicksToWait ) )

#define xTimerReset( xTimer, xTicksToWait ) \
    xTimerGenericCommand( ( xTimer ), tmrCOMMAND_RESET, ( TickType_t ) 0, NULL, ( xTicksToWait ) )

#define xTimerChangePeriod( xTimer, xNewPeriod, xTicksToWait ) \
    xTimerGenericCommand( ( xTimer ), tmrCOMMAND_CHANGE_PERIOD, ( xNewPeriod ), NULL, ( xTicksToWait ) )

#define xTimerDelete( xTimer, xTicksToWait ) \
    xTimerGenericCommand( ( xTimer ), tmrCOMMAND_DELETE, ( TickType_t ) 0, NULL, ( xTicksToWait ) )

/* ISR-safe versions */
#define xTimerStartFromISR( xTimer, pxHigherPriorityTaskWoken ) \
    xTimerGenericCommand( ( xTimer ), tmrCOMMAND_START_FROM_ISR, ( TickType_t ) 0, ( pxHigherPriorityTaskWoken ), 0 )

#define xTimerStopFromISR( xTimer, pxHigherPriorityTaskWoken ) \
    xTimerGenericCommand( ( xTimer ), tmrCOMMAND_STOP_FROM_ISR, ( TickType_t ) 0, ( pxHigherPriorityTaskWoken ), 0 )

#define xTimerResetFromISR( xTimer, pxHigherPriorityTaskWoken ) \
    xTimerGenericCommand( ( xTimer ), tmrCOMMAND_RESET_FROM_ISR, ( TickType_t ) 0, ( pxHigherPriorityTaskWoken ), 0 )

#define xTimerChangePeriodFromISR( xTimer, xNewPeriod, pxHigherPriorityTaskWoken ) \
    xTimerGenericCommand( ( xTimer ), tmrCOMMAND_CHANGE_PERIOD_FROM_ISR, ( xNewPeriod ), ( pxHigherPriorityTaskWoken ), 0 )

/* Query functions */
BaseType_t xTimerIsTimerActive( TimerHandle_t xTimer );
TickType_t xTimerGetPeriod( TimerHandle_t xTimer );
TickType_t xTimerGetExpiryTime( TimerHandle_t xTimer );
void * pvTimerGetTimerID( TimerHandle_t xTimer );
void vTimerSetTimerID( TimerHandle_t xTimer, void * pvNewID );
const char * pcTimerGetName( TimerHandle_t xTimer );
TaskHandle_t xTimerGetTimerDaemonTaskHandle( void );
