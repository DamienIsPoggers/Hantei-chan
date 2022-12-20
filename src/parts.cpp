#include "parts.h"
#include "misc.h"
#include "cg.h"

#include <algorithm>
#include <iostream>
#include <cassert>

#include <glm/gtc/matrix_transform.hpp>

PartProperty newPr();

struct DDS_PIXELFORMAT
{
	uint32_t    size;
	uint32_t    flags;
	uint32_t    fourCC;
	uint32_t    RGBBitCount;
	uint32_t    RBitMask;
	uint32_t    GBitMask;
	uint32_t    BBitMask;
	uint32_t    ABitMask;
};

struct DDS_HEADER
{
	uint32_t        size;
	uint32_t        flags;
	uint32_t        height;
	uint32_t        width;
	uint32_t        pitchOrLinearSize;
	uint32_t        depth;
	uint32_t        mipMapCount;
	uint32_t        reserved1[11];
	DDS_PIXELFORMAT ddspf;
	uint32_t        caps;
	uint32_t        caps2;
	uint32_t        caps3;
	uint32_t        caps4;
	uint32_t        reserved2;
};

Parts::Parts(CG* cg) : cg(cg), partVertices(Vao::F3F4, GL_STATIC_DRAW)
{

}

unsigned int* Parts::VeLoad(unsigned int* data, const unsigned int* data_end, int amount, int len)
{
	for (int id = 0; id < amount && data < data_end; id++) {
		Shape sh{};
		sh.type = data[0];
		switch (sh.type)
		{
		//Plane
		case 1:
		//Seems that no effect in MBTL is of type 2. No idea what it is.
		case 2:
			//do nothing
			break;
		//Ring
		case 3: 
		case 4:
			sh.radius = data[3];
			sh.width = data[4];
			sh.vertexCount = data[5];
			sh.length = data[6];
			sh.dz = data[7];
			sh.dRadius = data[8];
			break;
		//Sphere
		case 5:
			sh.radius = data[3];
			sh.vertexCount = data[4];
			sh.vertexCount2 = data[5];
			sh.length = data[6];
			sh.length2 = data[7];
			break;
		//Cone...?
		case 6:
			sh.radius = data[3];
			sh.dz = data[4];
			sh.vertexCount = data[5];
			sh.vertexCount2 = data[6];
			sh.length = data[7];
			break;
		default:
			std::cout << "\tUnknown Shape type: " << sh.type << "\n";
			break;
		}
		shapes.insert({ id,sh });
		data += len;
	}
	return data;
}
//UNI only
unsigned int* Parts::VnLoad(unsigned int* data, const unsigned int* data_end, int amount)
{
	while (data < data_end) {
		unsigned int* buf = data;
		++data;

		if (!memcmp(buf, "VNST", 4)) {
			for (int i = 0; i < amount; i++)
			{
				std::string name;
				name = (char*)data;
				shapeNames.insert({ i,name });
				data += 8;
			}
		}
		else if (!memcmp(buf, "VEED", 4)) {
			break;
		}
		else {
			char tag[5]{};
			memcpy(tag, buf, 4);
			std::cout << "\tUnknown VE level tag: " << tag << "\n";
		}
	}
	return data;
}

bool Decrappress(unsigned char* cdata, unsigned char* outData, size_t csize, size_t outSize)
{
	int accum = 0;
	size_t wi = 0;
	for (size_t i = 0; i < csize; ++i)
	{
		if (wi > outSize)
			return false;
		if (cdata[i] == 0)
		{
			++i;
			for (size_t j = 0; j < cdata[i + 1]; ++j)
				outData[wi++] = cdata[i];
			i += 1;
		}
		else
		{
			outData[wi++] = cdata[i];
		}
	}
	return true;
}

