#pragma once

#include <string>
#include <vector>
#include <map>

namespace maya {

// Bank definitions
// Words are ordered to match the generated audio files (alphabetical within bank)

const unsigned int NUM_BANKS = 9;

const std::vector<std::string> BANK_NAMES = {
    "Pronouns",      // Bank 0: 0V-1V
    "Verbs",         // Bank 1: 1V-2V
    "Descriptors",   // Bank 2: 2V-3V
    "Nouns",         // Bank 3: 3V-4V
    "Connectors",    // Bank 4: 4V-5V
    "Time",          // Bank 5: 5V-6V
    "Emotions",      // Bank 6: 6V-7V
    "Numbers",       // Bank 7: 7V-8V
    "Questions"      // Bank 8: 8V-9V
};

// Bank 0: Pronouns & Subjects
const std::vector<std::string> BANK_0_PRONOUNS = {
    "everyone", "he", "her", "him", "his", "I", "it", "me", "mine", "my",
    "nobody", "one", "our", "she", "someone", "them", "their", "they",
    "us", "we", "you", "your", "yours"
};

// Bank 1: Verbs & Actions
const std::vector<std::string> BANK_1_VERBS = {
    "am", "are", "be", "believe", "break", "build", "burn", "call", "come",
    "dance", "fade", "fall", "feel", "fly", "forget", "give", "go", "grow",
    "hate", "hear", "hold", "hurt", "is", "kiss", "know", "let", "love",
    "make", "miss", "move", "need", "remember", "rise", "run", "say", "see",
    "sing", "speak", "spin", "take", "tell", "think", "touch", "walk",
    "want", "was", "watch"
};

// Bank 2: Descriptors & Adjectives
const std::vector<std::string> BANK_2_DESCRIPTORS = {
    "afraid", "alive", "all", "angry", "bad", "big", "bright", "broken",
    "cold", "dark", "dead", "deep", "enough", "false", "fast", "found",
    "free", "good", "happy", "hard", "high", "hot", "less", "light",
    "lonely", "lost", "loud", "low", "more", "new", "none", "old", "quiet",
    "real", "sad", "slow", "small", "soft", "still", "too", "true", "warm",
    "whole", "wild"
};

// Bank 3: Objects & Nouns
const std::vector<std::string> BANK_3_NOUNS = {
    "arms", "blood", "body", "bones", "death", "door", "dream", "earth",
    "edge", "eyes", "fear", "fire", "hands", "head", "heart", "home",
    "hope", "lie", "life", "mind", "moon", "pain", "peace", "place",
    "rain", "road", "room", "sea", "skin", "sky", "soul", "stars", "storm",
    "sun", "time", "truth", "wall", "war", "water", "wave", "wind", "world"
};

// Bank 4: Connectors & Prepositions
const std::vector<std::string> BANK_4_CONNECTORS = {
    "a", "an", "and", "around", "away", "back", "because", "between", "but",
    "by", "down", "for", "from", "here", "if", "in", "into", "like", "off",
    "on", "or", "out", "over", "so", "than", "the", "then", "there",
    "through", "to", "under", "up", "with", "without"
};

// Bank 5: Time & Place
const std::vector<std::string> BANK_5_TIME = {
    "after", "again", "always", "autumn", "before", "day", "evening",
    "forever", "hour", "moment", "morning", "never", "night", "now", "once",
    "season", "sometimes", "soon", "spring", "summer", "today", "tomorrow",
    "tonight", "when", "winter", "year", "yesterday"
};

// Bank 6: Emotions & States
const std::vector<std::string> BANK_6_EMOTIONS = {
    "bliss", "change", "chaos", "despair", "doubt", "escape", "faith",
    "freedom", "glory", "grace", "grief", "guilt", "joy", "madness",
    "magic", "mercy", "power", "rage", "shadow", "shame", "silence",
    "sorrow", "surrender", "trust", "wonder"
};

// Bank 7: Numbers & Quantities
const std::vector<std::string> BANK_7_NUMBERS = {
    "both", "eight", "first", "five", "four", "half", "hundred", "last",
    "million", "nine", "only", "seven", "six", "ten", "thousand", "three",
    "twice", "two", "zero"
};

// Bank 8: Questions & Interjections
const std::vector<std::string> BANK_8_QUESTIONS = {
    "ah", "even", "hey", "hmm", "how", "just", "maybe", "no", "oh", "ooh",
    "please", "really", "right", "sorry", "thanks", "uh", "well", "what",
    "where", "which", "who", "why", "wow", "wrong", "yes", "yet"
};

// All banks in order
const std::vector<const std::vector<std::string>*> ALL_BANKS = {
    &BANK_0_PRONOUNS,
    &BANK_1_VERBS,
    &BANK_2_DESCRIPTORS,
    &BANK_3_NOUNS,
    &BANK_4_CONNECTORS,
    &BANK_5_TIME,
    &BANK_6_EMOTIONS,
    &BANK_7_NUMBERS,
    &BANK_8_QUESTIONS
};

// Helper function to get bank index from voltage (0-9V maps to banks 0-8)
inline unsigned int voltageToBank(float voltage) {
    int bank = static_cast<int>(voltage);
    if (bank < 0) bank = 0;
    if (bank >= (int)NUM_BANKS) bank = NUM_BANKS - 1;
    return static_cast<unsigned int>(bank);
}

// Helper function to get word index within bank from normalized value (0-1)
inline unsigned int normalizedToWordIndex(float normalized, unsigned int bankSize) {
    if (bankSize == 0) return 0;
    unsigned int index = static_cast<unsigned int>(normalized * bankSize);
    if (index >= bankSize) index = bankSize - 1;
    return index;
}

} // namespace maya
