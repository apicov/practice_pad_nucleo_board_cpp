#pragma once

#include "Exercise.hpp"
#include "PracticeConfig.hpp"
#include <cstdint>
#include <cstring>

extern "C" {
    #include "cmsis_os.h"
}

/**
 * @brief Session state machine states
 */
enum SessionState {
    SESSION_IDLE,           // No exercise loaded
    SESSION_READY,          // Exercise loaded, not started
    SESSION_ACTIVE,         // Currently practicing
    SESSION_PAUSED,         // Paused mid-session
    SESSION_SUCCESS,        // Completed 20/20 reps!
    SESSION_FAILED          // Failed a rep (but will reset and continue)
};

/**
 * @brief Result of a single repetition
 */
struct RepetitionResult {
    uint8_t rep_number;             // Which rep (1-20)
    uint8_t strikes_detected;       // How many strikes were detected
    uint8_t strikes_expected;       // How many strikes should be present
    uint8_t strikes_good;           // Strikes within "good" window
    uint8_t strikes_acceptable;     // Strikes within "acceptable" window
    int32_t avg_timing_error_ms;    // Average timing error (±ms)
    int32_t max_timing_error_ms;    // Worst strike timing
    bool success;                   // Did this rep pass?
    uint32_t duration_ms;           // Time to complete this rep
};

/**
 * @brief SessionManager - Implements the 20-repetition Stone method
 *
 * This class is the core of the practice system. It:
 * - Loads exercises and tracks tempo
 * - Counts strikes per repetition
 * - Validates timing accuracy
 * - Enforces the "20 perfect reps or reset to 0" rule
 * - Tracks session statistics
 */
class SessionManager {
public:
    /**
     * @brief Constructor
     */
    SessionManager();

    /**
     * @brief Load an exercise at a specific tempo
     * @param exercise_id Exercise ID from library
     * @param tempo Tempo in BPM
     * @return true if exercise loaded successfully
     */
    bool load_exercise(uint16_t exercise_id, uint8_t tempo);

    /**
     * @brief Get current exercise
     * @return Pointer to current exercise, or nullptr
     */
    const Exercise* get_current_exercise() const { return current_exercise_; }

    /**
     * @brief Get current tempo
     * @return Tempo in BPM
     */
    uint8_t get_current_tempo() const { return current_tempo_; }

    /**
     * @brief Start the practice session
     */
    void start_session();

    /**
     * @brief Pause the session
     */
    void pause_session();

    /**
     * @brief Resume the session
     */
    void resume_session();

    /**
     * @brief Reset session (start over from rep 0)
     */
    void reset_session();

    /**
     * @brief Abort session and return to idle
     */
    void abort_session();

    /**
     * @brief Process a detected strike
     * @param timing_offset_ms Timing offset from metronome beat (±ms)
     */
    void process_strike(int32_t timing_offset_ms);

    /**
     * @brief Called on each metronome beat
     * Used to detect pattern completion and missed beats
     */
    void process_beat();

    /**
     * @brief Get current state
     * @return Current session state
     */
    SessionState get_state() const { return state_; }

    /**
     * @brief Get current repetition number (0-19, displays as 1-20)
     * @return Current rep (0-based)
     */
    uint8_t get_current_rep() const { return current_rep_; }

    /**
     * @brief Get total reps required (always 20 for Stone method)
     * @return 20
     */
    uint8_t get_total_reps() const { return config_.required_reps; }

    /**
     * @brief Get last repetition result
     * @return Last rep result
     */
    const RepetitionResult& get_last_rep_result() const { return last_rep_result_; }

    /**
     * @brief Get session duration in milliseconds
     * @return Duration since session started
     */
    uint32_t get_session_duration_ms() const;

    /**
     * @brief Get number of attempts (how many times reset to 0)
     * @return Attempt count
     */
    uint8_t get_attempt_count() const { return attempt_count_; }

    /**
     * @brief Get practice configuration
     * @return Current config
     */
    const PracticeConfig& get_config() const { return config_; }

    /**
     * @brief Set practice configuration
     * @param config New configuration
     */
    void set_config(const PracticeConfig& config) { config_ = config; }

    /**
     * @brief Get beats processed in current rep
     * @return Beat count
     */
    uint8_t get_beats_in_current_rep() const { return beats_in_current_rep_; }

private:
    // Exercise info
    const Exercise* current_exercise_;
    uint8_t current_tempo_;

    // State
    SessionState state_;
    PracticeConfig config_;

    // Repetition tracking
    uint8_t current_rep_;                   // 0-19 (20 total)
    uint8_t strikes_in_current_rep_;        // Strikes detected in current rep
    uint8_t beats_in_current_rep_;          // Metronome beats in current rep
    uint8_t expected_strikes_per_rep_;      // From exercise definition
    uint8_t expected_beats_per_rep_;        // Usually 4 (one pattern)

    // Timing accuracy tracking
    int32_t timing_error_sum_;              // Sum for averaging
    int32_t timing_error_max_;              // Worst strike
    uint8_t strikes_good_;                  // Within "good" window
    uint8_t strikes_acceptable_;            // Within "acceptable" window

    // Session stats
    uint32_t session_start_time_;
    uint32_t rep_start_time_;
    uint8_t attempt_count_;
    RepetitionResult last_rep_result_;

    // Internal methods
    void finalize_repetition();
    void fail_session();
    void complete_session();
    void reset_rep_counters();
};
