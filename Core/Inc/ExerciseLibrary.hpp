#pragma once

#include "Exercise.hpp"
#include <cstdint>

/**
 * @brief Exercise library containing all exercises from Stick Control
 *
 * This class provides access to the complete exercise database.
 * Initially populated with Level 1, Section A (first 12 exercises).
 * More exercises can be added progressively.
 */
class ExerciseLibrary {
public:
    /**
     * @brief Get exercise by ID
     * @param id Exercise ID (1-based)
     * @return Pointer to exercise, or nullptr if not found
     */
    static const Exercise* get_exercise(uint16_t id);

    /**
     * @brief Get total number of exercises in library
     * @return Number of exercises
     */
    static uint16_t get_total_exercises();

    /**
     * @brief Get the first exercise (for starting practice)
     * @return Pointer to first exercise
     */
    static const Exercise* get_first_exercise();

    /**
     * @brief Get the next exercise after current one
     * @param current_id Current exercise ID
     * @return Pointer to next exercise, or nullptr if at end
     */
    static const Exercise* get_next_exercise(uint16_t current_id);

    /**
     * @brief Get previous exercise
     * @param current_id Current exercise ID
     * @return Pointer to previous exercise, or nullptr if at start
     */
    static const Exercise* get_previous_exercise(uint16_t current_id);

    /**
     * @brief Get all exercises in a specific level
     * @param level Level number (1-8)
     * @param count Output parameter for number of exercises found
     * @return Pointer to array of exercise pointers
     */
    static const Exercise** get_exercises_by_level(uint8_t level, uint16_t* count);

    /**
     * @brief Check if an exercise exists
     * @param id Exercise ID
     * @return true if exercise exists
     */
    static bool exercise_exists(uint16_t id);

private:
    // Internal exercise database
    static const Exercise exercises[];
    static const uint16_t exercise_count;
};
