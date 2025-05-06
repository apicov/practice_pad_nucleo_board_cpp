#include "Metronome.hpp"

Metronome::Metronome(TIM_HandleTypeDef* timer, int bpm)
:timer_(timer), tempo_(bpm), running_(false)
{
    set_tempo(bpm);
}

void Metronome::start()
{
    if (!running_) {
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
    tempo_ = bpm;
    uint32_t timer_period = (600'000 / bpm) - 1; // Calculate the period in hundreds of microseconds
    __HAL_TIM_SET_AUTORELOAD(timer_, timer_period); // Set the timer period
    __HAL_TIM_SET_COUNTER(timer_, 0); // Reset the timer counter
    // set the timer prescaler to 8400
    // 8400 prescaler = 84 MHz / 8400 = 10 kHz
    // 10 kHz = 100 us
    __HAL_TIM_SET_PRESCALER(timer_, 0); // Set the timer prescaler to 8400
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