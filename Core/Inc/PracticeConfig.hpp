#pragma once

#include <cstdint>

/**
 * @brief Practice configuration parameters
 *
 * This structure contains all configurable parameters for the practice system,
 * allowing users to adjust difficulty and tolerance levels.
 */
struct PracticeConfig {
    // Timing tolerance (in milliseconds)
    int32_t timing_window_perfect;    // ±ms for "perfect" strike (default: 20ms)
    int32_t timing_window_good;       // ±ms for "good" strike (default: 40ms)
    int32_t timing_window_acceptable; // ±ms for "acceptable" strike (default: 60ms)
    int32_t timing_window_max;        // ±ms maximum - beyond this = failure (default: 100ms)

    // Repetition requirements
    uint8_t required_reps;            // Number of reps to complete (default: 20)
    uint8_t max_failures_per_rep;     // Max strikes that can fail before rep fails (default: 0)

    // Success criteria
    uint8_t min_accuracy_percent;     // Minimum % of strikes that must be acceptable (default: 95)

    // Missed beat detection
    bool detect_missed_beats;         // Enable missed beat detection (default: true)

    // Feedback settings
    bool real_time_feedback;          // Send strike events in real-time (default: true)
    bool verbose_logging;             // Enable detailed console logging (default: true)

    // Default constructor with sensible defaults
    PracticeConfig() :
        timing_window_perfect(20),
        timing_window_good(40),
        timing_window_acceptable(60),
        timing_window_max(100),
        required_reps(20),
        max_failures_per_rep(0),
        min_accuracy_percent(95),
        detect_missed_beats(true),
        real_time_feedback(true),
        verbose_logging(true)
    {}
};

/**
 * @brief Difficulty presets for quick configuration
 */
namespace DifficultyPresets {

    // Beginner: More lenient timing, good for learning
    inline PracticeConfig beginner() {
        PracticeConfig cfg;
        cfg.timing_window_perfect = 30;
        cfg.timing_window_good = 60;
        cfg.timing_window_acceptable = 100;
        cfg.timing_window_max = 150;
        cfg.max_failures_per_rep = 1;  // Allow 1 bad strike per rep
        cfg.min_accuracy_percent = 85;
        return cfg;
    }

    // Standard: Default Stone method - strict but fair
    inline PracticeConfig standard() {
        PracticeConfig cfg;  // Uses defaults
        return cfg;
    }

    // Advanced: Tighter timing for experienced players
    inline PracticeConfig advanced() {
        PracticeConfig cfg;
        cfg.timing_window_perfect = 15;
        cfg.timing_window_good = 30;
        cfg.timing_window_acceptable = 45;
        cfg.timing_window_max = 60;
        cfg.max_failures_per_rep = 0;
        cfg.min_accuracy_percent = 98;
        return cfg;
    }

    // Master: Professional level - extremely tight
    inline PracticeConfig master() {
        PracticeConfig cfg;
        cfg.timing_window_perfect = 10;
        cfg.timing_window_good = 20;
        cfg.timing_window_acceptable = 30;
        cfg.timing_window_max = 40;
        cfg.max_failures_per_rep = 0;
        cfg.min_accuracy_percent = 100;
        return cfg;
    }
}
