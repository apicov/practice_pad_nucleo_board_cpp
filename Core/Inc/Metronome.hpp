#pragma once

#include <functional>
#include<cstdio>

extern "C"
{
    #include "tim.h"
    #include "cmsis_os.h"
}

class Metronome 
{
    public:
        using tick_callback = std::function<void(void)>;
        Metronome(TIM_HandleTypeDef* timer, int bpm = 60);
        //~Metronome();
        void start();
        void stop();
        void set_tempo(int bpm);
        int get_tempo() const;
        bool is_running() const;
        void register_tick_callback(tick_callback callback);
        void exec_tick_callback();
        void tick();
        TickType_t get_last_tick_time() const;
        TickType_t get_period() const;
        uint32_t get_half_period() const;

    private:
        TIM_HandleTypeDef* timer_;
        int tempo_; // in beats per minute
        bool running_;
        tick_callback tick_callback_;
        uint32_t period_; // in milliseconds
        uint32_t half_period_; // in milliseconds
        TickType_t last_tick_time_;
        uint32_t ticks_ = 0;// ticks since start of the metronome
};