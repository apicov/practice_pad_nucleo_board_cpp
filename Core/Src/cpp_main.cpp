extern "C"
{
    #include "main.h"
    #include "cpp_main.h"
    #include "cmsis_os.h"
    #include "adc.h"
    #include "FreeRTOS.h"
    #include "semphr.h"
    #include "dma.h"
    #include "usart.h"
    #include "gpio.h"
    #include "string.h"
}

#include<cstdio>

#define N_ADC1_CHANNELS 10
#define N_SAMPLES 5

int get_adc1_values(uint16_t *values);
void adc1_task(void *argument);
void uart_send_values_task(void *argument);

SemaphoreHandle_t xadc1_dma_complete = xSemaphoreCreateBinary();

int uart_write(UART_HandleTypeDef *huart,uint8_t *pData, uint16_t Size, long unsigned int timeout);
SemaphoreHandle_t xusart_tx_complete = xSemaphoreCreateBinary();


void cpp_main()
{
    printf("Hello from C++!\n");
    xTaskCreate(&adc1_task, "adc task", 2048, NULL, 5, NULL);
    //xTaskCreate(&uart_send_values_task, "send v task", 2048, NULL, 5, NULL);

    /* Start scheduler */
    osKernelStart();
    /* We should never get here as control is now taken by the scheduler */
    while (1);
}

void adc1_task(void *pvParameters)
{
    UNUSED(pvParameters);
    // To store the ADC values
    uint16_t values[N_ADC1_CHANNELS];
    TickType_t xLastWakeTime;
    const TickType_t xFrequency = pdMS_TO_TICKS(100); //
    char buffer[100];

    // Initialize xLastWakeTime with the current tick count
    xLastWakeTime = xTaskGetTickCount();

    for(;;) {
        HAL_GPIO_TogglePin(LD2_GPIO_Port, LD2_Pin);
        if (get_adc1_values(values) == -1) {
            printf("Error reading ADC1 values\n");
            continue;
        }
        sprintf(buffer, "%lu:%u %u %u %u %u %u %u %u %u %u\n", xTaskGetTickCount(), values[0],
            values[1], values[2], values[3], values[4], values[5], values[6], values[7],
            values[8], values[9]);

        printf("%s", buffer);
        if (uart_write(&huart2, (uint8_t *)buffer, strlen(buffer), 2000)) {
            printf("Error sending data through UART\n");
        }
        //printf("ADC1 values: %u %u %u %u\n", values[0], values[1], values[2], values[3]);
        //printf("Tick count: %lu\n", xTaskGetTickCount());
        //vTaskDelay(pdMS_TO_TICKS(1000));
        vTaskDelayUntil(&xLastWakeTime, xFrequency);
    }
}

// Callback function for ADC conversion (DMA) complete
void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef* hadc)
{
	if (hadc->Instance == hadc1.Instance) {
			static portBASE_TYPE xHigherPriorityTaskWoken = pdFALSE;
            //Give semaphore to signal that the conversion is complete
			xSemaphoreGiveFromISR(xadc1_dma_complete, &xHigherPriorityTaskWoken);
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

	 // Read  channels (0,1,4,6,7,8,9,10,11,12) of 8 bits  10 times
    HAL_ADC_Start_DMA(&hadc1, adc1Buffer, N_SAMPLES * N_ADC1_CHANNELS);
    // Wait for the conversion to complete
    if (xSemaphoreTake( xadc1_dma_complete, 2000) == pdTRUE ) {//portMAX_DELAY
        // Channel values are interleaved, lets sum values of each channel
        for (int i = 0 ; i < ( N_SAMPLES * N_ADC1_CHANNELS) ; i += N_ADC1_CHANNELS) {
            for (int j = 0 ; j < N_ADC1_CHANNELS ; j++) {
                temp_values[j] += adc1Buffer[i + j];
            }
        }

        // Get  means
        for (int i = 0 ; i < N_ADC1_CHANNELS ; i++) {
            values[i] =  temp_values[i] / N_SAMPLES;
        }
    
        HAL_ADC_Stop_DMA(&hadc1);
    
        return 0;
    }

    HAL_ADC_Stop_DMA(&hadc1);
    return -1;

}

// Write data to UART with DMA
int uart_write(UART_HandleTypeDef *huart,uint8_t *pData, uint16_t Size, long unsigned int timeout)
{
	HAL_UART_Transmit_DMA(huart,pData, Size);

	if (xSemaphoreTake( xusart_tx_complete, timeout) == pdTRUE ) {// portMAX_DELAY
	    HAL_GPIO_TogglePin(LD2_GPIO_Port, LD2_Pin);
        HAL_UART_DMAStop(huart);
		return 0;
	}
	else {
		HAL_UART_AbortTransmit(huart);
        HAL_UART_DMAStop(huart);
		return -1;
	}
	// TODO: hacer algun if para checar timeout y cancelar la espera del caracter si hay timeout(reiniciar el receiver dma)

}

// ISR callback for UART transmission complete with DMA
void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart)
{
	static portBASE_TYPE xHigherPriorityTaskWoken = pdFALSE;
    if (huart->Instance == huart2.Instance) {
        xSemaphoreGiveFromISR(xusart_tx_complete, &xHigherPriorityTaskWoken);
        portEND_SWITCHING_ISR(xHigherPriorityTaskWoken);
    }
}