//Graphic data
unsigned int* Parts::PgLoad(unsigned int* data, const unsigned int* data_end, int id)
{
	PartGfx tex{};
	tex.pgstNum = id;
	while (data < data_end) {
		unsigned int* buf = data;
		++data;
		if (!memcmp(buf, "PGNM", 4)) {
			tex.name = (char*)data;
			//std::cout << id <<" PG: "<<(char*)data << "\n";
			data += 0x20 / 4;
		}
		else if (!memcmp(buf, "PGTP", 4)) {
			//tex.type = data[0];
			//std::cout <<"\tGraphic type"<<tex.type <<"\n";
			++data;
		}
		else if (!memcmp(buf, "PGTE", 4)) { //UNI
		 //two shorts? Size again?
			++data;
		}
		else if (!memcmp(buf, "PGT2", 4)) { //UNI
			unsigned int someSize = data[0]; // is size1 + 16?
			tex.filesize = someSize - 16;
			tex.w = data[1];
			tex.h = data[2];
			data += 3;

			if (*data == 21)
				tex.type = 21;
			else if (!memcmp(data, "DXT1", 4))
				tex.type = 1;
			else if (!memcmp(data, "DXT5", 4))
				tex.type = 5;
			else
				assert(0 && "Unknown texture type");

			data += 1;

			if (tex.type == 5 && someSize == tex.w * tex.h + 128) //uncompressed?
			{
				data += 2; //skip unknown shit
				tex.s3tc = ((unsigned char*)data) + 128;
				tex.dontDelete = true;
				data += tex.w * tex.h / 4 +0x20;
			}
			else if (someSize == tex.w * tex.h * 4 + 128)
			{
				data += 2; //skip unknown shit
				tex.s3tc = ((unsigned char*)data) + 128;
				tex.dontDelete = true;
				data += tex.w * tex.h + 0x20;
			}
			else
			{
				int cSize = data[4]; //From 'DDS ' to PGED
				int oSize = data[5]; //Uncompressed size.
				data += 6;

				unsigned char* cData = (unsigned char*)data;
				tex.data = (char*)cData;
				auto oData = new unsigned char[oSize];
				bool result = Decrappress(cData, oData, cSize, oSize);
				assert(result);
				DDS_HEADER* dds = (DDS_HEADER*)(oData + 4);
				tex.s3tc = oData + 128;

				cData += cSize;
				data = (unsigned int*)cData;
			}
		}
		else if (!memcmp(buf, "PGTX", 4)) {
			tex.w = data[0];
			tex.h = data[1];
			tex.bpp = data[2];
			tex.data = (char*)(data + 3);
			//std::cout <<"\t"<<tex.w <<"x" <<tex.h <<" "<<tex.bpp<<"bpp\n";

			assert(tex.bpp == 32);
			data += (tex.w * tex.h) + 3;
		}
		else if (!memcmp(buf, "PGED", 4)) {
			gfxMeta.insert({ id,tex });
			break;
		}
		else {
			char tag[5]{};
			memcpy(tag, buf, 4);
			//std::cout <<"\tUnknown PG level tag: " << tag <<"\n";
		}
	}
	return data;
}

