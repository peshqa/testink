#pragma once

#include <iostream>
#include <fstream>

#pragma pack(push, 1)
typedef struct
{
	char chunkID[4];
	unsigned int chunkSize;
	char format[4];
	char subchunk1ID[4];
	unsigned int subchunk1Size;
	unsigned short audioFormat;
	unsigned short numChannels;
	unsigned int sampleRate;
	unsigned int byteRate;
	unsigned short blockAlign;
	unsigned short bitsPerSample;
	char subchunk2ID[4];
	unsigned int subchunk2Size;
} WAVHeader;
#pragma pack(pop)

int LoadWavFile(float *&samples, int &num_samples)
{
	std::ifstream file("C:\\Users\\Pavel\\Desktop\\tf2_hitsounds\\sound\\ui\\hitsound_quake2.wav");
	
	if (!file)
	{
		return 1;
	}
	
	WAVHeader header{};
	
	file.read((char*)&header, sizeof(WAVHeader));
	
	if (std::string(header.chunkID, 4) != "RIFF" ||
		std::string(header.format, 4) != "WAVE" || 
		std::string(header.subchunk1ID, 4) != "fmt " ||
		std::string(header.subchunk2ID, 4) != "data")
	{
		return 2;
	}
	
	if (header.audioFormat != 3)
	{
		return 3;
	}
	
	num_samples = header.subchunk2Size / sizeof(float) / 32;
	samples = new float[num_samples]; // TODO: dont forget delete []
	
	file.read((char*)samples, header.subchunk2Size);
	/*for (int i = 0; i < num_samples; i++)
	{
		char tmp = ((char*)samples)[i*4+0];
		((char*)samples)[i*4+0] = ((char*)samples)[i*4+3];
		((char*)samples)[i*4+3] = tmp;
		tmp = ((char*)samples)[i*4+1];
		((char*)samples)[i*4+1] = ((char*)samples)[i*4+2];
		((char*)samples)[i*4+2] = tmp;
	}*/
	
	return 0;
}