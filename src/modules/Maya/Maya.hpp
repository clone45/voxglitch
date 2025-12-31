#pragma once

#include "banks.h"
#include "VocabularyLoader.hpp"
#include "FadeEnvelope.hpp"

struct Maya : VoxglitchSamplerModule
{
	unsigned int selected_bank = 0;
	unsigned int selected_word_index = 0;  // Index within current bank
	std::string path;
	bool loading_samples = false;

	maya::Vocabulary vocabulary;  // Loaded vocabulary (from JSON or defaults)

	std::vector<SamplePlayer> sample_players;
	std::vector<std::string> word_names;  // Store word names for display
	std::map<std::string, unsigned int> word_to_sample_index;  // Map word name to sample index

	dsp::SchmittTrigger playTrigger;
	FadeEnvelope fade_envelope;

	bool playback = false;
	unsigned int current_sample_index = 0;  // Actual index into sample_players

	// Pitch CV mode: 0 = bipolar (-5V to +5V), 1 = unipolar (0-10V)
	int pitch_cv_mode = 0;

	// Sampled pitch value (captured at trigger time)
	float current_pitch = 0.0f;

	enum ParamIds
	{
		WORD_KNOB,
		WORD_ATTN_KNOB,
		BANK_KNOB,
		BANK_ATTN_KNOB,
		NUM_PARAMS
	};

	enum InputIds
	{
		TRIG_INPUT,
		WORD_INPUT,
		BANK_INPUT,
		PITCH_INPUT,
		NUM_INPUTS
	};

	enum OutputIds
	{
		AUDIO_LEFT_OUTPUT,
		AUDIO_RIGHT_OUTPUT,
		NUM_OUTPUTS
	};

	enum LightIds
	{
		NUM_LIGHTS
	};

	//
	// Constructor
	//
	Maya()
	{
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
		configParam(WORD_KNOB, 0.0f, 1.0f, 0.0f, "Word Select");
		configParam(WORD_ATTN_KNOB, 0.0f, 1.0f, 1.0f, "Word CV Attenuator");
		configParam(BANK_KNOB, 0.0f, 1.0f, 0.0f, "Bank Select");
		configParam(BANK_ATTN_KNOB, 0.0f, 1.0f, 1.0f, "Bank CV Attenuator");
		configInput(TRIG_INPUT, "Trigger");
		configInput(WORD_INPUT, "Word Selection CV");
		configInput(BANK_INPUT, "Bank Selection CV (0-9V)");
		configInput(PITCH_INPUT, "Pitch (-5V to +5V = ±1 octave)");
		configOutput(AUDIO_LEFT_OUTPUT, "Left Audio");
		configOutput(AUDIO_RIGHT_OUTPUT, "Right Audio");

		// Initialize fade envelope with default sample rate
		fade_envelope.setSampleRate(44100.0f);
	}

	// Save
	json_t *dataToJson() override
	{
		json_t *json_root = json_object();
		json_object_set_new(json_root, "path", json_string(this->path.c_str()));
		json_object_set_new(json_root, "pitch_cv_mode", json_integer(pitch_cv_mode));
		saveSamplerData(json_root);
		return json_root;
	}

	// Load
	void dataFromJson(json_t *json_root) override
	{
		json_t *loaded_path_json = json_object_get(json_root, "path");
		if (loaded_path_json)
		{
			this->path = json_string_value(loaded_path_json);
			this->load_samples_from_path(this->path);
		}

		json_t *pitch_cv_mode_json = json_object_get(json_root, "pitch_cv_mode");
		if (pitch_cv_mode_json) pitch_cv_mode = json_integer_value(pitch_cv_mode_json);

		loadSamplerData(json_root);
	}

	// Called when module is added to the rack
	void onAdd(const AddEvent& e) override
	{
		// Load vocabulary first (from JSON or fallback to defaults)
		std::string vocab_path = asset::plugin(pluginInstance, "res/modules/maya/vocabulary.json");
		if (!maya::loadVocabularyFromJson(vocab_path, vocabulary)) {
			maya::buildDefaultVocabulary(vocabulary);
		}

		// Load default samples if no samples have been loaded yet
		if (sample_players.empty() && path.empty())
		{
			std::string default_path = asset::plugin(pluginInstance, "res/modules/maya/samples");
			this->path = default_path;
			this->load_samples_from_path(default_path);
		}
	}

