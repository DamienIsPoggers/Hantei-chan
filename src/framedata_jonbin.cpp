#include "framedata.h"
#include "cg.h"
#include "main_frame.h"
#include "filedialog.h"
#include <fstream>
#include <cstdint>

#define VAL(X) ((const char*)&X)
#define PTR(X) ((const char*)X)

void MainFrame::BuildJonb(float offsetX, float offsetY, std::string id, float scale, std::string output, std::string prefix, bool justPat, bool justFra)
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

			auto seq = framedata.get_sequence(i1);
			if (seq)
			{
				int nframes = seq->frames.size() - 1;

				if (nframes >= 0 && i2 <= nframes)
				{
					std::ofstream file(name, std::ios_base::out | std::ios_base::binary);

					file.write("JONB", 4);

					buildHeader(file, framedata.get_sequence(i1), i2, id, prefix);


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

void MainFrame::buildHeader(std::ofstream& file, const Sequence *seq, const int fraNum, std::string id, std::string prefix)
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

void MainFrame::buildImages(std::ofstream& file, const Frame* frame, std::string id, std::string prefix)
{
	uint16_t imageNum = 0;
	const Frame_AF *af = &frame->AF;
	char buf[32]{};
	std::string sprName;
	for (int i = 0; i < af->layers.size(); i++)
	{
		const auto& l = af->layers[i];
		if (l.spriteId > -1)
		{
			imageNum++;
		}
	}
	file.write(VAL(imageNum), 2);
	for (int i = 0; i < af->layers.size(); i++)
	{
		const auto& l = af->layers[i];
		const int spr = l.spriteId;
		if (l.spriteId > -1)
		{
			if (l.usePat)
			{
				sprName += "vr_";
				sprName += id;
				sprName += "_ef000.bmp";
				strncpy(buf, sprName.c_str(), 32);
			}
			else {
				strncpy(buf, cg.get_filename(spr), 32);
			}
			file.write(buf, 32);
		}
	}
}