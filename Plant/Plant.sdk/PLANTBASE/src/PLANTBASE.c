/*
    1 tab == 4 spaces!
*/

/* FreeRTOS includes */
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
/* Digilent/Pmod includes */
#include "PmodHYGRO.h"
#include "PmodALS.h"
/* Xilinx includes. */
#include "sleep.h"
#include "xil_types.h"
#include "xil_printf.h"
#include "xil_cache.h"
#include "xgpio.h"
#include "xparameters.h"
/*-----------------------------------------------------------*/

// Hygro Polling Interval
#define HYGROPOLL 1000UL
// ALS Polling Interval
#define ALSPOLL 500UL

/* The Tx and Rx tasks as described at the top of this file. */
// hygro task
static void prvHygroPoll( void *pvParameters );
// ALS task
void prvALSPoll ( void *pvParameters );
// 7-segment task
static void prvSSWrite( void *pvParameters );
// periodic task
static void vPeriodicTask( void *pvParameters );
void EnableCaches();
void DisableCaches();
/*-----------------------------------------------------------*/

static void vPrintTaskStatus(TaskStatus_t TS);
void vApplicationIdleHook( void );

// Task Handles
static TaskHandle_t xHygroPoll;
static TaskHandle_t xSSWrite;
static TaskHandle_t xPeriodicTask;
static TaskHandle_t xALSPoll;
// pmod structs
static PmodHYGRO xHYGRO;
static PmodALS xALS;

// Queues
static QueueHandle_t xQueue1;



int main( void )
{
	/* define queues */
	// PmodHYGRO
	xQueue1 =	xQueueCreate(10,	sizeof(int));


	/* define tasks */
	xTaskCreate( 	prvSSWrite,
					( const char * ) "SSEG",
					(unsigned short) 255,
					NULL,
					tskIDLE_PRIORITY+1,
					&xSSWrite );/**/

	xTaskCreate( 	prvHygroPoll,
					( const char * ) "HYGRO",
					(unsigned short) 255,
					NULL,
					tskIDLE_PRIORITY+1,
					&xHygroPoll ); /**/

	xTaskCreate( 	prvALSPoll,
					( const char * ) "ALS",
					(unsigned short) 255,
					NULL,
					tskIDLE_PRIORITY+1,
					&xALSPoll );/**/

	xTaskCreate( 	vPeriodicTask,
						( const char * ) "PTASK",
						configMINIMAL_STACK_SIZE,
						NULL,
						tskIDLE_PRIORITY+1,
						&xPeriodicTask );/**/


	vTaskStartScheduler();

	xil_printf("MAIN SETUP DONE");
	for( ;; );
}


/*-----------------------------------------------------------*/
void prvALSPoll(	void *pvParameters	)
{
	// init. ALS
	EnableCaches();
	ALS_begin(&xALS, XPAR_ALS_AXI_LITE_SPI_BASEADDR);
	// run ALS
	u8 light = 0;
	TickType_t xLastWakeTime;
	xLastWakeTime = xTaskGetTickCount();

	for(	;;	)
	{
		vTaskDelayUntil(	&xLastWakeTime, pdMS_TO_TICKS(1000U));
		EnableCaches();
		light = ALS_read(&xALS);
		// cleanup
		DisableCaches();

		// TODO: queue operations

		xil_printf("light: %d", light);
	}
	xil_printf("ALS ERROR\r\n");

}

