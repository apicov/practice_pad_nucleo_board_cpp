#include "SessionManager.hpp"
#include "ExerciseLibrary.hpp"
#include <cstdio>
#include <cmath>

SessionManager::SessionManager()
    : current_exercise_(nullptr)
    , current_tempo_(60)
    , state_(SESSION_IDLE)
    , current_rep_(0)
    , strikes_in_current_rep_(0)
    , beats_in_current_rep_(0)
    , expected_strikes_per_rep_(4)
    , expected_beats_per_rep_(4)
    , timing_error_sum_(0)
    , timing_error_max_(0)
    , strikes_good_(0)
    , strikes_acceptable_(0)
    , session_start_time_(0)
    , rep_start_time_(0)
    , attempt_count_(0)
{
    // Initialize with standard difficulty preset
    config_ = DifficultyPresets::standard();

    // Clear last rep result
    memset(&last_rep_result_, 0, sizeof(RepetitionResult));
}

bool SessionManager::load_exercise(uint16_t exercise_id, uint8_t tempo) {
    // Get exercise from library
    current_exercise_ = ExerciseLibrary::get_exercise(exercise_id);

    if (current_exercise_ == nullptr) {
        printf("SessionManager: Exercise %d not found\n", exercise_id);
        return false;
    }

    // Set tempo
    current_tempo_ = tempo;

    // Set expected strikes (from exercise definition)
    expected_strikes_per_rep_ = current_exercise_->beats_per_pattern;
    expected_beats_per_rep_ = current_exercise_->beats_per_pattern;

    // Reset session state
    current_rep_ = 0;
    attempt_count_ = 0;
    state_ = SESSION_READY;

    printf("SessionManager: Loaded Exercise %d (%s) at %d BPM\n",
           current_exercise_->id, current_exercise_->name, current_tempo_);

    return true;
}

void SessionManager::start_session() {
    if (state_ != SESSION_READY && state_ != SESSION_PAUSED) {
        printf("SessionManager: Cannot start - not in READY state\n");
        return;
    }

    state_ = SESSION_ACTIVE;
    session_start_time_ = xTaskGetTickCount();
    rep_start_time_ = session_start_time_;
    reset_rep_counters();

    printf("SessionManager: Session started - GO!\n");
}

void SessionManager::pause_session() {
    if (state_ == SESSION_ACTIVE) {
        state_ = SESSION_PAUSED;
        printf("SessionManager: Session paused\n");
    }
}

void SessionManager::resume_session() {
    if (state_ == SESSION_PAUSED) {
        state_ = SESSION_ACTIVE;
        printf("SessionManager: Session resumed\n");
    }
}

void SessionManager::reset_session() {
    current_rep_ = 0;
    attempt_count_++;
    reset_rep_counters();
    state_ = SESSION_ACTIVE;
    session_start_time_ = xTaskGetTickCount();

    printf("SessionManager: Session reset - starting over from 0/20\n");
}

void SessionManager::abort_session() {
    state_ = SESSION_IDLE;
    current_rep_ = 0;
    attempt_count_ = 0;
    reset_rep_counters();

    printf("SessionManager: Session aborted\n");
}

void SessionManager::process_strike(int32_t timing_offset_ms) {
    if (state_ != SESSION_ACTIVE) {
        return;  // Ignore strikes when not active
    }

    // Increment strike counter
    strikes_in_current_rep_++;

    // Track timing accuracy
    timing_error_sum_ += timing_offset_ms;

    // Track max error (absolute value)
    int32_t abs_error = abs(timing_offset_ms);
    if (abs_error > timing_error_max_) {
        timing_error_max_ = abs_error;
    }

    // Categorize strike accuracy
    if (abs_error <= config_.timing_window_good) {
        strikes_good_++;
        strikes_acceptable_++;
    } else if (abs_error <= config_.timing_window_acceptable) {
        strikes_acceptable_++;
    }

    if (config_.verbose_logging) {
        printf("  Strike %d/%d: %+ld ms\n",
               strikes_in_current_rep_,
               expected_strikes_per_rep_,
               (long)timing_offset_ms);
    }

    // Check if we have too many strikes (extra hits)
    if (strikes_in_current_rep_ > expected_strikes_per_rep_) {
        printf("SessionManager: Too many strikes! Expected %d, got %d - FAIL\n",
               expected_strikes_per_rep_, strikes_in_current_rep_);
        fail_session();
        return;
    }
}

