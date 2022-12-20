#include "parts.h"
#include <fstream>
#include "misc.h"
#include <cstring>
#include <sstream>
#include <iomanip>

#define VAL(X) ((const char*)&X)
#define PTR(X) ((const char*)X)

void Parts::Save(const char* filename)
{
	std::ofstream file(filename, std::ios_base::out | std::ios_base::binary);
	if (!file.is_open())
		return;

	char header[32] = "PAniDataFile";

	//haha, food
	header[30] = 0xF0;
	header[31] = 0x0D;

	file.write(header, sizeof(header));

	file.write("_STR", 4); 

	int groupsize = groups.size();
	for (int i = 0; i < groupsize; i++)
	{
		if (groups.count(i))
		{
			WriteP_(file, i);
		}
		else {
			groupsize++;
		}
	}

	int ppsize = cutOuts.size();
	for (int i = 0; i < ppsize; i++)
	{
		if (cutOuts.count(i))
		{
			WritePP(file, i);
		}
		else {
			ppsize++;
		}
	}

	if(shapes.size() > 0)
		WriteVE(file);

	int pgsize = gfxMeta.size();
	for (int i = 0; i < pgsize; i++)
	{
		if (gfxMeta.count(i))
		{
			WritePG(file, i);
		}
		else {
			pgsize++;
		}
	}

	file.write("_END", 4);
	file.close();
}

void Parts::WriteP_(std::ofstream &file, int id)
{
	file.write("P_ST", 4);
	file.write(VAL(id), 4);

	file.write("PANA", 4);
	uint8_t size = groupNames[id].length();
	file.write(VAL(size), 1);
	file.write(groupNames[id].c_str(), groupNames[id].length());

	std::vector<PartProperty> part = groups[id];
	for (int i = 0; i < part.size(); i++)
	{
		file.write("PRST", 4);
		file.write(VAL(part[i].objID), 4);

		if (part[i].x != 0 || part[i].y != 0)
		{
			file.write("PRXY", 4);
			file.write(VAL(part[i].x), 4);
			file.write(VAL(part[i].y), 4);
		}

		if (part[i].additive == true)
		{
			file.write("PRAL", 4);
			file.write(VAL(part[i].additive), 1);
		}

		if (part[i].filter == true)
		{
			file.write("PRFL", 4);
			file.write(VAL(part[i].filter), 1);
		}

		if (part[i].scaleX != 1 || part[i].scaleY != 1)
		{
			file.write("PRZM", 4);
			file.write(VAL(part[i].scaleX), 4);
			file.write(VAL(part[i].scaleY), 4);
		}

		if (part[i].bgra[0] != 255 || part[i].bgra[1] != 255 || part[i].bgra[2] != 255 || part[i].bgra[3] != 255)
		{
			uint8_t colors[4];
			colors[0] = part[i].bgra[0];
			colors[1] = part[i].bgra[1];
			colors[2] = part[i].bgra[2];
			colors[3] = part[i].bgra[3];
			file.write("PRCL", 4);
			file.write(VAL(colors), 4);
		}

		if (part[i].addColor[0] != 0 || part[i].addColor[1] != 0 || part[i].addColor[2] != 0)
		{
			uint8_t colors[4];
			colors[0] = part[i].addColor[0];
			colors[1] = part[i].addColor[1];
			colors[2] = part[i].addColor[2];
			colors[3] = 0;
			file.write("PRSP", 4);
			file.write(VAL(colors), 4);
		}

		if (part[i].rotation[0] != 0 || part[i].rotation[1] != 0 || part[i].rotation[2] != 0)
		{
			int zero = 0;
			file.write("PRA3", 4);
			file.write(VAL(zero), 4);
			file.write(VAL(part[i].rotation[0]), 4);
			file.write(VAL(part[i].rotation[1]), 4);
			file.write(VAL(part[i].rotation[2]), 4);
		}

		if (part[i].priority > 0)
		{
			file.write("PRPR", 4);
			file.write(VAL(part[i].priority), 4);
		}

		file.write("PRID", 4);
		file.write(VAL(part[i].ppId), 4);

		file.write("PRED", 4);
	}

	file.write("P_ED", 4);
}