/*-----------------------------------------------------------*/
void prvHygroPoll( void *pvParameters )
{
	int TempTrunc = 0, RHTrunc =0 ;
	//hygro setup

	EnableCaches();
	HYGRO_begin(&xHYGRO, XPAR_HYGRO_AXI_LITE_IIC_BASEADDR, 0x40,  XPAR_HYGRO_AXI_LITE_TMR_BASEADDR, XPAR_HYGRO_DEVICE_ID, XPAR_CPU_M_AXI_DP_FREQ_HZ ); //setup of hygro with proper addresses etc
	DisableCaches();
	int Q1stat,Q2stat;
	float temp, hum;

	TickType_t xLastWakeTime;
	xLastWakeTime = xTaskGetTickCount();

	for( ;; )
	{
		vTaskDelayUntil( &xLastWakeTime , pdMS_TO_TICKS( 1000U ));
		EnableCaches();
		temp = HYGRO_tempC2F(HYGRO_getTemperature(&xHYGRO));
		TempTrunc = temp;
	    hum = HYGRO_getHumidity(&xHYGRO);
	    RHTrunc = hum;
	    DisableCaches();


	    Q1stat = xQueueSend(xQueue1, &TempTrunc, 1);

	    Q2stat = xQueueSend(xQueue1, &RHTrunc, 1);
	    xil_printf("Temp: %d", TempTrunc);
	    xil_printf("Hum: %d", RHTrunc);
	    xil_printf("\n", RHTrunc);


	}
	xil_printf("HY ERROR\r\n");


	DisableCaches();
}/**/
void prvSSWrite( void *pvParameters )
{
	//gpio init
	XGpio GP;
	int Status;

	Status = XGpio_Initialize(&GP,XPAR_GPIO_0_DEVICE_ID); //intialize GP1 with device id 0, from xparameters file
	if (Status != XST_SUCCESS) {
			xil_printf("Gpio Initialization Failed\r\n");
			//return XST_FAILURE;
		}

	XGpio_SetDataDirection(&GP, 1, 0);
	XGpio_SetDataDirection(&GP, 2, 0);
	//gpio init end

	int recVal,Qstatus; //var for reception, status observation

	int LowNum, UpNum;
	LowNum = 0;
	UpNum = 0;
	u32 send;

	TickType_t xLastWakeTime = xTaskGetTickCount();

	for( ;; )
	{
		vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS( 1000U ));
		Qstatus = xQueueReceive(xQueue1,&recVal,1);
		//get temp and hum from hygro task
		if(Qstatus != errQUEUE_EMPTY)
		{
			LowNum= recVal;
		}
		Qstatus = xQueueReceive(xQueue1,&recVal,1);

		if(Qstatus != errQUEUE_EMPTY)
		{
			UpNum = recVal;
		}

		send = ((LowNum)|(UpNum<<16));
		XGpio_DiscreteWrite(&GP, 1, send);
		send = UpNum>>4;
		XGpio_DiscreteWrite(&GP, 2, send);
	}
	xil_printf("SS ERROR\r\n");
}/**/

void vPeriodicTask( void *pvParameters )
{
TickType_t xLastWakeTime;
const TickType_t xDelay3ms= pdMS_TO_TICKS(10000);

xLastWakeTime = xTaskGetTickCount();
	for( ;; )
	{
		xil_printf( "Periodic task is running\r\n" );
		vTaskDelayUntil( &xLastWakeTime,xDelay3ms);
	}
}

void vApplicationIdleHook( void )
{
	TickType_t ticks = xTaskGetTickCount();
	if( ticks%100 == 0 )
	{
		TaskStatus_t *pxTaskStatusArray;
		volatile UBaseType_t uxArraySize, x;
		unsigned long ulTotalRunTime;
		uxArraySize = uxTaskGetNumberOfTasks(); //get # of running tasks
		pxTaskStatusArray = pvPortMalloc( uxArraySize * sizeof( TaskStatus_t ) ); //make array of status structs for that many

		uxArraySize = uxTaskGetSystemState( pxTaskStatusArray, uxArraySize, &ulTotalRunTime );

		if( pxTaskStatusArray != NULL )
		{
			 for( x = 0; x < uxArraySize; x++ )
			 {
				 //vPrintTaskStatus(pxTaskStatusArray[x]);
			 }
		}
		vPortFree(pxTaskStatusArray);
	}
}/**/


static void vPrintTaskStatus(TaskStatus_t TS)
{
	switch(TS.eCurrentState)
	{
		case eReady : {
			xil_printf( TS.pcTaskName );
			xil_printf( " is Ready\n" );
			break;
		}
		case eRunning : {
			xil_printf( TS.pcTaskName );
			xil_printf( " is Running\n" );
			break;
		}
		case eBlocked : {
			xil_printf( TS.pcTaskName );
			xil_printf( " is Blocked\n" );
			break;
		}
		case eSuspended : {
			xil_printf( TS.pcTaskName );
			xil_printf( " is Suspended\n" );
			break;
		}
		case eDeleted : {
			xil_printf( TS.pcTaskName );
			xil_printf( " is Deleted\n" );
			break;
		}
		default : {
			xil_printf( "StatusPrintError\n" );
			break;
		}
	}
}

void EnableCaches() {
#ifdef __MICROBLAZE__
#ifdef XPAR_MICROBLAZE_USE_ICACHE
   Xil_ICacheEnable();
#endif
#ifdef XPAR_MICROBLAZE_USE_DCACHE
   Xil_DCacheEnable();
#endif
#endif
}

void DisableCaches() {
#ifdef __MICROBLAZE__
#ifdef XPAR_MICROBLAZE_USE_ICACHE
   Xil_ICacheDisable();
#endif
#ifdef XPAR_MICROBLAZE_USE_DCACHE
   Xil_DCacheDisable();
#endif
#endif
}
