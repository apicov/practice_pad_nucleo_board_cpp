#include "Metronome.hpp"


Metronome::Metronome(TIM_HandleTypeDef* timer, int bpm)
:timer_(timer), tempo_(bpm), running_(false), last_tick_time_(0)
{
    // Set the timer to the desired BPM
    set_tempo(bpm);
}


void Metronome::start()
{
    if (!running_) {
        // Clear any pending interrupt flags
        __HAL_TIM_CLEAR_FLAG(&htim3, TIM_FLAG_UPDATE);
        // Reset the timer counter
        __HAL_TIM_SET_COUNTER(timer_, 0); 

        HAL_TIM_Base_Start_IT(timer_);
        running_ = true;
    }
}

void Metronome::stop()
{
    if (running_) {
        HAL_TIM_Base_Stop_IT(timer_);
        running_ = false;
    }
}

void Metronome::set_tempo(int bpm)
{
     // Clear any pending interrupt flags
    __HAL_TIM_CLEAR_FLAG(&htim3, TIM_FLAG_UPDATE);

    tempo_ = bpm;
    uint32_t timer_period = (600'000 / bpm) - 1; // Calculate the period in hundreds of microseconds
    __HAL_TIM_SET_AUTORELOAD(timer_, timer_period); // Set the timer period
    __HAL_TIM_SET_COUNTER(timer_, 0); // Reset the timer counter
    // set the timer prescaler to 8400
    // 8400 prescaler = 84 MHz / 8400 = 10 kHz
    // 10 kHz = 100 us
    __HAL_TIM_SET_PRESCALER(timer_, 8400 - 1); // Set the timer prescaler to 8400
    // calculate metronome's period in milliseconds
    period_ = (timer_period + 1) / 10;
    half_period_ = period_ / 2;
}

bool Metronome::is_running() const
{
    return running_;
}

int Metronome::get_tempo() const
{
    return tempo_;
}

void Metronome::register_tick_callback(tick_callback callback)
{
    tick_callback_ = callback;
}

void Metronome::exec_tick_callback()
{
    // execute the callback if it is set
    if (tick_callback_) {
        exec_tick_callback();
    }
}

void Metronome::tick()
{
    // store the current tick count
    last_tick_time_ = xTaskGetTickCount();
    ticks_++;
}

TickType_t Metronome::get_last_tick_time() const
{
    return last_tick_time_;
}

TickType_t Metronome::get_period() const
{
    return period_;
}

uint32_t Metronome::get_half_period() const
{
    return half_period_;
}