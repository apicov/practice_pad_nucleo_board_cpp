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
    #include "tim.h"
}

#include<cstdio>
#include "CircularBuffer.hpp"
#include "Metronome.hpp"

#define N_ADC1_CHANNELS 5
#define N_SAMPLES 5
#define N_PEAK_CHANNELS 5

typedef struct {
    int16_t values[N_ADC1_CHANNELS];
    TickType_t timestamp;
} adc1_data_t;

typedef struct {
    adc1_data_t sensor;
    TickType_t metronome_tick_timestamp;
} strike_t;

bool is_strike(adc1_data_t *values);
int get_adc1_values(int16_t *values);
void sampling_task(void *pvParameters);
void uart_send_values_task(void *pvParameters);
void strike_task(void *pvParameters);
void metronome_tick_task(void *pvParameters);


SemaphoreHandle_t xadc1_dma_complete = xSemaphoreCreateBinary();

int uart_write(UART_HandleTypeDef *huart,uint8_t *pData, uint16_t Size, long unsigned int timeout);
SemaphoreHandle_t xusart_tx_complete = xSemaphoreCreateBinary();

CircularBuffer<adc1_data_t> adc1_buffer(50);
CircularBuffer<strike_t> strikes_buffer(50);

SemaphoreHandle_t xc_buffer_mutex;

Metronome *metronome;
SemaphoreHandle_t xmetronome_timer_period_elapsed = xSemaphoreCreateBinary();

QueueHandle_t strikes_queue;

void cpp_main()
{
    printf("Hello from C++!\n");
    
    xc_buffer_mutex = xSemaphoreCreateMutex();
    if (xc_buffer_mutex == NULL) {
        // Handle error
        printf("Error creating mutex\n");
    }
    strikes_queue = xQueueCreate(5, sizeof(strike_t));
    if (strikes_queue == NULL) {
        // Handle error
        printf("Error creating queue\n");
    }
    metronome = new Metronome(&htim3, 60);
    metronome->start();
    // Create tasks
    if(xTaskCreate(&sampling_task, "sampling task", 1024, NULL, 10, NULL) != pdPASS) {
        printf("Error creating sampling task\n");
    }
    //if(xTaskCreate(&uart_send_values_task, "send v task", 1024, NULL, 6, NULL) != pdPASS) {
    //    printf("Error creating uart send values task\n");
    //}
    if(xTaskCreate(&strike_task, "strike task", 1024, NULL, 7, NULL) != pdPASS) {
        printf("Error creating strike detected task\n");
    }
    if(xTaskCreate(&metronome_tick_task, "metronome task", 1024, NULL, 6, NULL) != pdPASS) {
        printf("Error creating metronome task\n");
    }

    /* Start scheduler */
    osKernelStart();
    /* We should never get here as control is now taken by the scheduler */
    while (1);
}

