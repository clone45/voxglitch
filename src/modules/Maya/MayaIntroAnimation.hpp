#pragma once

#include <fstream>
#include <cmath>
#include <algorithm>

struct MayaIntroAnimation
{
	struct Pixel
	{
		// Original 2D position (centered around 0,0)
		float srcX, srcY;
		// Current screen position
		float x, y;
		// Depth (for perspective)
		float z;
		// Alpha for fade effect
		float alpha;
	};

	std::vector<Pixel> pixels;
	bool animationComplete = false;
	bool initialized = false;
	float elapsedTime = 0.0f;
	float displayWidth = 0.0f;
	float displayHeight = 0.0f;

	// Animation timing
	static constexpr float SPIN_IN_DURATION = 0.6f;   // Y-axis spin entrance
	static constexpr float HOLD_DURATION = 1.0f;      // Hold fully visible
	static constexpr float FADE_DURATION = 1.2f;      // Tilt + dissolve exit

	// 3D settings
	static constexpr float PERSPECTIVE_DISTANCE = 80.0f;   // Camera distance (lower = more dramatic)
	static constexpr float START_Y_ROTATION = M_PI / 2.0f; // Start rotated 90° (edge-on)
	static constexpr float EXIT_X_ROTATION = M_PI / 3.0f;  // Tilt back 60° during exit

	void rotatePoint(float* x, float* y, float* z, float alpha, float beta, float gamma)
	{
		// Rotate around X (alpha)
		float y1 = *y * std::cos(alpha) - *z * std::sin(alpha);
		float z1 = *y * std::sin(alpha) + *z * std::cos(alpha);
		*y = y1;
		*z = z1;

		// Rotate around Y (beta)
		float x1 = *x * std::cos(beta) + *z * std::sin(beta);
		float z2 = -*x * std::sin(beta) + *z * std::cos(beta);
		*x = x1;
		*z = z2;

		// Rotate around Z (gamma)
		float x2 = *x * std::cos(gamma) - *y * std::sin(gamma);
		float y2 = *x * std::sin(gamma) + *y * std::cos(gamma);
		*x = x2;
		*y = y2;
	}

	// Easing function for smooth animation
	float easeOutCubic(float t)
	{
		return 1.0f - std::pow(1.0f - t, 3.0f);
	}

	float easeInCubic(float t)
	{
		return t * t * t;
	}

	void initialize(float width, float height)
	{
		if (initialized) return;
		initialized = true;
		displayWidth = width;
		displayHeight = height;

		// Load pixels from binary file
		std::string bin_path = asset::plugin(pluginInstance, "res/modules/maya/maya_display_pixels.bin");
		std::ifstream file(bin_path, std::ios::binary);

		if (!file.is_open()) return;

		// Read header: width, height (2 bytes)
		uint8_t srcWidth, srcHeight;
		file.read(reinterpret_cast<char*>(&srcWidth), 1);
		file.read(reinterpret_cast<char*>(&srcHeight), 1);

		// Read packed pixel data
		std::vector<uint8_t> packedData((srcWidth * srcHeight + 7) / 8);
		file.read(reinterpret_cast<char*>(packedData.data()), packedData.size());
		file.close();

		// Unpack pixels
		std::vector<std::pair<float, float>> rawPixels;
		for (int y = 0; y < srcHeight; y++)
		{
			for (int x = 0; x < srcWidth; x++)
			{
				int bitIndex = y * srcWidth + x;
				int byteIndex = bitIndex / 8;
				int bitOffset = 7 - (bitIndex % 8);  // MSB first
				if (packedData[byteIndex] & (1 << bitOffset))
				{
					rawPixels.push_back({(float)x, (float)y});
				}
			}
		}

		if (rawPixels.empty()) return;

		// Scale to display size with margin
		float dstWidth = width - 2.0f;
		float dstHeight = height - 2.0f;
		float scaleX = dstWidth / (float)(srcWidth - 1);
		float scaleY = dstHeight / (float)(srcHeight - 1);

		float centerX = width / 2.0f;
		float centerY = height / 2.0f;

		for (const auto& p : rawPixels)
		{
			Pixel pixel;
			// Scale to screen coordinates
			float screenX = 1.0f + p.first * scaleX;
			float screenY = 1.0f + p.second * scaleY;

			// Store centered coordinates for rotation
			pixel.srcX = screenX - centerX;
			pixel.srcY = screenY - centerY;
			pixel.x = screenX;
			pixel.y = screenY;
			pixel.z = 0.0f;
			pixel.alpha = 255.0f;
			pixels.push_back(pixel);
		}
	}

