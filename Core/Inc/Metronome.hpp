#pragma once

#include <functional>

extern "C"
{
    #include "tim.h"
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
    private:
        TIM_HandleTypeDef* timer_;
        int tempo_; // in beats per minute
        bool running_;
        tick_callback tick_callback_;
        
};