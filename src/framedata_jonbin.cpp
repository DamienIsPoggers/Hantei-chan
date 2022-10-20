#include "framedata.h"
#include "cg.h"
#include "main_frame.h"
#include <fstream>
#include <cstdint>

#define VAL(X) ((const char*)&X)
#define PTR(X) ((const char*)X)

void buildHeader(std::ofstream& file, const Sequence* seq, const int fraNum, std::string id, std::string prefix);
void buildImages(std::ofstream& file, const Frame* frame, std::string id, std::string prefix);
int imageCount(const Frame_AF* af);
void imageName(std::ofstream& file, const Frame_AF* af, const int layerNum, std::string id, std::string prefix);


void FrameData::BuildJonb(float offsetX, float offsetY, std::string id, float scale, std::string output, std::string prefix, bool justPat, bool justFra)
{
	while (i1 < 1000)
	{
		if (i2 == 100)
		{
			i2 = -1;
			i1++;
		}
		else {
			name.clear();
			name = output + "Action_";
			if (i1 < 10)
			{
				name += "00" + std::to_string(i1);
			}
			else if (i1 < 100)
			{
				name += "0" + std::to_string(i1);
			}
			else {
				name += std::to_string(i1);
			}

			if (i2 < 10)
			{
				name += "_0" + std::to_string(i2);
			}
			else {
				name += "_" + std::to_string(i2);
			}

			name += ".jonbin";

			auto seq = get_sequence(i1);
			if (seq)
			{
				int nframes = seq->frames.size() - 1;

				if (nframes >= 0 && i2 <= nframes)
				{
					std::ofstream file(name, std::ios_base::out | std::ios_base::binary);

					file.write("JONB", 4);

					buildHeader(file, &m_sequences[i1], i2, id, prefix);


					file.close();
				}
				else {
					i1++;
					i2 = -1;
				}
			}

		}
		i2++;
	}
	i1 = 0;
	i2 = 0;
}

void buildHeader(std::ofstream& file, const Sequence *seq, const int fraNum, std::string id, std::string prefix)
{
	int i = 0;
	for (const auto& frame : seq->frames)
	{
		if (i == fraNum)
		{
			buildImages(file, &frame, id, prefix);
		}
		i++;
	}

}

void buildImages(std::ofstream& file, const Frame* frame, std::string id, std::string prefix)
{
	uint16_t imageNum = imageCount(&frame->AF);
	file.write(VAL(imageNum), 2);

	for (int i = 1; i <= imageNum; i++)
	{
		imageName(file, &frame->AF, i - 1, id, prefix);
	}

}

int imageCount(const Frame_AF *af)
{
	int layers = 0;
	for (int i = 0; i < af->layers.size(); i++)
	{
		const auto& l = af->layers[i];
		if (l.spriteId > -1)
		{
			layers++;
		}
	}

	return layers;
}

void imageName(std::ofstream& file, const Frame_AF* af, const int layerNum, std::string id, std::string prefix)
{
	const auto& l = af->layers[layerNum];
	int sprId = l.spriteId;
	CG cg;
	if (l.usePat)
	{
		std::string sprName = "vr_";
		sprName += id;
		sprName += "_ef000.bmp";
		char buf[32]{};
		strncpy(buf, sprName.c_str(), 32);
		file.write(PTR(buf), 32);
	}
	else {
		/*
		const char* filename = cg.get_filename(sprId);
		
		std::string sprName = prefix;
		//sprName.append(filename);
		//char buf[32]{};
		//strncpy(buf, sprName.c_str(), 32);
		file.write(PTR(filename), 32);
		*/
	}
}