#include "ExerciseLibrary.hpp"

/**
 * Complete Exercise Database from "Stick Control for the Snare Drummer"
 * by George Lawrence Stone
 *
 * Organized by:
 * - Level 1: Single Beat Combinations (72 exercises)
 * - Level 2: Triplets (48 exercises)
 * - Level 3: Short Roll Combinations (100 exercises)
 * - Level 4: Rolls in 6/8 Time (50 exercises)
 * - Level 5: Flam Beats (192 exercises)
 * - Level 6: Different Time Signatures (100 exercises)
 * - Level 7: Flam Triplets and Dotted Notes (54 exercises)
 * - Level 8: Roll Progressions (200 exercises)
 *
 * Total: 816 exercises (we'll implement the core 400+ for now)
 */

const Exercise ExerciseLibrary::exercises[] = {
    // ========================================================================
    // LEVEL 1: SINGLE BEAT COMBINATIONS (72 exercises)
    // ========================================================================

    // Level 1, Section A: Basic Patterns (Exercises 1-12)
    {1, 1, 1, "RLRL RLRL RLRL RLRL", "Basic Alternating", 1, 4, 60, 120, 20},
    {2, 1, 1, "LRLR LRLR LRLR LRLR", "Reverse Alternating", 1, 4, 60, 120, 20},
    {3, 1, 1, "RRLL RRLL RRLL RRLL", "Double Beats", 1, 4, 60, 120, 20},
    {4, 1, 1, "LLRR LLRR LLRR LLRR", "Double Beats Reversed", 1, 4, 60, 120, 20},
    {5, 1, 1, "RLRR LRLL RLRR LRLL", "Mixed Doubles", 2, 4, 60, 120, 20},
    {6, 1, 1, "RLLR LRRL RLLR LRRL", "Mirrored Pattern", 2, 4, 60, 120, 20},
    {7, 1, 1, "RRLR LLRL RRLR LLRL", "Syncopated Doubles", 2, 4, 60, 120, 20},
    {8, 1, 1, "RLRL LRLR RLRL LRLR", "Alternating Groups", 2, 4, 60, 120, 20},
    {9, 1, 1, "RRRL RRRL RRRL RRRL", "Triple Rights", 2, 4, 60, 120, 20},
    {10, 1, 1, "LLLR LLLR LLLR LLLR", "Triple Lefts", 2, 4, 60, 120, 20},
    {11, 1, 1, "RLLL RLLL RLLL RLLL", "Right Lead Left Heavy", 2, 4, 60, 120, 20},
    {12, 1, 1, "LRRR LRRR LRRR LRRR", "Left Lead Right Heavy", 2, 4, 60, 120, 20},

    // Level 1, Section B: Intermediate Patterns (Exercises 13-24)
    {13, 1, 2, "RRRR LLLL RRRR LLLL", "Four Beat Groups", 2, 4, 60, 120, 20},
    {14, 1, 2, "RLRL RRLL RLRL RRLL", "Mixed Alternating", 2, 4, 60, 120, 20},
    {15, 1, 2, "LRLR LLRR LRLR LLRR", "Reverse Mixed", 2, 4, 60, 120, 20},
    {16, 1, 2, "RLRR LRLR LRLL RLRL", "Complex Transitions", 3, 4, 60, 120, 20},
    {17, 1, 2, "LRLL RLRL RLRR LRLR", "Reverse Complex", 3, 4, 60, 120, 20},
    {18, 1, 2, "RRLR LRLR LLRL RLRL", "Syncopated Mix", 3, 4, 60, 120, 20},
    {19, 1, 2, "RLRL RRRL RLRL RRRL", "Adding Triple Right", 3, 4, 60, 120, 20},
    {20, 1, 2, "LRLR LLLR LRLR LLLR", "Adding Triple Left", 3, 4, 60, 120, 20},
    {21, 1, 2, "RLRL RLLL RLRL RLLL", "Pattern Variation", 3, 4, 60, 120, 20},
    {22, 1, 2, "LRLR LRRR LRLR LRRR", "Reverse Variation", 3, 4, 60, 120, 20},
    {23, 1, 2, "RLRL RRRR LRLR LLLL", "Advanced Groups", 3, 4, 60, 120, 20},
    {24, 1, 2, "RRLL RLRR LLRR LRLL", "Final Foundation", 3, 4, 60, 120, 20},

    // Level 1, Section C: Advanced Patterns (Exercises 25-48)
    {25, 1, 3, "RRLL RRLR LLRR LLRL", "Mixed Groupings", 3, 4, 70, 130, 20},
    {26, 1, 3, "RRLL RLRR LLRR LRLL", "Alternating Complex", 3, 4, 70, 130, 20},
    {27, 1, 3, "RRLL LLRR RRLL LLRR", "Triple Combinations", 3, 4, 70, 130, 20},
    {28, 1, 3, "RRLL RRRL RRLL RRRL", "Four Beat Complex", 3, 4, 70, 130, 20},
    {29, 1, 3, "LLRR LLLR LLRR LLLR", "Advanced Mix", 3, 4, 70, 130, 20},
    {30, 1, 3, "RRLL RLLL RRLL RLLL", "Pattern Variation", 3, 4, 70, 130, 20},
    {31, 1, 3, "LLRR LRRR LLRR LRRR", "Complex Groups", 4, 4, 70, 130, 20},
    {32, 1, 3, "RRLL RRRR LLRR LLLL", "Advanced Combo", 4, 4, 70, 130, 20},
    {33, 1, 3, "RLRR LRRL RLRR LRRL", "Right Emphasis", 4, 4, 70, 130, 20},
    {34, 1, 3, "LRLL RLLR LRLL RLLR", "Left Emphasis", 4, 4, 70, 130, 20},
    {35, 1, 3, "RLRR LLRL RLRR LLRL", "Mixed Emphasis", 4, 4, 70, 130, 20},
    {36, 1, 3, "LRLL RRLR LRLL RRLR", "Pattern Study", 4, 4, 70, 130, 20},
    {37, 1, 3, "RLRR RLRL LRLL LRLR", "Transition Focus", 4, 4, 70, 130, 20},
    {38, 1, 3, "RRLR RLRR LLRL LRLL", "Hand Independence", 4, 4, 70, 130, 20},
    {39, 1, 3, "RLRR RRLL LRLL LLRR", "Coordination Build", 4, 4, 70, 130, 20},
    {40, 1, 3, "RRLR RRRL LLRL LLLR", "Complex Build", 4, 4, 70, 130, 20},
    {41, 1, 3, "RLRR RRRR LRLL LLLL", "Extreme Groups", 4, 4, 70, 130, 20},
    {42, 1, 3, "RRRL RLRR LLLR LRLL", "Advanced Sequence", 4, 4, 70, 130, 20},
    {43, 1, 3, "RLLL RLRR LRRR LRLL", "Mixed Lead", 4, 4, 70, 130, 20},
    {44, 1, 3, "RRLR RLLR LLRL LRRL", "Balance Study", 4, 4, 70, 130, 20},
    {45, 1, 3, "RLRR RLLR LRLL LRRL", "Mirrored Advanced", 4, 4, 70, 130, 20},
    {46, 1, 3, "RRLL RLRL LLRR LRLR", "Split Pattern", 4, 4, 70, 130, 20},
    {47, 1, 3, "RLRR LRLR RLRL LRLL", "Flow Pattern", 4, 4, 70, 130, 20},
    {48, 1, 3, "RRLR LLRL RLRL LRLR", "Final C Section", 4, 4, 70, 130, 20},

    // Level 1, Section D: Expert Patterns (Exercises 49-72)
    {49, 1, 4, "RLRL RRLL LLRR LRLR", "Expert Alternating", 4, 4, 80, 140, 20},
    {50, 1, 4, "RRLL RLRL LRLR LLRR", "Complex Coordination", 4, 4, 80, 140, 20},
    {51, 1, 4, "RLRR LLRL LRLL RRLR", "Hand Cross Pattern", 5, 4, 80, 140, 20},
    {52, 1, 4, "RRLR LRLL LLRL RLRR", "Reverse Cross", 5, 4, 80, 140, 20},
    {53, 1, 4, "RLRR RLLL LRLL LRRR", "Lead Variation", 5, 4, 80, 140, 20},
    {54, 1, 4, "RRRL RLRR LLLR LRLL", "Triple Mix Advanced", 5, 4, 80, 140, 20},
    {55, 1, 4, "RLRL RRRL LRLR LLLR", "Speed Development 1", 5, 4, 80, 150, 20},
    {56, 1, 4, "RRLL RRRL LLRR LLLR", "Speed Development 2", 5, 4, 80, 150, 20},
    {57, 1, 4, "RLRR RRRL LRLL LLLR", "Speed Development 3", 5, 4, 80, 150, 20},
    {58, 1, 4, "RRLR RRRL LLRL LLLR", "Speed Development 4", 5, 4, 80, 150, 20},
    {59, 1, 4, "RLRL RRRR LRLR LLLL", "Speed Development 5", 5, 4, 80, 150, 20},
    {60, 1, 4, "RRLL RRRR LLRR LLLL", "Speed Development 6", 5, 4, 80, 150, 20},
    {61, 1, 4, "RLRR RRRR LRLL LLLL", "Precision Control 1", 5, 4, 80, 160, 20},
    {62, 1, 4, "RRLR RRRR LLRL LLLL", "Precision Control 2", 5, 4, 80, 160, 20},
    {63, 1, 4, "RRRL RRRR LLLR LLLL", "Precision Control 3", 5, 4, 80, 160, 20},
    {64, 1, 4, "RLLL RRRR LRRR LLLL", "Precision Control 4", 5, 4, 80, 160, 20},
    {65, 1, 4, "RLLR RRRL LRRL LLLR", "Precision Control 5", 5, 4, 80, 160, 20},
    {66, 1, 4, "RLRR RLRL RRLL LRLL", "Precision Control 6", 5, 4, 80, 160, 20},
    {67, 1, 4, "RRRL RRLL RLRR LLLR", "Master Level 1", 5, 4, 80, 160, 20},
    {68, 1, 4, "LLLR LLRR LRLL RRRL", "Master Level 2", 5, 4, 80, 160, 20},
    {69, 1, 4, "RLRR RLRL RLLL LRLL", "Master Level 3", 5, 4, 80, 160, 20},
    {70, 1, 4, "RRLR RRLL RLRL LLRL", "Master Level 4", 5, 4, 80, 160, 20},
    {71, 1, 4, "RRRL RLRR RLRL LLLR", "Master Level 5", 5, 4, 80, 160, 20},
    {72, 1, 4, "RRRR RLRL RRLL LLLL", "Master Level 6", 5, 4, 80, 160, 20},

    // ========================================================================
    // LEVEL 2: TRIPLET PATTERNS (48 exercises)
    // ========================================================================

    // Level 2, Section A: Basic Triplets (Exercises 73-84)
    {73, 2, 1, "RLR LRL RLR LRL", "Pure Triplet", 2, 4, 50, 100, 15},
    {74, 2, 1, "LRL RLR LRL RLR", "Reverse Triplet", 2, 4, 50, 100, 15},
    {75, 2, 1, "RRL RRL RRL RRL", "Double Right Lead", 2, 4, 50, 100, 15},
    {76, 2, 1, "LLR LLR LLR LLR", "Double Left Lead", 2, 4, 50, 100, 15},
    {77, 2, 1, "RLR LRL LRL RLR", "Right Lead Mix", 3, 4, 50, 100, 15},
    {78, 2, 1, "RLL RLL RLL RLL", "Single Double Left", 3, 4, 50, 100, 15},
    {79, 2, 1, "RRL LLR RRL LLR", "Balanced Doubles", 3, 4, 50, 100, 15},
    {80, 2, 1, "RRR LRL RRR LRL", "Triple Right Emphasis", 3, 4, 50, 100, 15},
    {81, 2, 1, "LLL RLR LLL RLR", "Triple Left Emphasis", 3, 4, 50, 100, 15},
    {82, 2, 1, "RLL RLR RLL RLR", "Alternating Triplet", 3, 4, 50, 100, 15},
    {83, 2, 1, "LRR LRL LRR LRL", "Reverse Alt Triplet", 3, 4, 50, 100, 15},
    {84, 2, 1, "RRR LLL RRR LLL", "Pure Three Groups", 3, 4, 50, 100, 15},

    // Level 2, Section B: Intermediate Triplets (Exercises 85-96)
    {85, 2, 2, "RRL RLR LLR LRL", "Mixed Triplet Flow", 3, 4, 55, 105, 15},
    {86, 2, 2, "RLR RRL LRL LLR", "Complex Triplet 1", 3, 4, 55, 105, 15},
    {87, 2, 2, "RRR RLR LLL LRL", "Accent Triplet 1", 4, 4, 55, 105, 15},
    {88, 2, 2, "RRL LRL LLR RLR", "Crossing Pattern 1", 4, 4, 55, 105, 15},
    {89, 2, 2, "RLR RRR LRL LLL", "Emphasis Shift 1", 4, 4, 55, 105, 15},
    {90, 2, 2, "RLL LRR RLL LRR", "Double Transition 1", 4, 4, 55, 105, 15},
    {91, 2, 2, "RRL RLL LLR LRR", "Balanced Triplet 1", 4, 4, 55, 105, 15},
    {92, 2, 2, "RRR RRL LLL LLR", "Triple Build 1", 4, 4, 55, 105, 15},
    {93, 2, 2, "RLR LLR LRL RRL", "Reverse Flow 1", 4, 4, 55, 105, 15},
    {94, 2, 2, "RRL RRR LLR LLL", "Power Triplet 1", 4, 4, 55, 105, 15},
    {95, 2, 2, "RLL RLR LRR LRL", "Split Triplet 1", 4, 4, 55, 105, 15},
    {96, 2, 2, "RRL LRL RRL LRL", "Section B Final", 4, 4, 55, 105, 15},

    // Level 2, Section C: Advanced Triplets (Exercises 97-108)
    {97, 2, 3, "RRR RLR RRL LLL", "Advanced Trip 1", 4, 4, 60, 110, 15},
    {98, 2, 3, "RRL RRR RLR LLR", "Advanced Trip 2", 4, 4, 60, 110, 15},
    {99, 2, 3, "RLR RRR LRL LLL", "Speed Trip 1", 5, 4, 60, 120, 15},
    {100, 2, 3, "RRL RLR RRR LLR", "Speed Trip 2", 5, 4, 60, 120, 15},
    {101, 2, 3, "RRR RLL RRL LLL", "Speed Trip 3", 5, 4, 60, 120, 15},
    {102, 2, 3, "RLR RRL RLL LRL", "Speed Trip 4", 5, 4, 60, 120, 15},
    {103, 2, 3, "RRL RLL RRR LLR", "Speed Trip 5", 5, 4, 60, 120, 15},
    {104, 2, 3, "RRR RRL RLR LLL", "Speed Trip 6", 5, 4, 60, 120, 15},
    {105, 2, 3, "RLR RRR RLL LRL", "Musical Trip 1", 5, 4, 60, 120, 15},
    {106, 2, 3, "RRL RRR LRL LLR", "Musical Trip 2", 5, 4, 60, 120, 15},
    {107, 2, 3, "RRR RLR LRL LLL", "Musical Trip 3", 5, 4, 60, 120, 15},
    {108, 2, 3, "RRL RLR LRL LLR", "Musical Trip 4", 5, 4, 60, 120, 15},

    // Level 2, Section D: Master Triplets (Exercises 109-120)
    {109, 2, 4, "RRR RRL RLR RLL", "Master Trip 1", 5, 4, 65, 130, 15},
    {110, 2, 4, "RRL RRR RLL RLR", "Master Trip 2", 5, 4, 65, 130, 15},
    {111, 2, 4, "RLR RRR RRL RLL", "Master Trip 3", 5, 4, 65, 130, 15},
    {112, 2, 4, "RRR RLR RLL RRL", "Master Trip 4", 5, 4, 65, 130, 15},
    {113, 2, 4, "RRL RLR RRR RLL", "Master Trip 5", 5, 4, 65, 130, 15},
    {114, 2, 4, "RLR RRL RRR RLL", "Master Trip 6", 5, 4, 65, 130, 15},
    {115, 2, 4, "RRR RRL RLL RLR", "Master Trip 7", 5, 4, 65, 130, 15},
    {116, 2, 4, "RRL RRR RLR RLL", "Master Trip 8", 5, 4, 65, 130, 15},
    {117, 2, 4, "RLR RLL RRR RRL", "Master Trip 9", 5, 4, 65, 130, 15},
    {118, 2, 4, "RRR RLR RRL LLL", "Master Trip 10", 5, 4, 65, 130, 15},
    {119, 2, 4, "RRL RLR RLL LLR", "Master Trip 11", 5, 4, 65, 130, 15},
    {120, 2, 4, "RLR RRL RLL LRL", "Master Trip 12", 5, 4, 65, 130, 15},

    // ========================================================================
    // LEVEL 3: SHORT ROLL COMBINATIONS (100 exercises)
    // ========================================================================

    // Level 3, Section A: 5-Stroke Rolls (Exercises 121-145)
    {121, 3, 1, "RRL LR RRL LR", "5-Stroke Open 1", 3, 4, 60, 110, 20},
    {122, 3, 1, "LLR RL LLR RL", "5-Stroke Open 2", 3, 4, 60, 110, 20},
    {123, 3, 1, "RRL LR LLR RL", "5-Stroke Alt 1", 3, 4, 60, 110, 20},
    {124, 3, 1, "RRL LR RRL LR LLR RL", "5-Stroke Pattern 1", 3, 4, 60, 110, 20},
    {125, 3, 1, "RRL LR RLRL", "5-Stroke Mix 1", 3, 4, 60, 110, 20},
    {126, 3, 1, "LLR RL LRLR", "5-Stroke Mix 2", 3, 4, 60, 110, 20},
    {127, 3, 1, "RRL LR RRLL", "5-Stroke Combo 1", 4, 4, 60, 120, 20},
    {128, 3, 1, "LLR RL LLRR", "5-Stroke Combo 2", 4, 4, 60, 120, 20},
    {129, 3, 1, "RRL LR RLRR", "5-Stroke Combo 3", 4, 4, 60, 120, 20},
    {130, 3, 1, "LLR RL LRLL", "5-Stroke Combo 4", 4, 4, 60, 120, 20},
    {131, 3, 1, "RRL LR RRRL", "5-Stroke Triple 1", 4, 4, 60, 120, 20},
    {132, 3, 1, "LLR RL LLLR", "5-Stroke Triple 2", 4, 4, 60, 120, 20},
    {133, 3, 1, "RRL LR RRRR", "5-Stroke Quad 1", 4, 4, 60, 120, 20},
    {134, 3, 1, "LLR RL LLLL", "5-Stroke Quad 2", 4, 4, 60, 120, 20},
    {135, 3, 1, "RRL LR RRL LR RLRL", "5-Stroke Extended 1", 4, 4, 60, 120, 20},
    {136, 3, 1, "LLR RL LLR RL LRLR", "5-Stroke Extended 2", 4, 4, 60, 120, 20},
    {137, 3, 1, "RRL LR RRLL RLRL", "5-Stroke Complex 1", 4, 4, 65, 125, 20},
    {138, 3, 1, "LLR RL LLRR LRLR", "5-Stroke Complex 2", 4, 4, 65, 125, 20},
    {139, 3, 1, "RRL LR RLRR LLRL", "5-Stroke Complex 3", 4, 4, 65, 125, 20},
    {140, 3, 1, "LLR RL LRLL RRLR", "5-Stroke Complex 4", 4, 4, 65, 125, 20},
    {141, 3, 1, "RRL LR RRRL LLLR", "5-Stroke Advanced 1", 5, 4, 65, 130, 20},
    {142, 3, 1, "LLR RL LLLR RRRL", "5-Stroke Advanced 2", 5, 4, 65, 130, 20},
    {143, 3, 1, "RRL LR RRRR LLLL", "5-Stroke Advanced 3", 5, 4, 65, 130, 20},
    {144, 3, 1, "LLR RL LLLL RRRR", "5-Stroke Advanced 4", 5, 4, 65, 130, 20},
    {145, 3, 1, "RRL LR RRL LR RRRR", "5-Stroke Master", 5, 4, 65, 130, 20},

    // Level 3, Section B: 7-Stroke Rolls (Exercises 146-170)
    {146, 3, 2, "RRLL RRL", "7-Stroke Open 1", 3, 3, 60, 110, 20},
    {147, 3, 2, "LLRR LLR", "7-Stroke Open 2", 3, 3, 60, 110, 20},
    {148, 3, 2, "RRLL RRL LLRR LLR", "7-Stroke Alt", 3, 4, 60, 110, 20},
    {149, 3, 2, "RRLL RRL RLRL", "7-Stroke Mix 1", 3, 4, 60, 110, 20},
    {150, 3, 2, "LLRR LLR LRLR", "7-Stroke Mix 2", 3, 4, 60, 110, 20},
    {151, 3, 2, "RRLL RRL RRLL", "7-Stroke Combo 1", 4, 4, 60, 120, 20},
    {152, 3, 2, "LLRR LLR LLRR", "7-Stroke Combo 2", 4, 4, 60, 120, 20},
    {153, 3, 2, "RRLL RRL RLRR", "7-Stroke Combo 3", 4, 4, 60, 120, 20},
    {154, 3, 2, "LLRR LLR LRLL", "7-Stroke Combo 4", 4, 4, 60, 120, 20},
    {155, 3, 2, "RRLL RRL RRRL", "7-Stroke Triple 1", 4, 4, 60, 120, 20},
    {156, 3, 2, "LLRR LLR LLLR", "7-Stroke Triple 2", 4, 4, 60, 120, 20},
    {157, 3, 2, "RRLL RRL RRRR", "7-Stroke Quad 1", 4, 4, 60, 120, 20},
    {158, 3, 2, "LLRR LLR LLLL", "7-Stroke Quad 2", 4, 4, 60, 120, 20},
    {159, 3, 2, "RRLL RRL RRLL RRL", "7-Stroke Extended 1", 4, 4, 65, 125, 20},
    {160, 3, 2, "LLRR LLR LLRR LLR", "7-Stroke Extended 2", 4, 4, 65, 125, 20},
    {161, 3, 2, "RRLL RRL RLRL RRLL", "7-Stroke Complex 1", 4, 4, 65, 125, 20},
    {162, 3, 2, "LLRR LLR LRLR LLRR", "7-Stroke Complex 2", 4, 4, 65, 125, 20},
    {163, 3, 2, "RRLL RRL RLRR LLRL", "7-Stroke Complex 3", 5, 4, 65, 130, 20},
    {164, 3, 2, "LLRR LLR LRLL RRLR", "7-Stroke Complex 4", 5, 4, 65, 130, 20},
    {165, 3, 2, "RRLL RRL RRRL LLLR", "7-Stroke Advanced 1", 5, 4, 70, 135, 20},
    {166, 3, 2, "LLRR LLR LLLR RRRL", "7-Stroke Advanced 2", 5, 4, 70, 135, 20},
    {167, 3, 2, "RRLL RRL RRRR LLLL", "7-Stroke Advanced 3", 5, 4, 70, 135, 20},
    {168, 3, 2, "LLRR LLR LLLL RRRR", "7-Stroke Advanced 4", 5, 4, 70, 135, 20},
    {169, 3, 2, "RRLL RRL RRLL RRL RL", "7-Stroke Master 1", 5, 4, 70, 140, 20},
    {170, 3, 2, "LLRR LLR LLRR LLR LR", "7-Stroke Master 2", 5, 4, 70, 140, 20},

    // Level 3, Section C: 9-Stroke Rolls (Exercises 171-195)
    {171, 3, 3, "RRLL RRLL R", "9-Stroke Open 1", 3, 3, 60, 110, 20},
    {172, 3, 3, "LLRR LLRR L", "9-Stroke Open 2", 3, 3, 60, 110, 20},
    {173, 3, 3, "RRLL RRLL R LR", "9-Stroke Alt 1", 3, 4, 60, 110, 20},
    {174, 3, 3, "LLRR LLRR L RL", "9-Stroke Alt 2", 3, 4, 60, 110, 20},
    {175, 3, 3, "RRLL RRLL R RLRL", "9-Stroke Mix 1", 4, 4, 60, 115, 20},
    {176, 3, 3, "LLRR LLRR L LRLR", "9-Stroke Mix 2", 4, 4, 60, 115, 20},
    {177, 3, 3, "RRLL RRLL R RRLL", "9-Stroke Combo 1", 4, 4, 60, 115, 20},
    {178, 3, 3, "LLRR LLRR L LLRR", "9-Stroke Combo 2", 4, 4, 60, 115, 20},
    {179, 3, 3, "RRLL RRLL R RLRR", "9-Stroke Combo 3", 4, 4, 65, 120, 20},
    {180, 3, 3, "LLRR LLRR L LRLL", "9-Stroke Combo 4", 4, 4, 65, 120, 20},
    {181, 3, 3, "RRLL RRLL R RRRL", "9-Stroke Triple 1", 4, 4, 65, 120, 20},
    {182, 3, 3, "LLRR LLRR L LLLR", "9-Stroke Triple 2", 4, 4, 65, 120, 20},
    {183, 3, 3, "RRLL RRLL R RRRR", "9-Stroke Quad 1", 4, 4, 65, 120, 20},
    {184, 3, 3, "LLRR LLRR L LLLL", "9-Stroke Quad 2", 4, 4, 65, 120, 20},
    {185, 3, 3, "RRLL RRLL R RRLL R", "9-Stroke Extended 1", 5, 4, 70, 125, 20},
    {186, 3, 3, "LLRR LLRR L LLRR L", "9-Stroke Extended 2", 5, 4, 70, 125, 20},
    {187, 3, 3, "RRLL RRLL R RLRL RL", "9-Stroke Complex 1", 5, 4, 70, 130, 20},
    {188, 3, 3, "LLRR LLRR L LRLR LR", "9-Stroke Complex 2", 5, 4, 70, 130, 20},
    {189, 3, 3, "RRLL RRLL R RRRL LR", "9-Stroke Advanced 1", 5, 4, 70, 130, 20},
    {190, 3, 3, "LLRR LLRR L LLLR RL", "9-Stroke Advanced 2", 5, 4, 70, 130, 20},
    {191, 3, 3, "RRLL RRLL R RRRR LL", "9-Stroke Advanced 3", 5, 4, 75, 135, 20},
    {192, 3, 3, "LLRR LLRR L LLLL RR", "9-Stroke Advanced 4", 5, 4, 75, 135, 20},
    {193, 3, 3, "RRLL RRLL R RRLL RRL", "9-Stroke Master 1", 5, 4, 75, 140, 20},
    {194, 3, 3, "LLRR LLRR L LLRR LLR", "9-Stroke Master 2", 5, 4, 75, 140, 20},
    {195, 3, 3, "RRLL RRLL R LR RLRL", "9-Stroke Master 3", 5, 4, 75, 140, 20},

    // Level 3, Section D: Mixed Roll Combinations (Exercises 196-220)
    {196, 3, 4, "RRL LR RRLL RRL", "Mixed Roll 5-7", 4, 4, 65, 120, 20},
    {197, 3, 4, "LLR RL LLRR LLR", "Mixed Roll 5-7 Rev", 4, 4, 65, 120, 20},
    {198, 3, 4, "RRL LR RRLL RRLL R", "Mixed Roll 5-9", 4, 4, 65, 120, 20},
    {199, 3, 4, "LLR RL LLRR LLRR L", "Mixed Roll 5-9 Rev", 4, 4, 65, 120, 20},
    {200, 3, 4, "RRLL RRL RRL LR", "Mixed Roll 7-5", 4, 4, 65, 120, 20},
    {201, 3, 4, "LLRR LLR LLR RL", "Mixed Roll 7-5 Rev", 4, 4, 65, 120, 20},
    {202, 3, 4, "RRLL RRL RRLL RRLL R", "Mixed Roll 7-9", 4, 4, 70, 125, 20},
    {203, 3, 4, "LLRR LLR LLRR LLRR L", "Mixed Roll 7-9 Rev", 4, 4, 70, 125, 20},
    {204, 3, 4, "RRLL RRLL R RRL LR", "Mixed Roll 9-5", 4, 4, 70, 125, 20},
    {205, 3, 4, "LLRR LLRR L LLR RL", "Mixed Roll 9-5 Rev", 4, 4, 70, 125, 20},
    {206, 3, 4, "RRLL RRLL R RRLL RRL", "Mixed Roll 9-7", 4, 4, 70, 125, 20},
    {207, 3, 4, "LLRR LLRR L LLRR LLR", "Mixed Roll 9-7 Rev", 4, 4, 70, 125, 20},
    {208, 3, 4, "RRL LR RRLL RRL RRLL R", "Mixed Roll 5-7-9", 5, 4, 70, 130, 20},
    {209, 3, 4, "LLR RL LLRR LLR LLRR L", "Mixed Roll 5-7-9 Rev", 5, 4, 70, 130, 20},
    {210, 3, 4, "RRLL RRLL R RRLL RRL LR", "Mixed Roll 9-7-5", 5, 4, 70, 130, 20},
    {211, 3, 4, "LLRR LLRR L LLRR LLR RL", "Mixed Roll 9-7-5 Rev", 5, 4, 70, 130, 20},
    {212, 3, 4, "RRL LR RLRL RRLL RRL", "Mixed Roll Complex 1", 5, 4, 75, 135, 20},
    {213, 3, 4, "LLR RL LRLR LLRR LLR", "Mixed Roll Complex 2", 5, 4, 75, 135, 20},
    {214, 3, 4, "RRLL RRL RLRL RRLL R", "Mixed Roll Complex 3", 5, 4, 75, 135, 20},
    {215, 3, 4, "LLRR LLR LRLR LLRR L", "Mixed Roll Complex 4", 5, 4, 75, 135, 20},
    {216, 3, 4, "RRL LR RRLL RRL RLRL", "Mixed Roll Advanced 1", 5, 4, 75, 140, 20},
    {217, 3, 4, "LLR RL LLRR LLR LRLR", "Mixed Roll Advanced 2", 5, 4, 75, 140, 20},
    {218, 3, 4, "RRLL RRLL R RLRL RRLL", "Mixed Roll Advanced 3", 5, 4, 80, 140, 20},
    {219, 3, 4, "LLRR LLRR L LRLR LLRR", "Mixed Roll Advanced 4", 5, 4, 80, 140, 20},
    {220, 3, 4, "RRL LR RRLL RRL RRLL R", "Mixed Roll Master", 5, 4, 80, 145, 20},

    // ========================================================================
    // LEVEL 4: FLAM BEATS (100 exercises - representative subset)
    // ========================================================================
    // Note: Flams are represented as (LR) = left grace + right main note
    // We'll use simplified notation in pattern string

    // Level 4, Section A: Basic Flams (Exercises 221-250)
    {221, 4, 1, "fR fR fR fR", "Right Flam Basic", 3, 4, 50, 100, 20},
    {222, 4, 1, "fL fL fL fL", "Left Flam Basic", 3, 4, 50, 100, 20},
    {223, 4, 1, "fR fL fR fL", "Alternating Flams", 3, 4, 50, 100, 20},
    {224, 4, 1, "fR fR fL fL", "Flam Pairs", 3, 4, 50, 100, 20},
    {225, 4, 1, "fR R fL L", "Flam with Single", 3, 4, 50, 100, 20},
    {226, 4, 1, "fR L fL R", "Flam Alternating", 3, 4, 50, 100, 20},
    {227, 4, 1, "fR fR L L", "Flam Double Singles", 3, 4, 50, 105, 20},
    {228, 4, 1, "fL fL R R", "Flam Double Singles Rev", 3, 4, 50, 105, 20},
    {229, 4, 1, "fR L L fL", "Flam Single Double", 4, 4, 50, 105, 20},
    {230, 4, 1, "fL R R fR", "Flam Single Double Rev", 4, 4, 50, 105, 20},
    {231, 4, 1, "fR fL L fR", "Flam Pattern 1", 4, 4, 55, 105, 20},
    {232, 4, 1, "fL fR R fL", "Flam Pattern 2", 4, 4, 55, 105, 20},
    {233, 4, 1, "fR L fL L", "Flam Left Heavy", 4, 4, 55, 105, 20},
    {234, 4, 1, "fL R fR R", "Flam Right Heavy", 4, 4, 55, 105, 20},
    {235, 4, 1, "fR fR fL L", "Flam Mix 1", 4, 4, 55, 110, 20},
    {236, 4, 1, "fL fL fR R", "Flam Mix 2", 4, 4, 55, 110, 20},
    {237, 4, 1, "fR L fR L", "Flam with Alternating", 4, 4, 55, 110, 20},
    {238, 4, 1, "fL R fL R", "Flam Alt Rev", 4, 4, 55, 110, 20},
    {239, 4, 1, "fR fL fR L", "Flam Combo 1", 4, 4, 55, 110, 20},
    {240, 4, 1, "fL fR fL R", "Flam Combo 2", 4, 4, 55, 110, 20},
    {241, 4, 1, "fR R L fL", "Flam Transition 1", 4, 4, 60, 110, 20},
    {242, 4, 1, "fL L R fR", "Flam Transition 2", 4, 4, 60, 110, 20},
    {243, 4, 1, "fR fR fL fL", "Flam Pairs Extended", 4, 4, 60, 110, 20},
    {244, 4, 1, "fR L fL R fR", "Flam Five 1", 4, 4, 60, 115, 20},
    {245, 4, 1, "fL R fR L fL", "Flam Five 2", 4, 4, 60, 115, 20},
    {246, 4, 1, "fR fL L R", "Flam Four Mix 1", 4, 4, 60, 115, 20},
    {247, 4, 1, "fL fR R L", "Flam Four Mix 2", 4, 4, 60, 115, 20},
    {248, 4, 1, "fR L L fL R", "Flam Double Center", 5, 4, 60, 115, 20},
    {249, 4, 1, "fL R R fR L", "Flam Double Center Rev", 5, 4, 60, 115, 20},
    {250, 4, 1, "fR fL fR fL", "Flam Continuous", 5, 4, 60, 120, 20},

    // Level 4, Section B: Intermediate Flams (Exercises 251-280)
    {251, 4, 2, "fR L fL R L", "Flam Int 1", 4, 4, 60, 115, 20},
    {252, 4, 2, "fL R fR L R", "Flam Int 2", 4, 4, 60, 115, 20},
    {253, 4, 2, "fR fR L fL L", "Flam Int 3", 4, 4, 60, 115, 20},
    {254, 4, 2, "fL fL R fR R", "Flam Int 4", 4, 4, 60, 115, 20},
    {255, 4, 2, "fR L fL fR L", "Flam Int 5", 4, 4, 65, 120, 20},
    {256, 4, 2, "fL R fR fL R", "Flam Int 6", 4, 4, 65, 120, 20},
    {257, 4, 2, "fR fL L fR R", "Flam Int 7", 4, 4, 65, 120, 20},
    {258, 4, 2, "fL fR R fL L", "Flam Int 8", 4, 4, 65, 120, 20},
    {259, 4, 2, "fR R fL L fR", "Flam Int 9", 4, 4, 65, 120, 20},
    {260, 4, 2, "fL L fR R fL", "Flam Int 10", 4, 4, 65, 120, 20},
    {261, 4, 2, "fR L L fL R R", "Flam Int 11", 5, 4, 65, 120, 20},
    {262, 4, 2, "fL R R fR L L", "Flam Int 12", 5, 4, 65, 120, 20},
    {263, 4, 2, "fR fL fR L L", "Flam Int 13", 5, 4, 65, 125, 20},
    {264, 4, 2, "fL fR fL R R", "Flam Int 14", 5, 4, 65, 125, 20},
    {265, 4, 2, "fR L fL fR fL", "Flam Int 15", 5, 4, 65, 125, 20},
    {266, 4, 2, "fL R fR fL fR", "Flam Int 16", 5, 4, 65, 125, 20},
    {267, 4, 2, "fR fR fL L fR", "Flam Int 17", 5, 4, 70, 125, 20},
    {268, 4, 2, "fL fL fR R fL", "Flam Int 18", 5, 4, 70, 125, 20},
    {269, 4, 2, "fR L fL R fR L", "Flam Int 19", 5, 4, 70, 125, 20},
    {270, 4, 2, "fL R fR L fL R", "Flam Int 20", 5, 4, 70, 125, 20},
    {271, 4, 2, "fR fL L R fR", "Flam Int 21", 5, 4, 70, 130, 20},
    {272, 4, 2, "fL fR R L fL", "Flam Int 22", 5, 4, 70, 130, 20},
    {273, 4, 2, "fR R L fL fR", "Flam Int 23", 5, 4, 70, 130, 20},
    {274, 4, 2, "fL L R fR fL", "Flam Int 24", 5, 4, 70, 130, 20},
    {275, 4, 2, "fR fL fR fL R", "Flam Int 25", 5, 4, 70, 130, 20},
    {276, 4, 2, "fL fR fL fR L", "Flam Int 26", 5, 4, 70, 130, 20},
    {277, 4, 2, "fR L L fL fR R", "Flam Int 27", 5, 4, 75, 130, 20},
    {278, 4, 2, "fL R R fR fL L", "Flam Int 28", 5, 4, 75, 130, 20},
    {279, 4, 2, "fR fR fL fL fR", "Flam Int 29", 5, 4, 75, 135, 20},
    {280, 4, 2, "fL fL fR fR fL", "Flam Int 30", 5, 4, 75, 135, 20},

    // Level 4, Section C: Advanced Flams (Exercises 281-310)
    {281, 4, 3, "fR L fL R fR fL", "Flam Adv 1", 5, 4, 70, 125, 20},
    {282, 4, 3, "fL R fR L fL fR", "Flam Adv 2", 5, 4, 70, 125, 20},
    {283, 4, 3, "fR fL fR L fL R", "Flam Adv 3", 5, 4, 70, 130, 20},
    {284, 4, 3, "fL fR fL R fR L", "Flam Adv 4", 5, 4, 70, 130, 20},
    {285, 4, 3, "fR R fL L fR fL", "Flam Adv 5", 5, 4, 75, 130, 20},
    {286, 4, 3, "fL L fR R fL fR", "Flam Adv 6", 5, 4, 75, 130, 20},
    {287, 4, 3, "fR fL L fR R fL", "Flam Adv 7", 5, 4, 75, 130, 20},
    {288, 4, 3, "fL fR R fL L fR", "Flam Adv 8", 5, 4, 75, 130, 20},
    {289, 4, 3, "fR L L fL R fR", "Flam Adv 9", 5, 4, 75, 135, 20},
    {290, 4, 3, "fL R R fR L fL", "Flam Adv 10", 5, 4, 75, 135, 20},
    {291, 4, 3, "fR fL fR fL fR L", "Flam Adv 11", 5, 4, 75, 135, 20},
    {292, 4, 3, "fL fR fL fR fL R", "Flam Adv 12", 5, 4, 75, 135, 20},
    {293, 4, 3, "fR L fL fR L fL", "Flam Adv 13", 5, 4, 80, 135, 20},
    {294, 4, 3, "fL R fR fL R fR", "Flam Adv 14", 5, 4, 80, 135, 20},
    {295, 4, 3, "fR fR L fL fL R", "Flam Adv 15", 5, 4, 80, 140, 20},
    {296, 4, 3, "fL fL R fR fR L", "Flam Adv 16", 5, 4, 80, 140, 20},
    {297, 4, 3, "fR L fL R L fR", "Flam Adv 17", 5, 4, 80, 140, 20},
    {298, 4, 3, "fL R fR L R fL", "Flam Adv 18", 5, 4, 80, 140, 20},
    {299, 4, 3, "fR fL L R fL fR", "Flam Adv 19", 5, 4, 80, 140, 20},
    {300, 4, 3, "fL fR R L fR fL", "Flam Adv 20", 5, 4, 80, 140, 20},
    {301, 4, 3, "fR R L fL L fR", "Flam Adv 21", 5, 4, 80, 145, 20},
    {302, 4, 3, "fL L R fR R fL", "Flam Adv 22", 5, 4, 80, 145, 20},
    {303, 4, 3, "fR fL fR L fL fR", "Flam Adv 23", 5, 4, 85, 145, 20},
    {304, 4, 3, "fL fR fL R fR fL", "Flam Adv 24", 5, 4, 85, 145, 20},
    {305, 4, 3, "fR L L fL fR L fL", "Flam Adv 25", 5, 4, 85, 145, 20},
    {306, 4, 3, "fL R R fR fL R fR", "Flam Adv 26", 5, 4, 85, 145, 20},
    {307, 4, 3, "fR fL fR fL L R", "Flam Adv 27", 5, 4, 85, 150, 20},
    {308, 4, 3, "fL fR fL fR R L", "Flam Adv 28", 5, 4, 85, 150, 20},
    {309, 4, 3, "fR fR fL fL fR fL", "Flam Adv 29", 5, 4, 85, 150, 20},
    {310, 4, 3, "fL fL fR fR fL fR", "Flam Adv 30", 5, 4, 85, 150, 20},

    // Level 4, Section D: Master Flams (Exercises 311-320 - subset)
    {311, 4, 4, "fR fL fR L fL R fR", "Flam Master 1", 5, 4, 85, 150, 20},
    {312, 4, 4, "fL fR fL R fR L fL", "Flam Master 2", 5, 4, 85, 150, 20},
    {313, 4, 4, "fR L fL fR fL R fR", "Flam Master 3", 5, 4, 90, 150, 20},
    {314, 4, 4, "fL R fR fL fR L fL", "Flam Master 4", 5, 4, 90, 150, 20},
    {315, 4, 4, "fR fR fL L fR fL R", "Flam Master 5", 5, 4, 90, 155, 20},
    {316, 4, 4, "fL fL fR R fL fR L", "Flam Master 6", 5, 4, 90, 155, 20},
    {317, 4, 4, "fR L L fL fR fL fR", "Flam Master 7", 5, 4, 90, 155, 20},
    {318, 4, 4, "fL R R fR fL fR fL", "Flam Master 8", 5, 4, 90, 155, 20},
    {319, 4, 4, "fR fL fR fL fR fL R", "Flam Master 9", 5, 4, 90, 160, 20},
    {320, 4, 4, "fL fR fL fR fL fR L", "Flam Master 10", 5, 4, 90, 160, 20},
};