void sampling_task(void *pvParameters)
{
    UNUSED(pvParameters);
    // To store the ADC values
    //uint16_t values[N_ADC1_CHANNELS];
    adc1_data_t raw_data;
    strike_t strike_data;
    TickType_t xLastWakeTime;
    const TickType_t xFrequency = pdMS_TO_TICKS(10);

    // Initialize xLastWakeTime with the current tick count
    xLastWakeTime = xTaskGetTickCount();
    for(;;) {        
        // Start the ADC conversion
        if (get_adc1_values(raw_data.values) == -1) {
            printf("Error reading ADC1 values\n");
            continue;
        }

        // send data to circular buffer
        raw_data.timestamp = xTaskGetTickCount();
        xSemaphoreTake(xc_buffer_mutex, portMAX_DELAY);
            adc1_buffer.push(&raw_data, 1);
            //peaks_task_buffer.push(&raw_data, 1);
        xSemaphoreGive(xc_buffer_mutex);
        
        // Detect drum strike
        if (is_strike(&raw_data)) {
            strike_data.sensor = raw_data;
            strike_data.metronome_tick_timestamp = metronome->get_last_tick_time();
            // send strike and metronome tick timestamp to queue
            xQueueSend(strikes_queue, &strike_data, portMAX_DELAY);
        }

        //printf("%s", buffer);
        //last_time = __HAL_TIM_GET_COUNTER(&htim1);
        //printf("%lu\n",__HAL_TIM_GET_COUNTER(&htim1) - last_time);
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

int get_adc1_values(int16_t *values)
{

	int32_t temp_values[N_ADC1_CHANNELS] = {0};

	 // Read  channels (0,1,8,10,11) of 8 bits  5 times
    HAL_ADC_Start_DMA(&hadc1, adc1Buffer, N_SAMPLES * N_ADC1_CHANNELS);

    // Wait for the conversion to complete
    if (xSemaphoreTake( xadc1_dma_complete, 2000) == pdTRUE ) {//portMAX_DELAY

        // Channel values are interleaved, sum values of each channel
        for (int i = 0 ; i < ( N_SAMPLES * N_ADC1_CHANNELS) ; i += N_ADC1_CHANNELS) {
            for (int j = 0 ; j < N_ADC1_CHANNELS ; j++) {
                temp_values[j] += adc1Buffer[i + j];
            }
        }

        // Calculate the average for each channel
        // Divide the sum of each channel by the number of samples
        for (int i = 0 ; i < N_ADC1_CHANNELS ; i++) {
            values[i] = (int16_t) (temp_values[i] / N_SAMPLES);
        }

        //normalize according to calibration coefficients
        values[0] = (int16_t) ((values[0] * 0.81787) - 96.6632 ); //top
        values[1] = (int16_t) ((values[1] * 0.76373) - 91.18395 ); //bottom
        values[2] = (int16_t) ((values[2] * 1.04608) - 123.94094 ); //left
        values[3] = (int16_t) ((values[3] * 0.944475) - 113.7599 ); //right
        values[4] = (int16_t) ((values[4] * -0.18813) + 20.4944 ); //force
    
        HAL_ADC_Stop_DMA(&hadc1);
    
        return 0;
    }

    HAL_ADC_Stop_DMA(&hadc1);
    return -1;

}

void uart_send_values_task(void *pvParameters)
{
        UNUSED(pvParameters);
        char buffer[100];        
        adc1_data_t adc1_values;
        size_t n_values;

        for(;;) {

            xSemaphoreTake(xc_buffer_mutex, portMAX_DELAY);
                n_values = adc1_buffer.element_count();
            xSemaphoreGive(xc_buffer_mutex);

            if (n_values > 0) {
                xSemaphoreTake(xc_buffer_mutex, portMAX_DELAY);
                    adc1_buffer.pop(&adc1_values, 1);
                xSemaphoreGive(xc_buffer_mutex);

                sprintf(buffer, "%lu:%d %d %d %d %d\n", adc1_values.timestamp, adc1_values.values[0],
                    adc1_values.values[1], adc1_values.values[2], adc1_values.values[3], adc1_values.values[4]);

                

                //sprintf(buffer, "%lu:%u %u %u %u\n", adc1_values.timestamp, adc1_values.values[0],
                //    adc1_values.values[1], adc1_values.values[2], adc1_values.values[3]);
                
                if (uart_write(&huart2, (uint8_t *)buffer, strlen(buffer), 2000)) {
                    printf("Error sending data through UART\n");
                }
                //printf("%s", buffer);
                //printf("a%lu\n",adc1_buffer.element_count());
            }
            else {
                vTaskDelay(pdMS_TO_TICKS(10));
            }
        }
}



// Write data to UART with DMA
int uart_write(UART_HandleTypeDef *huart,uint8_t *pData, uint16_t Size, long unsigned int timeout)
{
	HAL_UART_Transmit_DMA(huart,pData, Size);

	if (xSemaphoreTake( xusart_tx_complete, timeout) == pdTRUE ) {// portMAX_DELAY
	    //HAL_GPIO_TogglePin(LD2_GPIO_Port, LD2_Pin);
        //HAL_UART_DMAStop(huart);
		return 0;
	}
	else {
		HAL_UART_AbortTransmit(huart);
        //HAL_UART_DMAStop(huart);
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

bool is_strike(adc1_data_t *values)
{
    static int32_t prev_sum = 0;
    static TickType_t last_strike_time = 0;

    // Calculate the average of the values
    int sum = 0;
    // TODO: remove the extra unused channel
    for (int i = 0; i < N_ADC1_CHANNELS - 1; i++) {
        sum += values->values[i];
    }
    sum = sum >> 2;//sum /= 4;
    // Check if the sum is below a threshold
    if (sum < 12) {
        sum = 0;
    }
    // Check if the difference between the current and previous values is above a threshold
    if ( sum > prev_sum && (sum - prev_sum) > 10 && (values->timestamp - last_strike_time) > 60) {
        // Peak detected
        last_strike_time = values->timestamp;
        prev_sum = sum;
        return true;
    }

    prev_sum = sum;
    return false;
}

void strike_task(void *pvParameters)
{
    UNUSED(pvParameters);
    char buffer[100];        
    strike_t strike_data;
    TickType_t time_diff; // difference between the strike and metronome tick timestamps

    for(;;) {
        // wait for a strike to be detected (element in queue)
        if (xQueueReceive(strikes_queue, &strike_data, portMAX_DELAY) == pdTRUE) {
            // strike was before the metronome tick            
            if (strike_data.sensor.timestamp > 
                (strike_data.metronome_tick_timestamp + metronome->get_half_period())) {

                time_diff = strike_data.metronome_tick_timestamp + metronome->get_period()
                             - strike_data.sensor.timestamp;

                printf("Strike. Time diff: +%lu\n", time_diff);
            }
            // strike was after the metronome tick
            else {
                time_diff = strike_data.sensor.timestamp - strike_data.metronome_tick_timestamp;
                printf("Strike. Time diff: -%lu\n", time_diff);

            }
            
        }

        
    /*
                printf("Peak detected: %d %lu %u\n", sum, adc1_values.timestamp, peaks_count);
                last_peak_time = adc1_values.timestamp;

                sprintf(buffer, "%lu:%d %d %d %d %d\n", adc1_values.timestamp, adc1_values.values[0],
                    adc1_values.values[1], adc1_values.values[2], adc1_values.values[3], adc1_values.values[4]);
                if (uart_write(&huart2, (uint8_t *)buffer, strlen(buffer), 2000)) {
                    printf("Error sending data through UART\n");
                }
            }
            else {
                sprintf(buffer, "%lu:%d %d %d %d %d\n", adc1_values.timestamp, 0, 0, 0, 0, 0);
                if (uart_write(&huart2, (uint8_t *)buffer, strlen(buffer), 2000)) {
                    printf("Error sending data through UART\n");
                }
            }

            prev = sum;
            
            
            //printf("%s", buffer);
            //printf("p%lu\n",peaks_task_buffer.element_count());*/
        else {
            vTaskDelay(pdMS_TO_TICKS(10));
        }
    }
}


void metronome_tick_task(void *pvParameters)
{
    UNUSED(pvParameters);

    while (1) {
        xSemaphoreTake(xmetronome_timer_period_elapsed, portMAX_DELAY);
        metronome->tick();
        
        //vTaskDelay(pdMS_TO_TICKS(1000));
        //printf("Metronome tick\n");

        //blink metronome's led
        HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin, GPIO_PIN_SET);
        vTaskDelay(pdMS_TO_TICKS(50));
        HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin, GPIO_PIN_RESET);
    }

}