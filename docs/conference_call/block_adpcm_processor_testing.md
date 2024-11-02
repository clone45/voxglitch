Thank you for sharing both the data and the visual chart. This is indeed very helpful in understanding what's happening with the ADPCM compression and decompression process. Let's analyze what we're seeing:

Overall Shape: The general sine wave shape is preserved in the output, which is good. This means the ADPCM algorithm is capturing the fundamental frequency of the input signal.
Amplitude: The output wave reaches the full range of -1 to 1, matching the input. This indicates that the algorithm is maintaining the overall amplitude of the signal.
Phase Shift: There's a noticeable phase shift between the input and output. The output wave lags behind the input wave. This is likely due to the predictive nature of ADPCM and how it adapts to changes in the signal.
Distortion at the start: The output starts at 0 and then rapidly increases, not following the smooth start of the input sine wave. This is because ADPCM needs some samples to "warm up" and adjust its predictors and step sizes.
Flattening at peaks: The output wave tends to flatten at the peaks and troughs, especially visible around samples 13-17 and 39-50. This is a common artifact in ADPCM compression, often referred to as "slope overload".
Asymmetry: The positive half of the wave seems to be reproduced more accurately than the negative half. This could be due to how the algorithm handles positive vs. negative differences.
Quantization noise: The output appears more jagged than the smooth input sine wave. This is the result of quantization in the ADPCM process.

These observations suggest that while the ADPCM algorithm is functioning, there's room for improvement:

You might want to implement a "priming" mechanism to reduce the initial distortion.
The step size adaptation might need tuning to better handle rapid changes in the signal (to reduce slope overload).
The asymmetry in positive vs. negative handling could be investigated and potentially corrected.
You might experiment with different block sizes or adaptation rates to see if they improve overall fidelity.