//More like box cut-outs
unsigned int* Parts::PpLoad(unsigned int* data, const unsigned int* data_end, int id)
{
	CutOut pp{};
	std::string name;

	while (data < data_end) {
		unsigned int* buf = data;
		++data;
		/*
		if (!memcmp(buf, "PPNM", 4)) {
			pp.name = (char*)data;
			//std::cout << id <<" PP: "<<(char*)data << "\n";
			data += 0x20 / 4;
		}
		*/
		if (!memcmp(buf, "PPNA", 4)) { //UNI
		 //Non null terminated name. Sjis
			unsigned char* cdata = (unsigned char*)data;
			name.resize(*cdata);
			memcpy(name.data(), (cdata + 1), *cdata);
			//std::cout << id <<" PP: "<< name << "\n";
			cdata += *cdata + 1; //Length
			char nm = *name.c_str();
			pp.name = &nm;
			data = (unsigned int*)cdata;
		}
		else if (!memcmp(buf, "PPCC", 4)) {
			//XY. Transform coordinates seem inverted because they're vertex coordinates.
			//This fransform is applied earlier.
			memcpy(pp.xy, data, sizeof(int) * 2);
			data += 2;
		}
		else if (!memcmp(buf, "PPUV", 4)) {
			//Texture coords
			//LEFT, TOP, W, H
			memcpy(pp.uv, data, sizeof(int) * 4);
			data += 4;
		}
		else if (!memcmp(buf, "PPSS", 4)) {
			//W and H of coordinates. +Y is down as usual.
			memcpy(pp.wh, data, sizeof(int) * 2);
			data += 2;
		}
		else if (!memcmp(buf, "PPTE", 4)) { //UNI
			memcpy(pp.texRatio, data, sizeof(uint16_t)*2);
			++data;
		}
		else if (!memcmp(buf, "PPPA", 4)) { //UNI
			pp.colorSlot = *((char*)data);
			++data;
		}
		else if (!memcmp(buf, "PPTP", 4)) {
			//Texture reference id. Default is 0.
			/* if(name.empty())
				std::cout << id <<" PP: ----\n";
			std::cout << "\t"<<"PPTP " << *data << "\n"; */
			pp.texture = *data;
			++data;
		}
		else if (!memcmp(buf, "PPPP", 4)) {
			pp.shapeIndex = *data;
			++data;
		}
		else if (!memcmp(buf, "PPTX", 4)) {
			assert(0); //melty only?
			//No idea. Doesn't seem to do anything.
			++data;
		}
		else if (!memcmp(buf, "PPJP", 4)) {
			//Some XY offset for when the part "grabs" the player
			memcpy(pp.jump, data, sizeof(int) * 2);
			data += 2;
		}
		else if (!memcmp(buf, "PPED", 4)) {
			cutOuts.insert({ id,pp });
			break;

		}
		else {
			char tag[5]{};
			memcpy(tag, buf, 4);
			std::cout << "\tUnknown PP level tag: " << tag << "\n";
		}
	}
	return data;
}

//Part properties
unsigned int* Parts::PrLoad(unsigned int* data, const unsigned int* data_end, int groupId, int propId)
{
	PartProperty pr{};
	pr.objID = propId;
	while (data < data_end) {
		unsigned int* buf = data;
		++data;
		if (!memcmp(buf, "PRXY", 4)) {
			pr.x = ((int*)data)[0];
			pr.y = ((int*)data)[1];
			data += 2;
		}
		else if (!memcmp(buf, "PRAL", 4)) { //Opt
			unsigned char* cdata = (unsigned char*)data;
			pr.additive = *cdata;
			if (pr.additive == 2) {
				pr.additive = 1;
			}
			++cdata;
			data = (unsigned int*)cdata;
		}
		else if (!memcmp(buf, "PRRV", 4)) { //Opt
			//flip & 1 = flip horizontally; flip & 2 = flip vertically
			unsigned char* cdata = (unsigned char*)data;
			pr.flip = *cdata;
			++cdata;
			data = (unsigned int*)cdata;
		}
		else if (!memcmp(buf, "PRFL", 4)) { //Opt
			unsigned char* cdata = (unsigned char*)data;
			pr.filter = *cdata;
			++cdata;
			data = (unsigned int*)cdata;
		}
		else if (!memcmp(buf, "PRZM", 4)) {
			pr.scaleX = *((float*)data);
			pr.scaleY = *((float*)data + 1);
			data += 2;
		}
		else if (!memcmp(buf, "PRSP", 4)) {
			//Add color. bgra
			memcpy(pr.addColor, data, sizeof(char) * 4);
			pr.addRgba[0] = pr.addColor[2] / 255;
			pr.addRgba[1] = pr.addColor[1] / 255;
			pr.addRgba[2] = pr.addColor[0] / 255;
			++data;
		}
		else if (!memcmp(buf, "PRAN", 4)) {
			//Float, rotation, clockwise. 1.f = 360
			assert(0); //Deprecated?

			++data;
		}
		else if (!memcmp(buf, "PRPR", 4)) {
			//Priority. Higher value means draw first / lower on the stack.
			pr.priority = *data;
			pr.priority += propId / 1000.f; //AAAAAAA
			++data;
		}
		else if (!memcmp(buf, "PRID", 4)) {
			//Part id. Which thing to render
			pr.ppId = *data;
			++data;
		}
		else if (!memcmp(buf, "PRCL", 4)) {
			//Color key
			memcpy(pr.bgra, data, sizeof(char) * 4);
			pr.rgba[0] = pr.bgra[2] / 255;
			pr.rgba[1] = pr.bgra[1] / 255;
			pr.rgba[2] = pr.bgra[0] / 255;
			pr.rgba[3] = pr.bgra[3] / 255;
			++data;
		}
		else if (!memcmp(buf, "PRA3", 4)) { //UNI angle
		 //No idea what the first float is. Don't think it's a quaternion.
			memcpy(pr.rotation, data + 1, sizeof(float) * 3);
			data += 4;
		}
		else if (!memcmp(buf, "PRED", 4)) {
			groups[groupId].push_back(pr);
			break;
		}
		else {
			char tag[5]{};
			memcpy(tag, buf, 4);
			std::cout << "\tUnknown PR level tag: " << tag << "\n";
		}
	}
	return data;
}

