extern "C"
{
    #include "main.h"
    #include "cpp_main.h"
    #include "cmsis_os.h"
    #include "adc.h"
    #include "FreeRTOS.h"
    #include "semphr.h"
}

#include<cstdio>

#define N_ADC1_CHANNELS 4
#define N_SAMPLES 10

int get_adc1_values(uint16_t *values);
void StartDefaultTask(void *argument);

xSemaphoreHandle xAdc1_dma_complete;

void cpp_main()
{
    printf("Hello from C++!\n");
    vSemaphoreCreateBinary( xAdc1_dma_complete );
    xTaskCreate(&StartDefaultTask, "Demo Task", 2048, NULL, 5, NULL);

    /* Start scheduler */
    osKernelStart();
    /* We should never get here as control is now taken by the scheduler */
    while (1);
}


void StartDefaultTask(void *pvParameters)
{
    UNUSED(pvParameters);
    // to store the ADC values
    uint16_t values[N_ADC1_CHANNELS];
    TickType_t xLastWakeTime;
    const TickType_t xFrequency = pdMS_TO_TICKS(20); // 

    // Initialize xLastWakeTime with the current tick count
    xLastWakeTime = xTaskGetTickCount();

    for(;;) {
        HAL_GPIO_TogglePin(LD2_GPIO_Port, LD2_Pin);
        get_adc1_values(values);
        printf("ADC1 values: %u %u %u %u\n", values[0], values[1], values[2], values[3]);
        printf("Tick count: %lu\n", xTaskGetTickCount());
        //vTaskDelay(pdMS_TO_TICKS(1000));
        vTaskDelayUntil(&xLastWakeTime, xFrequency);
    }
}

// Callback function for ADC conversion (DMA) complete
void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef* hadc)
{
	if (hadc->Instance == hadc1.Instance) {
			static portBASE_TYPE xHigherPriorityTaskWoken = pdFALSE;
            //give semaphore to signal that the conversion is complete
			xSemaphoreGiveFromISR( xAdc1_dma_complete, &xHigherPriorityTaskWoken );
		  /* If giving a semaphore caused a task to unblock, and the unblocked task
			has a priority equal to or higher than the currently running task (the task
			this ISR interrupted), then higher_priority_task_woken will have
			automatically been set to pdTRUE within the semaphore function.
			portEND_SWITCHING_ISR() will then ensure that this ISR returns directly to
			the higher priority unblocked task. */
		  portEND_SWITCHING_ISR(xHigherPriorityTaskWoken);
		}
}


static uint32_t adc1Buffer[N_ADC1_CHANNELS * N_SAMPLES];

int get_adc1_values(uint16_t *values)
{

	uint32_t temp_values[N_ADC1_CHANNELS] = {0};

	 //read  channels (0,1,4, 6) of 8 bits  10 times
    HAL_ADC_Start_DMA(&hadc1, adc1Buffer, N_SAMPLES * N_ADC1_CHANNELS);
    //wait for the conversion to complete
    if (xSemaphoreTake( xAdc1_dma_complete, 2000) == pdTRUE ) {//portMAX_DELAY
        //channel values are interleaved, lets sum values of each channel
        for (int i = 0 ; i < ( N_SAMPLES * N_ADC1_CHANNELS) ; i += N_ADC1_CHANNELS) {
            for (int j = 0 ; j < N_ADC1_CHANNELS ; j++) {
                temp_values[j] += adc1Buffer[i + j];
            }
        }

        //get  means
        for (int i = 0 ; i < N_ADC1_CHANNELS ; i++) {
            values[i] =  temp_values[i] / N_SAMPLES;
        }
    
        HAL_ADC_Stop_DMA(&hadc1);
    
        return 0;
    }

    HAL_ADC_Stop_DMA(&hadc1);
    return -1;

}