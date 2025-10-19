#pragma once

// Base interface for excitation sources in Karplus-Strong synthesis
// Different excitation methods (noise, bytebeat, external audio, etc.)
// can implement this interface
struct ExcitationSource
{
    // Generate excitation signal into the provided buffer
    // buffer: output buffer to fill with excitation signal
    // length: number of samples to generate
    virtual void generate(float *buffer, int length) = 0;

    // Virtual destructor for proper cleanup
    virtual ~ExcitationSource() {}
};