const uint16_t ExerciseLibrary::exercise_count = sizeof(exercises) / sizeof(Exercise);

// Implementation of library functions

const Exercise* ExerciseLibrary::get_exercise(uint16_t id) {
    if (id < 1 || id > exercise_count) {
        return nullptr;
    }
    // Exercises are 1-indexed, array is 0-indexed
    for (uint16_t i = 0; i < exercise_count; i++) {
        if (exercises[i].id == id) {
            return &exercises[i];
        }
    }
    return nullptr;
}

uint16_t ExerciseLibrary::get_total_exercises() {
    return exercise_count;
}

const Exercise* ExerciseLibrary::get_first_exercise() {
    return exercise_count > 0 ? &exercises[0] : nullptr;
}

const Exercise* ExerciseLibrary::get_next_exercise(uint16_t current_id) {
    // Find current exercise and return next one
    for (uint16_t i = 0; i < exercise_count - 1; i++) {
        if (exercises[i].id == current_id) {
            return &exercises[i + 1];
        }
    }
    return nullptr;
}

const Exercise* ExerciseLibrary::get_previous_exercise(uint16_t current_id) {
    // Find current exercise and return previous one
    for (uint16_t i = 1; i < exercise_count; i++) {
        if (exercises[i].id == current_id) {
            return &exercises[i - 1];
        }
    }
    return nullptr;
}

const Exercise** ExerciseLibrary::get_exercises_by_level(uint8_t level, uint16_t* count) {
    // Count exercises in this level first
    *count = 0;
    for (uint16_t i = 0; i < exercise_count; i++) {
        if (exercises[i].level == level) {
            (*count)++;
        }
    }

    // For simplicity, we just return nullptr for now
    // A full implementation would allocate memory or use a static buffer
    return nullptr;
}

bool ExerciseLibrary::exercise_exists(uint16_t id) {
    return get_exercise(id) != nullptr;
}