//Contains many parts
unsigned int* Parts::P_Load(unsigned int* data, const unsigned int* data_end, int id)
{
	std::string name;
	bool hasData = false;
	while (data < data_end) {
		unsigned int* buf = data;
		++data;
		/*
		if (!memcmp(buf, "PANM", 4)) {
			//Melty name
			name = (char*)data;
			data += 0x20 / 4;
		} */
		if (!memcmp(buf, "PANA", 4)) { //UNI
		 //Non null terminated name
			unsigned char* cdata = (unsigned char*)data;
			name.resize(cdata[0]); //Length at 0
			memcpy(name.data(), cdata + 1, cdata[0]);
			cdata += cdata[0] + 1;
			groupNames.insert({id,name});
			data = (unsigned int*)cdata;
		}
		else if (!memcmp(buf, "PRST", 4)) {
			groups.insert({ id,{} });
			int propId = data[0];
			hasData = true;
			++data;
			data = PrLoad(data, data_end, id, propId);
		}
		else if (!memcmp(buf, "P_ED", 4)) {
			break;
		}
		else {
			char tag[5]{};
			memcpy(tag, buf, 4);
			std::cout << "\tUnknown P_ level tag: " << tag << "\n";
		}
	}
	/* if(hasData)
		std::cout << id <<" _P: "<< name << "\n"; */
	return data;
}


unsigned int* Parts::MainLoad(unsigned int* data, const unsigned int* data_end)
{
	while (data < data_end) {
		unsigned int* buf = data;
		++data;

		if (!memcmp(buf, "P_ST", 4)) {
			unsigned int p_id = *data;
			++data;
			data = P_Load(data, data_end, p_id);
		}
		else if (!memcmp(buf, "PPST", 4)) {
			int id = data[0];
			++data;
			data = PpLoad(data, data_end, id);
		}
		else if (!memcmp(buf, "PGST", 4)) {
			int id = data[0];
			++data;
			data = PgLoad(data, data_end, id);
		}
		else if (!memcmp(buf, "VEST", 4)) { //UNI
			int amount = data[0];
			int len = data[1];
			data += 2;
			data = VeLoad(data, data_end, amount, len);
			data = VnLoad(data, data_end, amount);
		}
		else if (!memcmp(buf, "_END", 4)) {
			break;
		}
		else {
			char tag[5]{};
			memcpy(tag, buf, 4);
			std::cout << "\tUnknown top level tag: " << tag << "\n";
		}
	}
	return data;
}