	void onSampleRateChange(const SampleRateChangeEvent& e) override
	{
		fade_envelope.setSampleRate(e.sampleRate);

		// Update all sample players' step amounts for the new sample rate
		for (auto& player : sample_players)
		{
			player.updateSampleRate();
		}
	}

	void load_samples_from_path(std::string path)
	{
		loading_samples = true;

		// Clear out any old samples
		this->sample_players.clear();
		this->word_names.clear();
		this->word_to_sample_index.clear();

		// Load all .wav and .mp3 files found in the folder specified by 'path'
		std::vector<std::string> dirList = system::getEntries(path);

		// Sort alphabetically
		sort(dirList.begin(), dirList.end());

		unsigned int sample_index = 0;
		for (auto file_path : dirList)
		{
			std::string ext = rack::string::lowercase(system::getExtension(file_path));
			if (ext == "wav" || ext == ".wav" || ext == "mp3" || ext == ".mp3")
			{
				SamplePlayer sample_player;
				sample_player.loadSample(file_path);
				this->sample_players.push_back(sample_player);

				// Extract word name from filename (without extension)
				std::string filename = system::getFilename(file_path);
				std::string word_name = filename.substr(0, filename.find_last_of('.'));
				this->word_names.push_back(word_name);

				// Build lookup map
				this->word_to_sample_index[word_name] = sample_index;
				sample_index++;
			}
		}

		loading_samples = false;
	}

	// Get the sample index for a word in a bank
	// Returns -1 if word not found
	int getSampleIndexForBankWord(unsigned int bank, unsigned int word_index)
	{
		if (bank >= vocabulary.banks.size()) return -1;

		const maya::VocabularyBank& vbank = vocabulary.banks[bank];
		if (word_index >= vbank.words.size()) return -1;

		// Use file name (not display) for sample lookup
		const std::string& filename = vbank.words[word_index].file;
		auto it = word_to_sample_index.find(filename);
		if (it != word_to_sample_index.end())
		{
			return static_cast<int>(it->second);
		}
		return -1;
	}

	std::string getCurrentWordName()
	{
		if (current_sample_index < word_names.size())
		{
			return word_names[current_sample_index];
		}
		return "";
	}

	std::string getCurrentWordDisplayName()
	{
		if (selected_bank >= vocabulary.banks.size()) return "";
		const maya::VocabularyBank& bank = vocabulary.banks[selected_bank];
		if (selected_word_index >= bank.words.size()) return "";
		return bank.words[selected_word_index].display;
	}

	std::string getCurrentBankName()
	{
		if (selected_bank < vocabulary.banks.size())
		{
			return vocabulary.banks[selected_bank].name;
		}
		return "";
	}

	unsigned int getWordCountInCurrentBank()
	{
		if (selected_bank >= vocabulary.banks.size()) return 0;
		return vocabulary.banks[selected_bank].words.size();
	}

	unsigned int getWordCount()
	{
		return sample_players.size();
	}

	float calculate_inputs(int input_index, int knob_index, int attenuator_index, float scale)
	{
		float input_value = inputs[input_index].getVoltage() / 10.0;
		float knob_value = params[knob_index].getValue();
		float attenuator_value = params[attenuator_index].getValue();

		return (((input_value * scale) * attenuator_value) + (knob_value * scale));
	}

