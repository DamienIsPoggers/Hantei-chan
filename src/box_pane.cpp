#include "box_pane.h"
#include <imgui.h>

constexpr int boxLimit = 33; 

BoxPane::BoxPane(Render* render, FrameData *frameData, FrameState &state, Parts *parts):
DrawWindow(render, frameData, state, parts),
currentBox(0), highlight(false)
{
	//Init box names
	for(int i = 0; i < boxLimit; i++)
	{
		if(i==0)
			boxNameList[i] = "Collision box";
		else if (i >= 1 && i <= 8)
			boxNameList[i] = "Hurtbox " + std::to_string(i);
		else if(i >=9 && i <= 10)
			boxNameList[i] = "Special box " + std::to_string(i-8);
		else if(i == 11)
			boxNameList[i] = "Clash box";
		else if(i == 12)
			boxNameList[i] = "Projectile box";
		else if(i>12 && i<=24)
			boxNameList[i] = "Special box " + std::to_string(i-8);
		else
			boxNameList[i] = "Attack box " + std::to_string(i-24);
	}

	currState.copied->box = {};
}

void BoxPane::BoxStart(int x, int y)
{
	auto seq = frameData->get_sequence(currState.pattern);
	if(seq)
	{
		auto &frames = seq->frames;
		if(frames.size()>0)
		{
			Hitbox &box = frames[currState.frame].hitboxes[currentBox];
			box.xy[0] = box.xy[2] = x;
			box.xy[1] = box.xy[3] = y;

			dragxy[0] = box.xy[0];
			dragxy[1] = box.xy[1];
		}
	}
}

void BoxPane::BoxDrag(int x, int y)
{
	auto seq = frameData->get_sequence(currState.pattern);
	if(seq)
	{
		auto &frames = seq->frames;
		if(frames.size()>0)
		{
			Hitbox &box = frames[currState.frame].hitboxes[currentBox];
			dragxy[0] += x/render->scale;
			dragxy[1] += y/render->scale;

			box.xy[2] = dragxy[0];
			box.xy[3] = dragxy[1];
		}
	}
}

void BoxPane::Draw()
{
	namespace im = ImGui;
	im::Begin("Box Pane",0);

	/*
	if(frameData->get_sequence(currState.pattern) && frameData->get_sequence(currState.pattern)->frames.size() > 0)
	{
		auto &frames = frameData->get_sequence(currState.pattern)->frames;
		
		im::SetNextItemWidth(200.f);
		if (im::BeginCombo("Box", boxNameList[currentBox].c_str(), ImGuiComboFlags_HeightLarge))
		{
			for (int n = 0; n < boxLimit; n++)
			{
				const bool is_selected = (currentBox == n);
				if (im::Selectable(boxNameList[n].c_str(), is_selected))
				{
					currentBox = n;
				}

				if (is_selected)
					im::SetItemDefaultFocus();
			}
			im::EndCombo();
		}

		im::SameLine(0.0f, 20.f);
		im::PushButtonRepeat(true);
		if(im::ArrowButton("##left", ImGuiDir_Left))
			AdvanceBox(-1);
		im::SameLine(0,im::GetStyle().ItemInnerSpacing.x);
		if(im::ArrowButton("##right", ImGuiDir_Right))
			AdvanceBox(+1);
		im::PopButtonRepeat();

		BoxList &boxes = frames[currState.frame].hitboxes;

		im::SameLine(0,20.f);
		if(im::Button("Copy all"))
		{
			currState.copied->boxes = boxes;
		}
		im::SameLine(0,20.f);
		if(im::Button("Paste all"))
		{
			if(currState.copied->boxes.size()>0)
				boxes = currState.copied->boxes;
		}

		im::Checkbox("Highlight selected", &highlight);
		im::SameLine(0,20.f);
		if(im::Button("Delete selected"))
			boxes.erase(currentBox);

		if(highlight)
			render->highLightN = currentBox;
		else
			render->highLightN = -1;

		im::SameLine(0,20.f);
		if(im::Button("Copy params"))
		{
			currState.copied->box = boxes[currentBox];
		}
		im::SameLine(0,20.f);
		if(im::Button("Paste params"))
		{
			boxes[currentBox] = currState.copied->box;
		}
		

		const int step = 8;
		im::InputScalarN("Top left", ImGuiDataType_S32, boxes[currentBox].xy, 2, &step, NULL, "%d", 0);
		im::InputScalarN("Bottom right", ImGuiDataType_S32, boxes[currentBox].xy+2, 2, &step, NULL, "%d", 0);
	}
	*/

	im::End();
}

void BoxPane::AdvanceBox(int dir)
{
	currentBox += dir;
	if(currentBox < 0)
		currentBox = boxLimit-1;
	else if(currentBox >= boxLimit)
		currentBox = 0;
}