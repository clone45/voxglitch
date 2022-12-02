/** Limits the derivative of the output by a rise and fall speed, in units/s. */
template <typename T = float>
struct TFastSlewLimiter {
	T out = 0.f;
	T rise = 0.f;
	T fall = 0.f;
    T deltaTime = 0.f;
    T fallDelta = 0.f;
    T riseDelta = 0.f;

	void reset() {
		out = 0.f;
	}

	void setRiseFall(T rise, T fall) {
		this->rise = rise;
		this->fall = fall;
	}

	void setDeltaTime(T deltaTime) {
		this->deltaTime = deltaTime;
        this->fallDelta = fall * deltaTime;
        this->riseDelta = rise * deltaTime;
	}

	T process(T in) {
        if(in == out) return out;
		out = simd::clamp(in, out - fallDelta, out + riseDelta);
		return out;
	}

    void updateRackSampleRate()
    {
      setDeltaTime(APP->engine->getSampleTime());
    }    
};

typedef TFastSlewLimiter<> FastSlewLimiter;