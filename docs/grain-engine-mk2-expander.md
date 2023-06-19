## Grain Engine MK2 Expander

<img src="images\grain-engine-mk2-expander/grain-engine-mk2-expander-front-panel.png" alt="" style="zoom:50%;" />

Grain Engine MK2 Expander (aka GEMK2/se) is an expansion module to Grain Engine MK2.  To activate this module, it must be placed to the left of Grain Engine MK2:

<img src="images\grain-engine-mk2-expander/gemk2es-positioning.png" alt="" style="zoom:50%;" />

GEMK2/se starts recording audio at INPUTS when START REC has been triggered.  When recording, the recording light will be red.  To stop recording, trigger either the START REC input or the STOP REC input.

When recording is stopped, the audio information is passed to Grain Engine MK2 and stored in one of the sample slots.  Which slot it's stored in (1 - 5) is controlled by the GEMK2/se's SAMPLE input.

What's really happening under the hood is that GEMK2/se is writing the recorded audio to a file and passing the filename to Grain Engine MK2 for loading.  You can see the filename of the saved audio by opening the right-click context menu:

<img src="images\grain-engine-mk2-expander/filename.png" alt="" style="zoom:50%;" />

These samples are stored in a folder named "gemk2es_audio_files" that's created in your Rack folder.  



## Troubleshooting

> No lights on the front panel are lit and the module isn't doing anything.  

Ensure that the module is placed immediately to the left of Grain Engine MK2

> Everything look like it's hooked up correctly, but I'm not getting any audio from Grain Engine MK2

Be sure that the SAMPLE inputs are aligned on both the Grain Engine MK2 and the expansion module.  Try turning them both fully to the left so that sample slot #1 is selected on both.  Also listen to the THROUGH outputs of the expansion module to ensure that it's receiving audio at INPUTS.