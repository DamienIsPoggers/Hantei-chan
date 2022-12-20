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
	uint16_t texRatio[2];
	//int type;
	int jump[2]; //unused?? PPJP???
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
	int textureIndex;
	unsigned char* s3tc = nullptr; //image itself??
	bool dontDelete = false;
	int filesize;
	int pgstNum;
};

//PR data
struct PartProperty {
	int priority; //PRPR
	float rotation[3]; //PRA3
	int x; //PRXY 1-4
	int y; //PRXY 5-8
	float scaleX = 1.f; //PRZM 1-4
	float scaleY = 1.f; //PRZM 5-8
	int ppId; //PRID
	bool additive; //PRAL
	bool filter; //PRFL
	char flip; //PRAV
	unsigned char bgra[4] = { 255,255,255,255 }; //PRCL
	unsigned char addColor[4] = { 0,0,0,0 }; //PRSP
	int objID; //id for prst
	float rgba[4] = { 1, 1, 1, 1 }; //used for color edit in editor
	float addRgba[3] = { 0, 0, 0 }; //used for color edit in editor
};

class Parts {
public:

	CG* cg;
	char* data = nullptr;

	//Has id and a list of parts
	std::unordered_map<int, std::vector<PartProperty>> groups;
	std::unordered_map<int, std::string> groupNames;
	std::unordered_map<int, CutOut> cutOuts;
	std::unordered_map<int, Shape> shapes;
	std::unordered_map<int, std::string> shapeNames;
	std::unordered_map<int, PartGfx> gfxMeta;
	std::vector<Texture*> textures;
	Vao partVertices;

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
	int curTexId;

	Parts(CG* cg);
	bool Load(const char* name);
	void Save(const char* filename);

	PartProperty* GetPr(const int sprite, const int layer);
	CutOut* GetPP(const int particle);

	void reloadTextures();
	void reorderLayers();

	void newSprite(int num);
	void newLayer(int sprnum, int num);

	PartGfx* GetTexture(unsigned int n);
	void Draw(int pattern, std::function<void(glm::mat4)> setMatrix,
		std::function<void(float, float, float)> setAddColor,
		std::function<void(char)> setFlip, float color[4]);

private:

	//saving stuff
	void WriteP_(std::ofstream& file, int id);
	void WritePP(std::ofstream& file, int id);
	void WriteVE(std::ofstream& file);
	void WritePG(std::ofstream& file, int id);
};

#endif /* PARTS_H_GUARD */
