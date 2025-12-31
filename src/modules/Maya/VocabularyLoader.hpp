#pragma once

#include <string>
#include <vector>
#include <jansson.h>
#include "banks.h"

namespace maya {

struct VocabularyWord {
    std::string display;  // What to show on LCD
    std::string file;     // Filename without extension
};

struct VocabularyBank {
    std::string name;
    std::vector<VocabularyWord> words;
};

struct Vocabulary {
    std::vector<VocabularyBank> banks;
    bool loaded_from_json = false;

    unsigned int getNumBanks() const {
        return banks.size();
    }

    unsigned int getWordCount(unsigned int bank) const {
        if (bank >= banks.size()) return 0;
        return banks[bank].words.size();
    }

    std::string getBankName(unsigned int bank) const {
        if (bank >= banks.size()) return "";
        return banks[bank].name;
    }

    std::string getWordDisplay(unsigned int bank, unsigned int word_index) const {
        if (bank >= banks.size()) return "";
        if (word_index >= banks[bank].words.size()) return "";
        return banks[bank].words[word_index].display;
    }

    std::string getWordFile(unsigned int bank, unsigned int word_index) const {
        if (bank >= banks.size()) return "";
        if (word_index >= banks[bank].words.size()) return "";
        return banks[bank].words[word_index].file;
    }
};

// Load vocabulary from JSON file
// Returns true if successful, false if should use defaults
inline bool loadVocabularyFromJson(const std::string& json_path, Vocabulary& vocab) {
    FILE* file = fopen(json_path.c_str(), "r");
    if (!file) {
        WARN("Maya: vocabulary.json not found at %s, using defaults", json_path.c_str());
        return false;
    }

    json_error_t error;
    json_t* root = json_loadf(file, 0, &error);
    fclose(file);

    if (!root) {
        WARN("Maya: Failed to parse vocabulary.json: %s", error.text);
        return false;
    }

    json_t* banks_array = json_object_get(root, "banks");
    if (!banks_array || !json_is_array(banks_array)) {
        WARN("Maya: vocabulary.json missing 'banks' array");
        json_decref(root);
        return false;
    }

    vocab.banks.clear();

    size_t bank_index;
    json_t* bank_obj;
    json_array_foreach(banks_array, bank_index, bank_obj) {
        if (!json_is_object(bank_obj)) continue;

        VocabularyBank bank;

        json_t* name_json = json_object_get(bank_obj, "name");
        if (name_json && json_is_string(name_json)) {
            bank.name = json_string_value(name_json);
        } else {
            continue;  // Skip banks without names
        }

        json_t* words_array = json_object_get(bank_obj, "words");
        if (words_array && json_is_array(words_array)) {
            size_t word_index;
            json_t* word_obj;
            json_array_foreach(words_array, word_index, word_obj) {
                if (!json_is_object(word_obj)) continue;

                VocabularyWord word;

                json_t* display_json = json_object_get(word_obj, "display");
                json_t* file_json = json_object_get(word_obj, "file");

                if (display_json && json_is_string(display_json)) {
                    word.display = json_string_value(display_json);
                }
                if (file_json && json_is_string(file_json)) {
                    word.file = json_string_value(file_json);
                }

                // Only add if we have both display and file
                if (!word.display.empty() && !word.file.empty()) {
                    bank.words.push_back(word);
                }
            }
        }

        vocab.banks.push_back(bank);
    }

    json_decref(root);
    vocab.loaded_from_json = true;
    return true;
}

// Build vocabulary from hardcoded banks.h (fallback)
inline void buildDefaultVocabulary(Vocabulary& vocab) {
    vocab.banks.clear();
    vocab.loaded_from_json = false;

    for (unsigned int i = 0; i < NUM_BANKS; i++) {
        VocabularyBank bank;
        bank.name = BANK_NAMES[i];

        const std::vector<std::string>* words = ALL_BANKS[i];
        for (const std::string& w : *words) {
            VocabularyWord vw;
            vw.display = w;
            vw.file = w;
            bank.words.push_back(vw);
        }

        vocab.banks.push_back(bank);
    }
}

} // namespace maya
