#ifndef DRAWWINDOW_H_GUARD
#define DRAWWINDOW_H_GUARD

#include "render.h"
#include "framestate.h"

//ImGui Windows that draw themselves. Just for utility.
class DrawWindow
{
public:
	DrawWindow(Render* render, FrameData* frameData, FrameState& state, Parts* parts) :
		render(render),
		frameData(frameData),
		parts(parts),
		currState(state) {};

	FrameState &currState;
	bool isVisible = true;

	virtual void Draw() = 0;
protected:
	Render *render;
	FrameData *frameData;
	Parts* parts;
};

#endif /* DRAWWINDOW_H_GUARD */
