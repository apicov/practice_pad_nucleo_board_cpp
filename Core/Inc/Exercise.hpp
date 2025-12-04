#pragma once

#include <cstdint>

/**
 * @brief Exercise definition for Stick Control practice system
 *
 * Each exercise represents a specific sticking pattern from the book
 * "Stick Control for the Snare Drummer" by George Lawrence Stone.
 */
struct Exercise {
    uint16_t id;                      // Unique exercise ID (1-400+)
    uint8_t level;                    // Level 1-8 (1=Single Beats, 2=Triplets, etc.)
    uint8_t section;                  // Section within level (1=A, 2=B, 3=C, 4=D)
    char pattern[24];                 // Sticking pattern, e.g., "RLRL RLRL RLRL RLRL"
    char name[40];                    // Exercise name, e.g., "Basic Alternating"
    uint8_t difficulty;               // Difficulty rating 1-5 (stars)
    uint8_t beats_per_pattern;        // Number of beats in one pattern (usually 4)
    uint8_t default_tempo_start;      // Starting tempo in BPM (e.g., 60)
    uint8_t default_tempo_end;        // Ending tempo in BPM (e.g., 120)
    uint8_t tempo_step;               // Tempo increment in BPM (e.g., 20)
};

/**
 * @brief Tempo milestone tracking for an exercise
 *
 * Each exercise must be mastered at multiple tempos before
 * the next exercise is unlocked (Stone's method).
 */
struct TempoMilestone {
    uint16_t exercise_id;             // Which exercise
    uint8_t tempo;                    // Specific tempo (60, 80, 100, 120, etc.)
    bool completed;                   // Has this tempo been completed?
    uint32_t best_timing_ms;          // Best average timing accuracy (±ms)
    uint32_t completion_timestamp;    // When completed (FreeRTOS tick count)
};