bool Parts::Load(const char* name)
{
	char* data;
	unsigned int size;
	if (!ReadInMem(name, data, size))
		return false;

	if (memcmp(data, "PAniDataFile", 12))
	{
		delete[] data;
		return false;
	}

	unsigned int* d = (unsigned int*)(data + 0x20);
	unsigned int* d_end = (unsigned int*)(data + size);
	if (memcmp(d, "_STR", 4)) {
		delete[] data;
		return false;
	}

	delete[] this->data;
	this->data = data;
	for (auto& tex : textures)
		delete tex;
	textures.clear();
	cutOuts.clear();
	gfxMeta.clear();
	groups.clear();
	groupNames.clear();
	shapes.clear();
	shapeNames.clear();
	partVertices.Clear();
	MainLoad(d + 1, d_end);
	std::cout << std::endl;

	//Load textures
	for (auto& gfxKv : gfxMeta)
	{
		auto& gfx = gfxKv.second;
		textures.push_back(new Texture);
		if (gfx.s3tc)
		{
			if (gfx.type == 21)
			{
				textures.back()->LoadDirect((char*)gfx.s3tc, gfx.w, gfx.h);
			}
			else
			{
				size_t size;
				if (gfx.type == 5)
					size = gfx.w * gfx.h;
				else if (gfx.type == 1)
					size = (gfx.w * gfx.h) * 3 / 6;
				else
					assert(0);

				textures.back()->LoadCompressed((char*)gfx.s3tc, gfx.w, gfx.h, size, gfx.type);
			}
			if (!gfx.dontDelete)
				delete[](gfx.s3tc - 128);
		}
		else
			textures.back()->LoadDirect(gfx.data, gfx.w, gfx.h);
		gfx.textureIndex = textures.back()->id;
	}

	//Load vertex data in vao.
	std::vector<float> vertexData;
	vertexData.reserve(128 * 7);
	for (auto& partKv : cutOuts)
	{
		auto& part = partKv.second;
		constexpr int tX[] = { 0,1,1, 1,0,0 };
		constexpr int tY[] = { 0,0,1, 1,1,0 };
		constexpr int tXI[] = { 0,1,1, 1,0,0 };
		constexpr int tYI[] = { 1,1,0, 0,0,1 };
		struct {
			float x, y, z, s, t, p, q;
		}point[6];
		float width = 256;//gfxMeta[part.texture].w/2.f;
		float height = 256;//gfxMeta[part.texture].h/2.f;
		if (gfxMeta.count(part.texture) == 0)
		{
			//std::cout << "There's no graphic id "<< part.texture<<" requested by part id "<<partKv.first<<"\n";
			continue;
		}
		if (shapes.count(part.shapeIndex) == 0) {
			continue;
		}
		Shape s = shapes[part.shapeIndex];
		auto size = vertexData.size();
		switch (s.type)
		{
		case 1:
		case 2:
		{
			for (int i = 0; i < 6; i++)
			{
				point[i].x = -part.xy[0] + part.wh[0] * tX[i];
				point[i].y = -part.xy[1] + part.wh[1] * tY[i];
				point[i].z = 0;
				point[i].s = float(part.uv[0] + part.uv[2] * tX[i]) / width;
				point[i].t = float(part.uv[1] + part.uv[3] * tY[i]) / height;
				point[i].p = float(part.uv[0] + part.uv[2] * (1 - tX[i])) / width;
				point[i].q = float(part.uv[1] + part.uv[3] * (1 - tY[i])) / height;
			}
			vertexData.resize(size + 6 * 7);
			memcpy(&vertexData[size], point, sizeof(point));
			break;
		}
		case 3:
		{
			float angle = 0;
			float delta = glm::pi<float>() * s.length / 5000 / s.vertexCount;
			vertexData.resize(size + 6 * s.vertexCount * 7);
			for (int i = 0; i < s.vertexCount; i++) {
				for (int j = 0; j < 6; j++)
				{
					point[j].x = float(s.radius - ((i + tY[j] == s.vertexCount && s.length == 10000) ? 0 : float(s.dRadius * (i + tY[j])) / s.vertexCount)) * glm::sin(angle + delta * tY[j]);
					point[j].y = -float(s.radius - ((i + tY[j] == s.vertexCount && s.length == 10000) ? 0 : float(s.dRadius * (i + tY[j])) / s.vertexCount)) * glm::cos(angle + delta * tY[j]);
					point[j].z = float(s.width * -(tX[j] * 2 - 1) - float(s.dz * (i + tY[j])) / s.vertexCount);
					point[j].s = float(part.uv[0] + part.uv[2] * (tX[j])) / width;
					point[j].t = float(part.uv[1] + 1.0f * part.uv[3] * (i + tY[j]) / s.vertexCount) / height;
					point[j].p = float(part.uv[0] + part.uv[2] * (1 - tX[j])) / width;
					point[j].q = float(part.uv[1] + 1.0f * part.uv[3] * (s.vertexCount - i - tY[j]) / s.vertexCount) / height;
				}
				angle += delta;
				memcpy(&vertexData[size + 6 * 7 * i], point, sizeof(point));
			}
			break;
		}
		case 4:
		{
			float angle = 0;
			float delta = glm::pi<float>() * s.length / 5000 / s.vertexCount;
			vertexData.resize(size + 6 * s.vertexCount * 7);
			for (int i = 0; i < s.vertexCount; i++) {
				for (int j = 0; j < 6; j++)
				{
					point[j].x = float((s.radius - (1 - tX[j]) * s.width - ((i + tY[j] == s.vertexCount && s.length == 10000) ? 0 : float(s.dRadius * (i + tY[j])) / s.vertexCount)) * glm::sin(angle + delta * tY[j]));
					point[j].y = -float((s.radius - (1 - tX[j]) * s.width - ((i + tY[j] == s.vertexCount && s.length == 10000) ? 0 : float(s.dRadius * (i + tY[j])) / s.vertexCount)) * glm::cos(angle + delta * tY[j]));
					point[j].z = -float(s.dz * (i + tY[j])) / s.vertexCount;
					point[j].s = float(part.uv[0] + part.uv[2] * (tX[j])) / width;
					point[j].t = float(part.uv[1] + 1.0f * part.uv[3] * (i + tY[j]) / s.vertexCount) / height;
					point[j].p = float(part.uv[0] + part.uv[2] * (1 - tX[j])) / width;
					point[j].q = float(part.uv[1] + 1.0f * part.uv[3] * (s.vertexCount - i - tY[j]) / s.vertexCount) / height;
				}
				angle += delta;
				memcpy(&vertexData[size + 6 * 7 * i], point, sizeof(point));
			}
			break;
		}
		case 5:
		{
			float angle = 0;
			float delta = glm::pi<float>() * s.length / 5000 / s.vertexCount;
			float angle2 = 0;
			float delta2 = glm::pi<float>() * s.length2 / 10000 / s.vertexCount2;
			vertexData.resize(size + 6 * s.vertexCount * s.vertexCount2 * 7);
			for (int i = 0; i < s.vertexCount; i++) {
				angle2 = 0;
				for (int j = 0; j < s.vertexCount2; j++) {
					for (int k = 0; k < 6; k++)
					{
						point[k].x = float((s.radius) * glm::sin(angle2 + delta2 * tX[k]) * glm::sin(angle + delta * tY[k]));
						point[k].y = float((s.radius) * glm::sin(angle2 + delta2 * tX[k]) * glm::cos(angle + delta * tY[k]));
						point[k].z = s.radius * glm::cos(angle2 + delta2 * tX[k]);
						point[k].s = float(part.uv[0] + float(part.uv[2] * (i + tY[k])) / s.vertexCount) / width;
						point[k].t = float(part.uv[1] + float(part.uv[3] * (j + tX[k]) / s.vertexCount2)) / height;
						point[k].p = float(part.uv[0] + float(part.uv[2] * (s.vertexCount - i - tY[k])) / s.vertexCount) / width;
						point[k].q = float(part.uv[1] + float(part.uv[3] * (s.vertexCount2 - j - tX[k]) / s.vertexCount2)) / height;
					}
					memcpy(&vertexData[size + 6 * 7 * (s.vertexCount2 * i + j)], point, sizeof(point));
					angle2 += delta2;
				}
				angle += delta;
			}
			break;
		}
		case 6:
		{
			float angle = 0;
			float delta = glm::pi<float>() * s.length / 5000 / s.vertexCount;
			vertexData.resize(size + 6 * s.vertexCount * s.vertexCount2 * 7);
			float w = float(s.radius) / s.vertexCount2;
			for (int i = 0; i < s.vertexCount; i++) {
				for (int j = 0; j < s.vertexCount2; j++) {
					for (int k = 0; k < 6; k++) {
						point[k].x = -w * (s.vertexCount2 - 1 - j + tX[k]) * glm::sin(angle + delta * tY[k]);
						point[k].y = -w * (s.vertexCount2 - 1 - j + tX[k]) * glm::cos(angle + delta * tY[k]);
						point[k].z = -s.dz * float(j + (1 - tX[k])) / s.vertexCount2;
						point[k].s = float(part.uv[0] + 1.0f * part.uv[2] * (i + tY[k]) / s.vertexCount) / width;
						point[k].t = float(part.uv[1] + 1.0f * part.uv[3] * (1 + j - tX[k]) / s.vertexCount2) / height;
						point[k].p = float(part.uv[0] + 1.0f * part.uv[2] * (s.vertexCount - i - tY[k]) / s.vertexCount) / width;
						point[k].q = float(part.uv[1] + 1.0f * part.uv[3] * (s.vertexCount2 - 1 - j + tX[k]) / s.vertexCount2) / height;
					}
					memcpy(&vertexData[size + 6 * 7 * (s.vertexCount2 * i + j)], point, sizeof(point));
				}
				angle += delta;
			}
			break;
		}
		default:
			continue;
			break;
		}
	}
	int index = 0;
	for (auto& cutOut : cutOuts)
	{
		if (gfxMeta.count(cutOut.second.texture) == 0) {
			continue;
		}
		if (shapes.count(cutOut.second.shapeIndex) == 0) {
			continue;
		}
		Shape s = shapes[cutOut.second.shapeIndex];
		switch (s.type) {
		case 1:
		case 2:
		{
			cutOut.second.vaoIndex = partVertices.Prepare(6 * 7 * sizeof(float), &vertexData[index]);
			index += 6 * 7;
			break;
		}
		case 3:
		case 4:
		{
			cutOut.second.vaoIndex = partVertices.Prepare(6 * 7 * shapes[cutOut.second.shapeIndex].vertexCount * sizeof(float), &vertexData[index]);
			index += 6 * 7 * shapes[cutOut.second.shapeIndex].vertexCount;
			break;
		}
		case 5:
		{
			cutOut.second.vaoIndex = partVertices.Prepare(6 * 7 * shapes[cutOut.second.shapeIndex].vertexCount * shapes[cutOut.second.shapeIndex].vertexCount2 * sizeof(float), &vertexData[index]);
			index += 6 * 7 * shapes[cutOut.second.shapeIndex].vertexCount * shapes[cutOut.second.shapeIndex].vertexCount2;
			break;
		}
		case 6:
		{
			cutOut.second.vaoIndex = partVertices.Prepare(6 * 7 * shapes[cutOut.second.shapeIndex].vertexCount * shapes[cutOut.second.shapeIndex].vertexCount2 * sizeof(float), &vertexData[index]);
			index += 6 * 7 * shapes[cutOut.second.shapeIndex].vertexCount * shapes[cutOut.second.shapeIndex].vertexCount2;
			break;
		}
		}
	}

	//Sort parts in group by priority;
	reorderLayers();

	partVertices.Load();
	loaded = true;
	return true;
}

