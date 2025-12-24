#pragma once

#include <fstream>
#include "MayaIntroAnimation.hpp"

struct MayaWordDisplay : TransparentWidget
{
	Maya *module;
	std::shared_ptr<Font> font;
	MayaIntroAnimation introAnimation;

	// Static storage for preview pixels (module browser only)
	static std::vector<std::pair<float, float>>& getPreviewPixels()
	{
		static std::vector<std::pair<float, float>> pixels;
		static bool loaded = false;

		if (!loaded)
		{
			loaded = true;
			std::string bin_path = asset::plugin(pluginInstance, "res/modules/maya/maya_display_pixels.bin");
			std::ifstream file(bin_path, std::ios::binary);

			if (file.is_open())
			{
				// Read header: width, height
				uint8_t srcWidth, srcHeight;
				file.read(reinterpret_cast<char*>(&srcWidth), 1);
				file.read(reinterpret_cast<char*>(&srcHeight), 1);

				// Read packed pixel data
				std::vector<uint8_t> packedData((srcWidth * srcHeight + 7) / 8);
				file.read(reinterpret_cast<char*>(packedData.data()), packedData.size());
				file.close();

				// Scale factors
				float dstWidth = 83.0f;   // 85 - 2px margin
				float dstHeight = 29.0f;  // 31 - 2px margin
				float scaleX = dstWidth / (float)(srcWidth - 1);
				float scaleY = dstHeight / (float)(srcHeight - 1);

				// Unpack and scale pixels
				for (int y = 0; y < srcHeight; y++)
				{
					for (int x = 0; x < srcWidth; x++)
					{
						int bitIndex = y * srcWidth + x;
						int byteIndex = bitIndex / 8;
						int bitOffset = 7 - (bitIndex % 8);
						if (packedData[byteIndex] & (1 << bitOffset))
						{
							float nx = 1.0f + x * scaleX;
							float ny = 1.0f + y * scaleY;
							pixels.push_back({nx, ny});
						}
					}
				}
			}
		}
		return pixels;
	}

	void draw(const DrawArgs &args) override
	{
		// Handle intro animation when module is active
		if (module)
		{
			// getFrameTime() returns delta time (seconds since last frame)
			float deltaTime = APP->window->getFrameTime();
			// Clamp to reasonable value
			if (deltaTime <= 0 || deltaTime > 0.1f) deltaTime = 1.0f / 60.0f;
			// Initialize animation on first draw with module
			if (!introAnimation.initialized)
			{
				introAnimation.initialize(box.size.x, box.size.y);
			}

			// If animation is still running, draw it and return
			if (!introAnimation.isComplete())
			{
				introAnimation.update(deltaTime);
				introAnimation.draw(args.vg, box.size.x, box.size.y);
				return;
			}

			// Animation complete - draw normal display
			drawNormalDisplay(args);
		}
		else
		{
			// Preview mode (module browser) - draw static pixels
			drawPreviewPixels(args);
		}
	}

	void drawNormalDisplay(const DrawArgs &args)
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

		std::string word_name = module->getCurrentWordName();
		std::string bank_name = module->getCurrentBankName();
		unsigned int word_count = module->getWordCountInCurrentBank();

		if (word_count > 0)
		{
			display_text = word_name;
			if (display_text.length() > 12)
			{
				display_text = display_text.substr(0, 11) + "~";
			}
			count_text = bank_name + " " + std::to_string(module->selected_word_index + 1) + "/" + std::to_string(word_count);
		}
		else
		{
			display_text = "No words";
			count_text = "0/0";
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

	void drawPreviewPixels(const DrawArgs &args)
	{
		const auto& pixels = getPreviewPixels();
		if (!pixels.empty())
		{
			nvgFillColor(args.vg, nvgRGBA(150, 219, 234, 0xff));
			for (const auto& p : pixels)
			{
				nvgBeginPath(args.vg);
				nvgRect(args.vg, p.first, p.second, 1.0f, 1.0f);
				nvgFill(args.vg);
			}
			return;
		}

		// Fallback if no pixels loaded
		std::shared_ptr<Font> font = APP->window->loadFont(asset::plugin(pluginInstance, "res/fonts/ShareTechMono-Regular.ttf"));
		if (font)
		{
			nvgFontSize(args.vg, 14);
			nvgFontFaceId(args.vg, font->handle);
			nvgTextLetterSpacing(args.vg, 0);
		}

		nvgFillColor(args.vg, nvgRGBA(150, 219, 234, 0xff));
		nvgTextAlign(args.vg, NVG_ALIGN_CENTER | NVG_ALIGN_MIDDLE);
		nvgText(args.vg, box.size.x / 2, box.size.y / 2 - 6, "hello", NULL);

		if (font) nvgFontSize(args.vg, 10);
		nvgFillColor(args.vg, nvgRGBA(102, 151, 163, 0xff));
		nvgText(args.vg, box.size.x / 2, box.size.y / 2 + 10, "1/48", NULL);
	}
};
