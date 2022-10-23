#ifndef MAINFRAME_H_GUARD
#define MAINFRAME_H_GUARD
#include "context_gl.h"
#include "render.h"
#include "main_pane.h"
#include "right_pane.h"
#include "box_pane.h"
#include "about.h"
#include "framedata.h"
#include "cg.h"
#include "parts.h"
#include <glm/mat4x4.hpp>
#include <string>


class MainFrame
{
public:
	MainFrame(ContextGl *context);
	~MainFrame();
	
	void Draw();
	void UpdateBackProj(float x, float y);
	void HandleMouseDrag(int x, int y, bool dragRight, bool dragLeft);
	bool HandleKeys(uint64_t vkey);

	void RightClick(int x, int y);
	void LoadSettings();

	bool drawImgui = true;

	std::string id;


	FrameData framedata;

	void BuildJonb(float offsetX, float offsetY, std::string id, float scale, std::string output, std::string prefix, bool justPat, bool justFra);
	

private:
	ContextGl *context;
	float clearColor[3];
	int style_idx = 0;
	int zoom_idx = 1;
	bool smoothRender = false; 
	

	Render render;
	FrameState currState{};
	CG cg;
	Parts parts;
	int curPalette;
	float reScaleFactor = 1.0;

	//Animation
	int duration = 0;
	int loopCounter;

	//Option

	std::string currentFilePath;

	void DrawBack();
	void DrawUi();
	void Menu(unsigned int errorId);

	void RenderUpdate();
	void AdvancePattern(int dir);
	void AdvanceFrame(int dir);
	void ChangeOffset(int x, int y);

	void SetZoom(int level);
	void LoadTheme(int i );
	void WarmStyle();
	void ChangeClearColor(float r, float g, float b);

	int mDeltaX = 0, mDeltaY = 0;
	int x=0, y=0;
	

	MainPane mainPane;
	RightPane rightPane;
	BoxPane boxPane;
	AboutWindow aboutWindow;
	HelpWindow helpWindow;

	//jonbin stuff

	std::string outputPath;
	std::string charID;
	float jonbScaleFactor = 1.0;
	bool hasFailed = false;
	float hipOffsetX = 0.0;
	float hipOffsetY = 0.0;
	std::string prefix;
	std::string name;
	int i1 = 0;
	int i2 = 0;
	uint16_t imageNum = 0;
	uint16_t hurtboxCount = 0;
	uint16_t hitboxCount = 0;
	uint16_t snapCount1 = 0;
	uint16_t snapCount2 = 0;
	uint16_t snapCount3 = 0;
	uint16_t snapCount4 = 0;
	uint16_t snapCount5 = 0;
	bool specToSnap = false;

	void buildImages(std::ofstream& file, const Frame* frame, std::string id, std::string prefix);
	void buildHeader(std::ofstream& file, const Sequence* seq, const int fraNum, std::string id, std::string prefix);
	void buildChunks(std::ofstream& file, const Sequence* seq);
	void buildBoxes(std::ofstream& file, const Sequence* seq);


};


#endif /* MAINFRAME_H_GUARD */