	void process(const ProcessArgs &args) override
	{
		// If the samples are being loaded, don't do anything
		if (loading_samples)
			return;

		if (sample_players.empty())
			return;

		// Calculate bank selection (0-9V maps to banks 0-9)
		unsigned int num_banks = vocabulary.banks.size();
		float bank_cv = 0.0f;
		if (inputs[BANK_INPUT].isConnected())
		{
			bank_cv = inputs[BANK_INPUT].getVoltage() * params[BANK_ATTN_KNOB].getValue();
		}
		float bank_knob = params[BANK_KNOB].getValue() * (num_banks > 0 ? num_banks - 1 : 0);
		unsigned int new_bank = maya::voltageToBank(bank_cv + bank_knob);
		if (new_bank >= num_banks && num_banks > 0) new_bank = num_banks - 1;

		// Calculate word selection within bank (0-1 normalized)
		float word_cv = 0.0f;
		if (inputs[WORD_INPUT].isConnected())
		{
			word_cv = (inputs[WORD_INPUT].getVoltage() / 10.0f) * params[WORD_ATTN_KNOB].getValue();
		}
		float word_knob = params[WORD_KNOB].getValue();
		float word_normalized = clamp(word_cv + word_knob, 0.0f, 1.0f);

		// Get word count in the selected bank
		unsigned int bank_word_count = 0;
		if (new_bank < num_banks)
		{
			bank_word_count = vocabulary.banks[new_bank].words.size();
		}

		unsigned int new_word_index = maya::normalizedToWordIndex(word_normalized, bank_word_count);

		// Get the actual sample index for this bank/word combination
		int new_sample_index = getSampleIndexForBankWord(new_bank, new_word_index);

		// If word not found in samples, try to find any valid sample
		if (new_sample_index < 0)
		{
			new_sample_index = 0;
		}

		// Update display selection (what the knobs are showing)
		selected_bank = new_bank;
		selected_word_index = new_word_index;

		// Handle trigger input - this is when we actually change the playing sample
		if (inputs[TRIG_INPUT].isConnected())
		{
			if (playTrigger.process(inputs[TRIG_INPUT].getVoltage(), constants::gate_low_trigger, constants::gate_high_trigger))
			{
				// Sample pitch at trigger time (before anything else)
				float pitch_voltage = inputs[PITCH_INPUT].getVoltage();
				if (pitch_cv_mode == 0)
				{
					// Bipolar: -5V to +5V maps to -0.5 to +0.5 octaves
					current_pitch = clamp(pitch_voltage / 10.0f, -0.5f, 0.5f);
				}
				else
				{
					// Unipolar: 0-10V maps to -0.5 to +0.5 octaves (5V = center)
					current_pitch = clamp((pitch_voltage - 5.0f) / 10.0f, -0.5f, 0.5f);
				}

				// Stop current sample if different
				if ((unsigned int)new_sample_index != current_sample_index && current_sample_index < sample_players.size())
				{
					sample_players[current_sample_index].stop();
				}

				// Update to new sample
				current_sample_index = static_cast<unsigned int>(new_sample_index);

				// Check bounds and trigger
				if (current_sample_index < sample_players.size())
				{
					playback = true;
					fade_envelope.trigger();
					sample_players[current_sample_index].trigger(0.0f, false);
				}
			}
		}

		// Check to see if the selected sample slot refers to an existing sample
		if (current_sample_index >= sample_players.size())
			return;

		SamplePlayer *selected_sample_player = &sample_players[current_sample_index];

		// Check if sample just finished playing
		if (playback && !selected_sample_player->playing)
		{
			fade_envelope.release();
			playback = false;
		}

		// Process audio with fade envelope
		float gain = fade_envelope.process();

		if (fade_envelope.isActive() && selected_sample_player->playing)
		{
			float left_audio;
			float right_audio;

			selected_sample_player->getStereoOutput(&left_audio, &right_audio, interpolation);

			outputs[AUDIO_LEFT_OUTPUT].setVoltage(left_audio * gain * GAIN);
			outputs[AUDIO_RIGHT_OUTPUT].setVoltage(right_audio * gain * GAIN);

			// Use the pitch that was sampled at trigger time
			selected_sample_player->step(current_pitch, 0.0f, 1.0f, false);
		}
		else
		{
			// Output silence when not playing
			outputs[AUDIO_LEFT_OUTPUT].setVoltage(0.0f);
			outputs[AUDIO_RIGHT_OUTPUT].setVoltage(0.0f);
		}
	}
};