void Parts::reloadTextures()
{
}

void Parts::reorderLayers()
{
	for (auto& group : groups)
	{
		auto& v = group.second;
		std::stable_sort(v.rbegin(), v.rend(), [](const PartProperty& a, const PartProperty& b) {
			return a.priority < b.priority;
			});
	}
}

void Parts::Draw(int pattern,
	std::function<void(glm::mat4)> setMatrix,
	std::function<void(float, float, float)> setAddColor,
	std::function<void(char)> setFlip, float color[4])
{
	curTexId = -1;
	if (groups.count(pattern))
	{
		partVertices.Bind();
		for (auto& part : groups[pattern])
		{
			if (cutOuts.count(part.ppId) == 0)
				continue;
			auto cutout = cutOuts[part.ppId];

			constexpr float tau = glm::pi<float>() * 2.f;
			glm::mat4 view = glm::mat4(1.f);

			view = glm::translate(view, glm::vec3(part.x, part.y, 0.f));
			view = glm::rotate(view, -part.rotation[1] * tau, glm::vec3(0.0, 1.f, 0.f));
			view = glm::rotate(view, -part.rotation[0] * tau, glm::vec3(1.0, 0.f, 0.f));
			view = glm::rotate(view, part.rotation[2] * tau, glm::vec3(0.0, 0.f, 1.f));

			setFlip(part.flip);

			view = glm::scale(view, glm::vec3(part.scaleX, part.scaleY, 1.f));

			setMatrix(view);
			if (screenShot)
				glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_SRC_ALPHA, GL_ONE);
			else if (part.additive)
				glBlendFunc(GL_SRC_ALPHA, GL_ONE);
			else
				glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

			float newColor[4];
			memcpy(newColor, color, sizeof(float) * 4);
			if (cutout.colorSlot != 0) {
				unsigned int color = cg->getColorFromPal(cutout.colorSlot);
				if (color) {
					newColor[0] *= (color & 0xFF) / 255.f;
					newColor[1] *= ((color >> 8) & 0xFF) / 255.f;
					newColor[2] *= ((color >> 16) & 0xFF) / 255.f;
					newColor[3] *= (color >> 24) / 255.f;
				}

			}
			newColor[0] *= part.bgra[2] / 255.f;
			newColor[1] *= part.bgra[1] / 255.f;
			newColor[2] *= part.bgra[0] / 255.f;
			newColor[3] *= part.bgra[3] / 255.f;
			glVertexAttrib4fv(2, newColor);
			setAddColor(part.addColor[2] / 255.f, part.addColor[1] / 255.f, part.addColor[0] / 255.f);
			DrawPart(part.ppId);
		}
	}
}