void Parts::WritePP(std::ofstream& file, int id)
{
	CutOut pp = cutOuts[id];

	file.write("PPST", 4);
	file.write(VAL(id), 4);

	if (pp.name != nullptr)
	{
		file.write("PPNA", 4);
		int size = sizeof(pp.name) + 1;
		file.write(VAL(size), 1);
		file.write(pp.name, size--);
	}

	if (pp.xy[0] != 0 || pp.xy[1] != 0)
	{
		file.write("PPCC", 4);
		file.write(VAL(pp.xy), 8);
	}

	if (pp.uv[0] != 0 || pp.uv[1] != 0 || pp.uv[2] != 0 || pp.uv[3] != 0)
	{
		file.write("PPUV", 4);
		file.write(VAL(pp.uv), 16);
	}

	if (pp.wh[0] != 0 || pp.wh[1] != 0)
	{
		file.write("PPSS", 4);
		file.write(VAL(pp.wh), 8);
	}

	if (pp.texture != 0)
	{
		file.write("PPTP", 4);
		file.write(VAL(pp.texture), 4);
	}

	if (pp.texRatio[0] != 0 || pp.texRatio[1] != 0)
	{
		file.write("PPTE", 4);
		file.write(VAL(pp.texRatio), 4);
	}

	if (pp.colorSlot != 0)
	{
		file.write("PPPA", 4);
		file.write(VAL(pp.colorSlot), 4);
	}

	if (pp.shapeIndex != 0)
	{
		file.write("PPPP", 4);
		file.write(VAL(pp.shapeIndex), 4);
	}


	file.write("PPED", 4);
}

void Parts::WriteVE(std::ofstream& file)
{
	file.write("VEST", 4);
	int count = shapes.size();
	file.write(VAL(count), 4);
	int header2 = 16;
	file.write(VAL(header2), 4);

	uint64_t zero = 0;

	for (int i = 0; i < count; i++)
	{
		Shape shape = shapes[i];
		file.write(VAL(shape.type), 4);
		file.write(VAL(zero), 8);
		int paramsWritten = 13;
		switch (shape.type)
		{
		//Plane
		case 1:
		case 2:
			break;
		//Ring
		case 3:
		case 4:
			file.write(VAL(shape.radius), 4);
			file.write(VAL(shape.width), 4);
			file.write(VAL(shape.vertexCount), 4);
			file.write(VAL(shape.length), 4);
			file.write(VAL(shape.dz), 4);
			file.write(VAL(shape.dRadius), 4);
			paramsWritten = paramsWritten - 6;
			break;
		//Sphere
		case 5:
			file.write(VAL(shape.radius), 4);
			file.write(VAL(shape.vertexCount), 4);
			file.write(VAL(shape.vertexCount2), 4);
			file.write(VAL(shape.length), 4);
			file.write(VAL(shape.length2), 4);
			paramsWritten = paramsWritten - 5;
			break;
		//Cone...?
		case 6:
			file.write(VAL(shape.radius), 4);
			file.write(VAL(shape.dz), 4);
			file.write(VAL(shape.vertexCount), 4);
			file.write(VAL(shape.vertexCount2), 4);
			file.write(VAL(shape.length), 4);
			paramsWritten = paramsWritten - 5;
			break;
		}
		while (paramsWritten > 1)
		{
			file.write(VAL(zero), 8);
			paramsWritten = paramsWritten - 2;
		}
		if (paramsWritten == 1)
			file.write(VAL(zero), 4);
	}

	file.write("VNST", 4);

	for (int i = 0; i < count; i++)
	{
		char buf[32] = "";
		file.write(shapeNames[i].c_str(), shapeNames[i].length());
		file.write(buf, 32 - shapeNames[i].length());
	}

	file.write("VEED", 4);
}

void Parts::WritePG(std::ofstream& file, int id)
{
	PartGfx pg = gfxMeta[id];
	file.write("PGST", 4);
	file.write(VAL(pg.pgstNum), 4);
	
	file.write("PGNM", 4);
	char buf[32] = "";
	file.write(pg.name, sizeof(pg.name));
	file.write(".dds", 4);
	file.write(buf, 28 - sizeof(pg.name));

	file.write("PGTE", 4);
	file.write(VAL(pg.w), 2);
	file.write(VAL(pg.h), 2);

	file.write("PGT2", 4);
	unsigned int tempSize = pg.filesize + 16;
	file.write(VAL(tempSize), 4);
	file.write(VAL(pg.w), 4);
	file.write(VAL(pg.h), 4);
	if (pg.type == 1) file.write("DXT1", 4);
	else if (pg.type == 5) file.write("DXT5", 4);
	else file.write(VAL(pg.type), 4);

	unsigned char misc[] = { 0x04, 0x00, 0x00, 0x00, 0x03, 0x00, 0x00, 0x00, 0xFF, 0x0B, 0x10, 0x01, 0x00, 0x00, 0x00, 0x00 };
	file.write(PTR(misc), sizeof(misc));

	file.write(VAL(pg.filesize), 4);
	int aspect = pg.w * pg.h;
	aspect = aspect + 0x80;
	file.write(VAL(aspect), 4);

	file.write(PTR(pg.data), pg.filesize);

	file.write("PGED", 4);
}