void SessionManager::process_beat() {
    if (state_ != SESSION_ACTIVE) {
        return;
    }

    beats_in_current_rep_++;

    // Check if pattern is complete (received all expected beats)
    if (beats_in_current_rep_ >= expected_beats_per_rep_) {
        // Pattern complete - finalize this repetition
        finalize_repetition();
    }
}

void SessionManager::finalize_repetition() {
    // Calculate average timing error
    int32_t avg_error = (strikes_in_current_rep_ > 0) ?
                        (timing_error_sum_ / strikes_in_current_rep_) : 0;

    // Build repetition result
    last_rep_result_.rep_number = current_rep_ + 1;  // 1-based for display
    last_rep_result_.strikes_detected = strikes_in_current_rep_;
    last_rep_result_.strikes_expected = expected_strikes_per_rep_;
    last_rep_result_.strikes_good = strikes_good_;
    last_rep_result_.strikes_acceptable = strikes_acceptable_;
    last_rep_result_.avg_timing_error_ms = avg_error;
    last_rep_result_.max_timing_error_ms = timing_error_max_;
    last_rep_result_.duration_ms = xTaskGetTickCount() - rep_start_time_;

    // Determine if this repetition was successful
    bool correct_count = (strikes_in_current_rep_ == expected_strikes_per_rep_);
    bool timing_ok = (timing_error_max_ <= config_.timing_window_max);
    bool accuracy_ok = true;

    // Check accuracy percentage
    if (strikes_in_current_rep_ > 0) {
        uint8_t accuracy_percent = (strikes_acceptable_ * 100) / strikes_in_current_rep_;
        accuracy_ok = (accuracy_percent >= config_.min_accuracy_percent);
    }

    last_rep_result_.success = correct_count && timing_ok && accuracy_ok;

    if (last_rep_result_.success) {
        // SUCCESS! Increment rep counter
        current_rep_++;

        printf("SessionManager: Rep %d/20 SUCCESS (avg: %+ld ms, max: %ld ms, accuracy: %d/%d)\n",
               current_rep_,
               (long)avg_error,
               (long)timing_error_max_,
               strikes_acceptable_,
               strikes_in_current_rep_);

        // Check if we've completed all 20 reps
        if (current_rep_ >= config_.required_reps) {
            complete_session();
            return;
        }
    } else {
        // FAILURE! Reset to 0
        printf("SessionManager: Rep %d FAILED - RESET TO 0!\n", current_rep_ + 1);
        printf("  Reason: count=%d/%d, timing_max=%ld ms (limit=%ld), accuracy=%d/%d\n",
               strikes_in_current_rep_,
               expected_strikes_per_rep_,
               (long)timing_error_max_,
               (long)config_.timing_window_max,
               strikes_acceptable_,
               strikes_in_current_rep_);

        fail_session();
        return;
    }

    // Prepare for next rep
    reset_rep_counters();
    rep_start_time_ = xTaskGetTickCount();
}

void SessionManager::fail_session() {
    // CRITICAL: Reset rep counter to 0!
    current_rep_ = 0;
    attempt_count_++;

    // Reset counters for next attempt
    reset_rep_counters();
    rep_start_time_ = xTaskGetTickCount();

    // Stay in SESSION_ACTIVE - continue practicing!
    state_ = SESSION_ACTIVE;

    printf("SessionManager: Starting over - Attempt #%d\n", attempt_count_);
}

void SessionManager::complete_session() {
    state_ = SESSION_SUCCESS;

    uint32_t total_duration = xTaskGetTickCount() - session_start_time_;

    printf("\n");
    printf("============================================\n");
    printf("🎉 SUCCESS! Exercise %d completed at %d BPM\n",
           current_exercise_->id, current_tempo_);
    printf("============================================\n");
    printf("Total time: %lu ms\n", total_duration);
    printf("Attempts: %d\n", attempt_count_);
    printf("\n");
}

void SessionManager::reset_rep_counters() {
    strikes_in_current_rep_ = 0;
    beats_in_current_rep_ = 0;
    timing_error_sum_ = 0;
    timing_error_max_ = 0;
    strikes_good_ = 0;
    strikes_acceptable_ = 0;
}

uint32_t SessionManager::get_session_duration_ms() const {
    if (session_start_time_ == 0) {
        return 0;
    }
    return xTaskGetTickCount() - session_start_time_;
}
