#include "framedata.h"
#include "cg.h"
#include "main_frame.h"
#include "filedialog.h"
#include "box_pane.h"
#include "hitbox.h"
#include <fstream>
#include <cstdint>

#define VAL(X) ((const char*)&X)
#define PTR(X) ((const char*)X)

void MainFrame::BuildJonb(float offsetX, float offsetY, std::string id, float scale, std::string output, std::string prefix, bool justPat, bool justFra)
{
	while (i1 < 1000)
	{
		imageNum = 0;
		hurtboxCount = 0;
		hitboxCount = 0;
		snapCount1 = 0;
		snapCount2 = 0;
		snapCount3 = 0;
		snapCount4 = 0;
		snapCount5 = 0;
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
	uint8_t twenty = 20;
	file.write(VAL(twenty), 1);
	uint16_t one = 1;
	file.write(VAL(one), 2);
	file.write(VAL(imageNum), 2);
	uint64_t zero = 0;
	file.write(VAL(zero), 2);

	auto& frame = framedata.get_sequence(i1)->frames;
	BoxList& boxes = frame[i2].hitboxes;
	for (int j = 1; j <= 8; j++)
	{
		if (!(boxes[j].xy[0] == 0 && boxes[j].xy[1] == 0
			&& boxes[j].xy[2] == 0 && boxes[j].xy[3] == 0))
		{
			hurtboxCount++;
		}
	}
	for (int j = 25; j <= 32; j++)
	{
		if (!(boxes[j].xy[0] == 0 && boxes[j].xy[1] == 0
			&& boxes[j].xy[2] == 0 && boxes[j].xy[3] == 0))
		{
			hitboxCount++;
		}
	}
	file.write(VAL(hurtboxCount), 2);
	file.write(VAL(hitboxCount), 2);

	if (specToSnap && !(boxes[9].xy[0] == 0 && boxes[9].xy[1] == 0
		&& boxes[9].xy[2] == 0 && boxes[9].xy[3] == 0))
	{
		snapCount1++;
	}
	if (specToSnap && !(boxes[10].xy[0] == 0 && boxes[10].xy[1] == 0
		&& boxes[10].xy[2] == 0 && boxes[10].xy[3] == 0))
	{
		snapCount1++;
	}
	auto sequ = framedata.get_sequence(i1);
	if (sequ)
	{
		Frame& frame = sequ->frames[i2];
		for (int j = 0; j < frame.EF.size(); j++)
		{
			if (frame.EF[j].type == 14)
			{
				snapCount1++;
			}
		}
	}
	file.write(VAL(snapCount1), 2);
	file.write(VAL(zero), 8);

	if (!(boxes[18].xy[0] == 0 && boxes[18].xy[1] == 0
		&& boxes[18].xy[2] == 0 && boxes[18].xy[3] == 0))
	{
		snapCount2++;
	}
	file.write(VAL(snapCount2), 2);
	if (!(boxes[19].xy[0] == 0 && boxes[19].xy[1] == 0
		&& boxes[19].xy[2] == 0 && boxes[19].xy[3] == 0))
	{
		snapCount3++;
	}
	file.write(VAL(snapCount3), 2);
	file.write(VAL(zero), 4);
	if (!(boxes[20].xy[0] == 0 && boxes[20].xy[1] == 0
		&& boxes[20].xy[2] == 0 && boxes[20].xy[3] == 0))
	{
		snapCount4++;
	}
	file.write(VAL(snapCount4), 2);
	if (!(boxes[21].xy[0] == 0 && boxes[21].xy[1] == 0
		&& boxes[21].xy[2] == 0 && boxes[21].xy[3] == 0))
	{
		snapCount5++;
	}
	file.write(VAL(snapCount5), 2);

	for (int j = 0; j < 3; j++)
	{
		file.write(VAL(zero), 8);
	}
	file.write(VAL(zero), 4);

}

void MainFrame::buildImages(std::ofstream& file, const Frame* frame, std::string id, std::string prefix)
{
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