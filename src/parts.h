#ifndef PARTS_H_GUARD
#define PARTS_H_GUARD

#include "image.h"
#include "cg.h"
#include "texture.h"
#include "vao.h"
#include <vector>
#include <unordered_map>
#include <functional>
#include <glm/mat4x4.hpp>

class Parts {
public:
	CG* cg;
	char* data = nullptr;

	//VE data
	struct Shape
	{
		int type;
		int vertexCount;
		int vertexCount2;
		int length;
		int length2;
		int radius;
		int dRadius;
		int width;
		int dz;
	};

	//PP data
	struct CutOut
	{
		char* name; //PPNA

		int uv[4]; //PPUV
		int xy[2]; //PPCC
		int wh[2]; //PPSS
		int texture = 0; //PPTP
		//int type;
		int jump; //unused??
		int vaoIndex; //used but idk for
		unsigned char colorSlot; //PPPA
		int shapeIndex; //PPPP
	};

	//PG data
	struct PartGfx {
		char* name; //PGNM
		char* data; 
		//PGT2 data
		int w, h; //bytes 5-8 && 9-12
		int bpp;
		int type; //bytes 13-16
		int textureIndex; //PGST
		unsigned char* s3tc = nullptr;
		bool dontDelete = false;
	};

	//PR data
	struct PartProperty {
		float priority; //PRST ?
		float rotation[3]; //PRA3
		float x; //PRXY 1-4
		float y; //PRXY 5-8
		float scaleX = 1.f; //PRZM 1-4
		float scaleY = 1.f; //PRZM 5-8
		int ppId; //PRID
		bool additive; //PRAL
		bool filter; //PRFL
		char flip; //PRAV
		unsigned char bgra[4] = { 255,255,255,255 }; //PRCL
		unsigned char addColor[4] = { 0,0,0,0 }; //PRSP
	};

	//Has id and a list of parts

	std::unordered_map<int, std::vector<PartProperty>> groups;
	std::unordered_map<int, CutOut> cutOuts;
	std::unordered_map<int, Shape> shapes;
	std::unordered_map<int, PartGfx> gfxMeta;
	std::vector<Texture*> textures;
	Vao partVertices;
	int curTexId;

	unsigned int* MainLoad(unsigned int* data, const unsigned int* data_end);
	unsigned int* P_Load(unsigned int* data, const unsigned int* data_end, int id);
	unsigned int* PrLoad(unsigned int* data, const unsigned int* data_end, int groupId, int propId);
	unsigned int* PpLoad(unsigned int* data, const unsigned int* data_end, int id);
	unsigned int* PgLoad(unsigned int* data, const unsigned int* data_end, int id);
	unsigned int* VeLoad(unsigned int* data, const unsigned int* data_end, int amount, int len);
	unsigned int* VnLoad(unsigned int* data, const unsigned int* data_end, int amount);
	void DrawPart(int id);

public:
	bool loaded = false;

	Parts(CG* cg);
	bool Load(const char* name);

	PartGfx* GetTexture(unsigned int n);
	void Draw(int pattern, std::function<void(glm::mat4)> setMatrix,
		std::function<void(float, float, float)> setAddColor,
		std::function<void(char)> setFlip, float color[4]);

};

#endif /* PARTS_H_GUARD */
