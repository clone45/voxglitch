ADPCM (Adaptive Differential Pulse Code Modulation) communication systems, like those used in VoIP or digital telephony, often employ various techniques to handle dropouts or packet loss. These techniques aim to maintain audio quality and intelligibility even when some data is lost. Here are some common approaches:

== Packet Loss Concealment (PLC):

This is a general term for techniques that try to "fill in" missing audio data.
Simple PLC might repeat the last known good frame of audio.
More advanced PLC might use interpolation or prediction based on surrounding audio data.

Some common PLC techniques include:

* Waveform substitution: Repeating the last known good frame.
* Pitch waveform replication: Repeating the last pitch period of audio.
* Time-scale modification: Stretching the surrounding audio to fill the gap.
* Model-based interpolation: Using speech models to generate likely missing data.


=== Forward Error Correction (FEC):

Extra data is sent along with the primary audio data to help reconstruct lost packets.
This increases bandwidth usage but can significantly improve quality in lossy networks.


Interleaving:

Audio data is mixed up before transmission and reassembled at the receiver.
This spreads the effect of packet loss over time, making it less noticeable.


Adaptive Playback:

The receiver adjusts its playback speed slightly to compensate for jitter and lost packets.
This can help maintain synchronization and reduce perceived gaps in audio.


Redundant Transmissions:

Critical audio data (like the start of words) might be sent multiple times.
This increases the chance that important audio segments are received.


Error Resilient Encoding:

The ADPCM encoder might be designed to limit error propagation.
For example, periodically resetting the predictor state to prevent long-term drift due to errors.


Silence Detection and Comfort Noise Generation:

The system detects silence in the audio and doesn't transmit it.
The receiver generates "comfort noise" during silent periods, which can mask minor dropouts.


Multiple Description Coding:

The audio is encoded into multiple, independently decodable streams.
Even if some streams are lost, the others can still produce acceptable audio quality.


Adaptive Buffer Management:

The receiver dynamically adjusts its buffer size based on network conditions.
This can help smooth out jitter and minor packet loss at the cost of increased latency.


Packet Loss Notification and Retransmission:

In some systems, the receiver might request retransmission of lost packets.
This is more common in non-real-time applications due to added latency.