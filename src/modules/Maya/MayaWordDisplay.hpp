#pragma once

struct MayaWordDisplay : TransparentWidget
{
	Maya *module;
	std::shared_ptr<Font> font;

	void draw(const DrawArgs &args) override
	{
		std::shared_ptr<Font> font = APP->window->loadFont(asset::plugin(pluginInstance, "res/fonts/ShareTechMono-Regular.ttf"));

		if (font)
		{
			nvgFontSize(args.vg, 14);
			nvgFontFaceId(args.vg, font->handle);
			nvgTextLetterSpacing(args.vg, 0);
		}

		std::string display_text = "";
		std::string count_text = "";

		if (module)
		{
			std::string word_name = module->getCurrentWordName();
			std::string bank_name = module->getCurrentBankName();
			unsigned int word_count = module->getWordCountInCurrentBank();

			if (word_count > 0)
			{
				display_text = word_name;
				// Truncate if too long
				if (display_text.length() > 12)
				{
					display_text = display_text.substr(0, 11) + "~";
				}

				// Show bank name and word index within bank
				count_text = bank_name + " " + std::to_string(module->selected_word_index + 1) + "/" + std::to_string(word_count);
			}
			else
			{
				display_text = "No words";
				count_text = "0/0";
			}
		}
		else
		{
			// Preview mode
			display_text = "hello";
			count_text = "1/48";
		}

		// Draw word name (large, centered)
		nvgFillColor(args.vg, nvgRGBA(150, 219, 234, 0xff));
		nvgTextAlign(args.vg, NVG_ALIGN_CENTER | NVG_ALIGN_MIDDLE);
		nvgText(args.vg, box.size.x / 2, box.size.y / 2 - 6, display_text.c_str(), NULL);

		// Draw count (smaller, below)
		if (font)
		{
			nvgFontSize(args.vg, 10);
		}
		nvgFillColor(args.vg, nvgRGBA(102, 151, 163, 0xff));
		nvgText(args.vg, box.size.x / 2, box.size.y / 2 + 10, count_text.c_str(), NULL);
	}
};
