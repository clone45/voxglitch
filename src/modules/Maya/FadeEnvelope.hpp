#pragma once

struct FadeEnvelope
{
	enum State { IDLE, FADE_IN, SUSTAIN, FADE_OUT };

	State state = IDLE;
	float gain = 0.0f;
	float fade_increment = 0.0f;
	float fade_time_ms = 5.0f;  // 5ms fade by default

	void setSampleRate(float sample_rate)
	{
		float fade_samples = (fade_time_ms / 1000.0f) * sample_rate;
		fade_increment = 1.0f / fade_samples;
	}

	void trigger()
	{
		state = FADE_IN;
		gain = 0.0f;
	}

	void release()
	{
		if (state == SUSTAIN || state == FADE_IN)
		{
			state = FADE_OUT;
		}
	}

	bool isActive()
	{
		return state != IDLE;
	}

	float process()
	{
		switch (state)
		{
			case FADE_IN:
				gain += fade_increment;
				if (gain >= 1.0f)
				{
					gain = 1.0f;
					state = SUSTAIN;
				}
				break;

			case FADE_OUT:
				gain -= fade_increment;
				if (gain <= 0.0f)
				{
					gain = 0.0f;
					state = IDLE;
				}
				break;

			case SUSTAIN:
			case IDLE:
			default:
				break;
		}

		return gain;
	}
};
