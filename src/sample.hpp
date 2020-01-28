#pragma once

#include "dr_wav.h"

struct Sample
{
	std::string path;
	std::string filename;
	drwav_uint64 total_sample_count;
	bool loading;
	std::vector<float> leftPlayBuffer;
	std::vector<float> rightPlayBuffer;
	unsigned int sample_rate;
	unsigned int channels;
	bool loaded = false;

	Sample()
	{
		leftPlayBuffer.resize(0);
		rightPlayBuffer.resize(0);
		total_sample_count = 0;
		loading = false;
		filename = "[ empty ]";
		path = "";
		sample_rate = 0;
		channels = 0;
	}

	virtual ~Sample() {}

	virtual void load(std::string path, bool loadAsMono = true)
	{
		this->loading = true;

		unsigned int reported_channels;
		unsigned int reported_sample_rate;
		drwav_uint64 reported_total_sample_count;
		float *pSampleData;

		pSampleData = drwav_open_and_read_file_f32(path.c_str(), &reported_channels, &reported_sample_rate, &reported_total_sample_count);

		if (pSampleData != NULL)
		{
			// I'm aware that the "this" pointer isn't necessary here, but I wanted to include
			// it just to make the code as clear as possible.

			this->channels = reported_channels;
			this->sample_rate = reported_sample_rate;
			this->leftPlayBuffer.clear();
			this->rightPlayBuffer.clear();

			if(this->channels > 1 && ! loadAsMono)
			{
				for (unsigned int i=0; i < reported_total_sample_count; i = i + this->channels)
				{
					this->leftPlayBuffer.push_back(pSampleData[i]);
					this->rightPlayBuffer.push_back(pSampleData[i+1]);
				}
			}
			else
			{
				for (unsigned int i=0; i < reported_total_sample_count; i = i + this->channels)
				{
					this->leftPlayBuffer.push_back(pSampleData[i]);
				}
			}

			drwav_free(pSampleData);

			this->total_sample_count = leftPlayBuffer.size();
			this->filename = rack::string::filename(path);
			this->path = path;

			this->loading = false;
			this->loaded = true;
		}
		else
		{
			this->loading = false;
		}
	};
};