void Parts::DrawPart(int i)
{
	if (cutOuts.count(i) > 0 && gfxMeta.count(cutOuts[i].texture))
	{
		if (curTexId != gfxMeta[cutOuts[i].texture].textureIndex)
		{
			curTexId = gfxMeta[cutOuts[i].texture].textureIndex;
			glBindTexture(GL_TEXTURE_2D, curTexId);
		}
		partVertices.Draw(cutOuts[i].vaoIndex);
	}
}

PartProperty* Parts::GetPr(const int sprite, const int layer)
{
	if (groups.count(sprite))
	{
		for (auto& part : groups[sprite])
		{
			if (part.objID == layer) return &part;
		}
	}
	return nullptr;
}

CutOut* Parts::GetPP(const int particle)
{
	if (cutOuts.count(particle))
	{
		return &cutOuts[particle];
	}
	return nullptr;
}

PartGfx* Parts::GetTexture(unsigned int n)
{
	if (gfxMeta.count(n))
	{
		return &gfxMeta[n];
	}
	return nullptr;
}

void Parts::newSprite(int num)
{
	std::string nm = "";
	groupNames.insert({ num, nm });

	PartProperty pr = newPr();
	pr.objID = 0;
	groups.insert({ num, {} });
	groups[num].push_back(pr);
}

void Parts::newLayer(int sprnum, int num)
{
	PartProperty pr = newPr();
	pr.objID = num;
	groups[sprnum].push_back(pr);
}

PartProperty newPr()
{
	PartProperty pr;
	pr.addColor[0] = 0;
	pr.addColor[1] = 0;
	pr.addColor[2] = 0;
	pr.addColor[3] = 0;
	pr.addRgba[0] = 0;
	pr.addRgba[1] = 0;
	pr.addRgba[2] = 0;

	pr.bgra[0] = 255;
	pr.bgra[1] = 255;
	pr.bgra[2] = 255;
	pr.bgra[3] = 255;
	pr.rgba[0] = 1;
	pr.bgra[1] = 1;
	pr.bgra[2] = 1;
	pr.bgra[3] = 1;

	pr.rotation[0] = 0;
	pr.rotation[1] = 0;
	pr.rotation[2] = 0;

	pr.scaleX = 0;
	pr.scaleY = 0;

	pr.priority = 0;
	pr.additive = false;
	pr.filter = false;
	pr.flip = false;
	pr.ppId = -1;
	pr.x = 0;
	pr.y = 0;
	pr.objID = 0;

	return pr;
}