	// Returns true if animation is still running
	bool update(float deltaTime)
	{
		if (animationComplete || pixels.empty()) return false;

		elapsedTime += deltaTime;

		float centerX = displayWidth / 2.0f;
		float centerY = displayHeight / 2.0f;

		// Phase 1: Spin in (Y-axis rotation)
		if (elapsedTime < SPIN_IN_DURATION)
		{
			float t = elapsedTime / SPIN_IN_DURATION;
			float easedT = easeOutCubic(t);
			float yRotation = START_Y_ROTATION * (1.0f - easedT); // 90° -> 0°

			for (auto& pixel : pixels)
			{
				float x = pixel.srcX;
				float y = pixel.srcY;
				float z = 0.0f;

				rotatePoint(&x, &y, &z, 0.0f, yRotation, 0.0f);

				// Apply perspective projection
				float scale = PERSPECTIVE_DISTANCE / (PERSPECTIVE_DISTANCE + z);
				pixel.x = centerX + x * scale;
				pixel.y = centerY + y * scale;
				pixel.z = z;

				// Fade in based on z (pixels facing away are dimmer)
				float depthFade = std::max(0.0f, std::min(1.0f, (z + 50.0f) / 100.0f));
				pixel.alpha = 255.0f * easedT * (0.3f + 0.7f * depthFade);
			}
			return true;
		}

		// Phase 2: Hold (fully visible, no rotation)
		float holdStart = SPIN_IN_DURATION;
		if (elapsedTime < holdStart + HOLD_DURATION)
		{
			// Ensure all pixels are at full visibility and correct position
			for (auto& pixel : pixels)
			{
				pixel.x = centerX + pixel.srcX;
				pixel.y = centerY + pixel.srcY;
				pixel.z = 0.0f;
				pixel.alpha = 255.0f;
			}
			return true;
		}

		// Phase 3: Simple fade out
		float fadeStart = holdStart + HOLD_DURATION;
		float fadeTime = elapsedTime - fadeStart;
		float fadeDuration = 0.5f;  // Half second fade

		if (fadeTime < fadeDuration)
		{
			float alpha = 255.0f * (1.0f - fadeTime / fadeDuration);
			for (auto& pixel : pixels)
			{
				pixel.x = centerX + pixel.srcX;
				pixel.y = centerY + pixel.srcY;
				pixel.alpha = alpha;
			}
			return true;
		}

		animationComplete = true;
		return false;
	}

	void draw(NVGcontext* vg, float width, float height)
	{
		if (animationComplete || pixels.empty()) return;

		// Draw background
		nvgBeginPath(vg);
		nvgRect(vg, 0, 0, width, height);
		nvgFillColor(vg, nvgRGBA(20, 40, 50, 255));
		nvgFill(vg);

		// Sort by z for proper depth ordering (back to front)
		std::vector<Pixel*> sortedPixels;
		for (auto& pixel : pixels)
		{
			if (pixel.alpha > 0)
			{
				sortedPixels.push_back(&pixel);
			}
		}
		std::sort(sortedPixels.begin(), sortedPixels.end(),
			[](const Pixel* a, const Pixel* b) { return a->z > b->z; });

		for (const auto* pixel : sortedPixels)
		{
			nvgBeginPath(vg);
			nvgRect(vg, pixel->x, pixel->y, 1.0f, 1.0f);
			nvgFillColor(vg, nvgRGBA(150, 219, 234, (int)pixel->alpha));
			nvgFill(vg);
		}
	}

	bool isComplete() const
	{
		return animationComplete || (initialized && pixels.empty());
	}

	void reset()
	{
		elapsedTime = 0.0f;
		animationComplete = false;
		for (auto& pixel : pixels)
		{
			pixel.alpha = 255.0f;
		}
	}
};
