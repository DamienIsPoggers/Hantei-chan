#include <imgui.h>
#include "imgui_utils.h"
#include "framedata.h"
#include "parts.h"

namespace im = ImGui;

inline void PRDisplay(PartProperty *PR, std::string *name)
{
	constexpr float width = 50.f;
	
	if (PR == nullptr)
	{
		im::Text("Sprite and/or Layer doesn't exist in file.");
	}
	else {
		im::InputInt("Particle ID", &PR->ppId);
		im::Separator();

		im::SetNextItemWidth(width);
		im::DragInt("X", &PR->x);
		im::SameLine();
		im::SetNextItemWidth(width);
		im::DragInt("Y", &PR->y);

		im::Checkbox("Bilinear Filter", &PR->filter);
		im::SameLine();
		im::Checkbox("Additive Filter", &PR->additive);
		im::SameLine();
		im::Checkbox("Flip", (bool*)&PR->flip);
		im::ColorEdit4("Color", PR->rgba);
		PR->bgra[0] = PR->rgba[2] * 255;
		PR->bgra[1] = PR->rgba[1] * 255;
		PR->bgra[2] = PR->rgba[0] * 255;
		PR->bgra[3] = PR->rgba[3] * 255;
		im::ColorEdit3("Color Overlay", PR->addRgba);
		PR->addColor[0] = PR->addRgba[2] * 255;
		PR->addColor[1] = PR->addRgba[1] * 255;
		PR->addColor[2] = PR->addRgba[0] * 255;

		im::DragFloat3("Rot XYZ", PR->rotation, 0.005);
		float* scale[2]{ &PR->scaleX, &PR->scaleY };
		im::DragFloat2("Scale", *scale, 0.1);
		im::InputInt("Layer priority", &PR->priority);
	}
}

inline void PPDisplay(CutOut* pp)
{
	constexpr float width = 50.f;

	im::SetNextItemWidth(width);
	im::DragInt("UV X", &pp->uv[0]);
	im::SameLine();
	im::SetNextItemWidth(width);
	im::DragInt("UV Y", &pp->uv[1]);

	im::SetNextItemWidth(width);
	im::DragInt("UV Width", &pp->uv[2]);
	im::SameLine();
	im::SetNextItemWidth(width);
	im::DragInt("UV Height", &pp->uv[3]);

	im::Spacing();
	im::SetNextItemWidth(width);
	im::DragInt("X", &pp->xy[0]);
	im::SameLine();
	im::SetNextItemWidth(width);
	im::DragInt("Y", &pp->xy[1]);

	im::Spacing();
	float wh[2] = { pp->wh[0], pp->wh[1] };
	im::DragFloat2("Scale", wh);
	pp->wh[0] = wh[0];
	pp->wh[1] = wh[1];

	im::SetNextItemWidth(width * 3);
	im::InputInt("Reference Page", &pp->texture);

	im::SetNextItemWidth(width * 2);
	im::InputInt("Shape", &pp->shapeIndex);
	im::SameLine();
	im::SetNextItemWidth(width * 2);
	int color = pp->colorSlot;
	im::InputInt("Palette Color", &color);
	pp->colorSlot = color;

}

inline void IfDisplay(std::vector<Frame_IF> *ifList_, Frame_IF* copied)
{
	std::vector<Frame_IF> & ifList = *ifList_;
	constexpr float width = 75.f;
	int deleteI = -1;
	for( int i = 0; i < ifList.size(); i++)
	{
		if(i>0)
			im::Separator();
		im::PushID(i); 

		im::SetNextItemWidth(width); 
		im::InputInt("Type", &ifList[i].type, 0, 0); im::SameLine(0.f, 40);
		im::SetNextItemWidth(width);

		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(1,0,0,0.4));
		if(im::Button("Delete"))
		{
			deleteI = i;
		}
		ImGui::PopStyleColor();

		im::InputScalarN("##params", ImGuiDataType_S32, ifList[i].parameters, 6, NULL, NULL, "%d", 0); im::SameLine();
		if(im::Button("Copy"))
		{
			*copied = ifList[i];
		}
		im::InputScalarN("##params2", ImGuiDataType_S32, ifList[i].parameters+6, 3, NULL, NULL, "%d", 0);
		im::PopID();
	};

	if(deleteI >= 0)
		ifList.erase(ifList.begin() + deleteI);

	if(im::Button("Add"))
		ifList.push_back({});
}

inline void EfDisplay(std::vector<Frame_EF> *efList_, Frame_EF* copied)
{
	std::vector<Frame_EF> & efList = *efList_;
	constexpr float width = 50.f;
	int deleteI = -1;
	for( int i = 0; i < efList.size(); i++)
	{
		if(i>0)
			im::Separator();
		im::PushID(i); 

		im::SetNextItemWidth(width); 
		im::InputInt("Type", &efList[i].type, 0, 0); im::SameLine(0.f, 30);
		im::SetNextItemWidth(width); 
		im::InputInt("Number", &efList[i].number, 0, 0); im::SameLine(0.f, 30);

		im::SetNextItemWidth(width);
		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(1,0,0,0.4));
		if(im::Button("Delete"))
		{
			deleteI = i;
		}
		ImGui::PopStyleColor();
		
		im::InputScalarN("##params", ImGuiDataType_S32, efList[i].parameters, 6, NULL, NULL, "%d", 0); im::SameLine();
		if(im::Button("Copy"))
		{
			*copied = efList[i];
		}
		im::InputScalarN("##params2", ImGuiDataType_S32, efList[i].parameters+6, 6, NULL, NULL, "%d", 0);

		im::PopID();
	};

	if(deleteI >= 0)
		efList.erase(efList.begin() + deleteI);
	
	if(im::Button("Add"))
		efList.push_back({});
}

inline void AsDisplay(Frame_AS *as)
{
	const char* const stateList[] = {
		"Standing",
		"Airborne",
		"Crouching"
	};

	const char* const cancelList[] = {
		"Never",
		"On hit",
		"Always",
		"On successful hit"
	};

	const char* const counterList[] = {
		"No change",
		"High counter",
		"Low counter",
		"Clear"
	};

	const char* const invulList[] = {
		"None",
		"High and mid",
		"Low and mid",
		"All but throw",
		"Throw only",
		"Unknown"
	};

	constexpr float width = 75.f;

	unsigned int flagIndex = -1;
	BitField("Movement Flags", &as->movementFlags, &flagIndex, 8);
	switch (flagIndex)
	{
		case 0: Tooltip("Set Y"); break;
		case 1: Tooltip("Add Y"); break;
		case 4: Tooltip("Set X"); break;
		case 5: Tooltip("Add X"); break;
	}

	im::SetNextItemWidth(width*2);
	im::InputInt2("Speed", as->speed); im::SameLine(0.f, 20); im::SetNextItemWidth(width); 
	im::InputInt("Max X speed", &as->maxSpeedX, 0, 0);
	im::SetNextItemWidth(width*2);
	im::InputInt2("Accel", as->accel);
	
	im::Separator();
	flagIndex = -1;
	BitField("Flagset 1", &as->statusFlags[0], &flagIndex);
	switch (flagIndex)
	{
		case 0: Tooltip("Vector influences other animations (dash momentum)"); break;
		case 1: Tooltip("Force clean vector (kill dash momentum)"); break;
		case 2: Tooltip("Don't transition to walking"); break;
		case 4: Tooltip("Can ground tech"); break;
		case 5: Tooltip("Unknown 5"); break;
		case 8: Tooltip("Unknown 8"); break;
		case 9: Tooltip("Unknown 9"); break;
		case 12: Tooltip("Unknown 12"); break;
		case 31: Tooltip("Vector initialization only at the beginning (?)"); break;
	}
	
	flagIndex = -1;
	BitField("Flagset 2", &as->statusFlags[1], &flagIndex);
	switch (flagIndex)
	{
		case 0: Tooltip("Can always EX cancel"); break;
		case 2: Tooltip("Can only jump cancel"); break;
		case 31: Tooltip("Can't block"); break;
	}

	im::SetNextItemWidth(width);
	im::InputInt("Number of hits", &as->hitsNumber, 0, 0); im::SameLine(0,20.f);
	im::SetNextItemWidth(width);
	im::InputInt("ASCF", &as->ascf, 0, 0);
	im::Checkbox("Player can move", &as->canMove); //
	im::Combo("State", &as->stanceState, stateList, IM_ARRAYSIZE(stateList));
	im::Combo("Invincibility", &as->invincibility, invulList, IM_ARRAYSIZE(invulList));
	im::Combo("Counterhit", &as->counterType, counterList, IM_ARRAYSIZE(counterList)); 
	im::Combo("Cancel normal", &as->cancelNormal, cancelList, IM_ARRAYSIZE(cancelList));
	im::Combo("Cancel special", &as->cancelSpecial, cancelList, IM_ARRAYSIZE(cancelList));
	

	im::Separator();
	flagIndex = -1;
	BitField("Sine flags", &as->sineFlags, &flagIndex, 8);
	switch (flagIndex)
	{
		case 0: Tooltip("Use Y"); break;
		case 4: Tooltip("Use X"); break;
	}
	im::InputInt4("Sinewave", as->sineParameters); im::SameLine();
	im::TextDisabled("(?)");
	if(im::IsItemHovered())
		Tooltip("Sine parameters:\nX dist, Y dist\nX frequency, Y frequency");
	im::InputFloat2("Phases", as->sinePhases);
}

inline void AtDisplay(Frame_AT *at)
{
	const char* const hitEffectList[] = {
		"Weak punch",
		"Medium punch",
		"Strong punch",
		"Weak kick",
		"Medium kick",
		"Strong kick",
		"Super punch",
		"Super kick",
		"Old cut (slash)",
		"Burn",
		"Freeze",
		"Shock",
		"Big flash (SE)",
		"Small flash (SE)",
		"None",
		"Strong hit",
		"Double slash",
		"Super slash",
		"Weak cut",
		"Medium cut",
		"Strong cut",
		"Faint wave",
		//Are there more is it OOB?
	};

	const char* const addedEffectList[] = {
		"None",
		"Burn",
		"Freeze",
		"Shock",
		"Confusion"
	};

	const char* const vectorFlags[] = {
		"Default",
		"Nullify",
		"Reverse",
		"Both"
	};

	const char* const hitStopList[] = {
		"Weak",
		"Medium",
		"Strong",
		"None",
		"Stronger",
		"Strongest",
		"Weakest"
	};
	
	constexpr float width = 75.f;
	unsigned int flagIndex = -1;

	BitField("Guard Flags", &at->guard_flags, &flagIndex);
	switch (flagIndex)
	{
		case 0: Tooltip("Stand blockable"); break;
		case 1: Tooltip("Air blockable"); break;
		case 2: Tooltip("Crouch blockable"); break;
		case 8: Tooltip("Miss if enemy is standing"); break;
		case 9: Tooltip("Miss if enemy is airborne"); break;
		case 10: Tooltip("Miss if enemy is crouching"); break;
		case 11: Tooltip("Miss if enemy is in hitstun"); break;
		case 12: Tooltip("Miss if enemy is in blockstun"); break;
		case 13: Tooltip("Miss if OTG"); break;
		case 14: Tooltip("Hit only in hitstun?"); break;
		case 15: Tooltip("Can't hit playable character?"); break;
	}

	flagIndex = -1;
	BitField("Hit Flags", &at->otherFlags, &flagIndex);
	switch (flagIndex)
	{
		case 0: Tooltip("Enable chip damage"); break;
		case 1: Tooltip("Can't KO"); break;
		case 2: Tooltip("Make enemy unhittable"); break;
		case 3: Tooltip("Can't be clashed with"); break;
		case 4: Tooltip("Auto super jump cancel"); break;
		case 5: Tooltip("Don't increase combo counter"); break;
		case 6: Tooltip("Shake the screen on hit"); break;
		case 7: Tooltip("Not air techable"); break;
		case 8: Tooltip("Not ground techable (HKD)"); break;
		case 9: Tooltip("Friendly fire?"); break;
		case 10: Tooltip("No self hitstop"); break;

		case 12: Tooltip("Can't be bursted"); break;
		case 13: Tooltip("Can't be shielded"); break;
		case 14: Tooltip("Can't critical"); break;
		
		case 16: Tooltip("Use custom blockstop"); break;
		case 17: Tooltip("OTG Relaunches"); break;
		case 18: Tooltip("Can't counterhit"); break;
		case 19: Tooltip("Unknown 19"); break;
		case 20: Tooltip("Unknown 20"); break;
		case 21: Tooltip("Unknown 21"); break;
		case 22: Tooltip("Unknown 22. Affects ATUH"); break;

		//Not in AACC.
		case 25: Tooltip("No hitstop on multihit?"); break; 
		case 29: Tooltip("Block enemy blast during Stun?"); break;
	}

	flagIndex = -1;
	BitField("Hitgrab Flags", &at->hitgrab, &flagIndex, 8);
	switch (flagIndex)
	{
		case 0: Tooltip("Enabled"); break;
		case 1: Tooltip("Target collision"); break;
		case 2: Tooltip("Target origin"); break;
		case 3: Tooltip("Ignore player invulnerability timer"); break;
	}

	im::InputInt("Blockstop", &at->blockStopTime, 0,0);

	im::SetNextItemWidth(width);
	im::InputInt("Preset hitstop", &at->hitStop, 0,0); im::SameLine();
	//im::Combo("Hitstop", &at->hitStop, hitStopList, IM_ARRAYSIZE(hitStopList)); im::SameLine(0.f, 20);
	im::SetNextItemWidth(width);
	im::InputInt("Custom##Hitstop", &at->hitStopTime, 0,0);

	im::SetNextItemWidth(width);
	im::InputInt("Set hitstun", &at->hitStun, 0,0);  im::SameLine(0.f, 20); im::SetNextItemWidth(width);
	im::InputInt("Self hitstop", &at->addHitStun, 0,0);

	im::InputInt3("Hitstun decay", at->hitStunDecay);
	im::SameLine(); im::TextDisabled("(?)");
	if(im::IsItemHovered())
		Tooltip(R"_(1. Hit stun time reduction.
2. Combopoint set if combo starter
3. Combopoint SMP modifier (up to
255, higher is more extreme effect))_");
		
	im::SetNextItemWidth(width);
	im::InputInt("Untech time", &at->untechTime, 0,0);  im::SameLine(0.f, 20); im::SetNextItemWidth(width);
	im::InputInt("Circuit break time", &at->breakTime, 0,0);


	im::SetNextItemWidth(width);
	im::InputFloat("Extra gravity", &at->extraGravity, 0,0); im::SameLine(0.f, 20); im::SetNextItemWidth(width);
	im::InputInt("Correction %", &at->correction2, 0, 0);

	im::SetNextItemWidth(width);
	im::InputInt("Starter correction", &at->correction, 0, 0); im::SameLine(0.f, 20); im::SetNextItemWidth(width*2);
	im::Combo("Type##Correction", &at->correction_type, "Normal\0Multiplicative\0Substractive\0");

	im::SetNextItemWidth(width);
//	im::InputInt("VS damage", &at->red_damage, 0, 0); im::SameLine(0.f, 20); im::SetNextItemWidth(width);
	im::InputInt("Damage", &at->damage, 0, 0); im::SameLine(0.f, 20); im::SetNextItemWidth(width);
/* 
	im::SetNextItemWidth(width);
	im::InputInt("Guard damage", &at->guard_damage, 0, 0); im::SameLine(0.f, 20); im::SetNextItemWidth(width); */
	im::InputInt("Meter gain", &at->meter_gain, 0, 0); im::SetNextItemWidth(width*2);
	im::InputInt("Minimum damage %", &at->minDamage, 0, 0);

	im::Separator();
	auto comboWidth = (im::GetWindowWidth())/4.f;
	im::InputInt3("Guard Vector", at->guardVector);
	//im::InputInt3("GV flags", at->gVFlags);
	for(int i = 0; i < 3; i++)
	{	
		im::SetNextItemWidth(comboWidth);
		if(i > 0)
			im::SameLine();
		im::PushID(i); 
		im::Combo("##GFLAG", &at->gVFlags[i], vectorFlags, IM_ARRAYSIZE(vectorFlags));
		im::PopID();
	}

	im::Separator();
	im::InputInt3("Hit Vector", at->hitVector);
	//im::InputInt3("HV flags", at->hVFlags);
	im::SameLine(); im::TextDisabled("(?)");
	if(im::IsItemHovered())
		Tooltip("Stand, air and crouch.\nSee vector text file.");
	
	for(int i = 0; i < 3; i++)
	{	
		im::SetNextItemWidth(comboWidth);
		if(i > 0)
			im::SameLine();
		im::PushID(i); 
		im::Combo("##HFLAG", &at->hVFlags[i], vectorFlags, IM_ARRAYSIZE(vectorFlags));
		im::PopID();
	}
	im::Separator();
	
	im::SetNextItemWidth(150);
	im::Combo("Hit effect", &at->hitEffect, hitEffectList, IM_ARRAYSIZE(hitEffectList)); im::SameLine(0, 20.f);
	im::SetNextItemWidth(70);
	im::InputInt("ID##Hit effect", &at->hitEffect, 0, 0); 
	
	im::SetNextItemWidth(70);
	im::InputInt("Sound effect", &at->soundEffect, 0, 0); im::SetNextItemWidth(120);

	im::Combo("Added effect", &at->addedEffect, addedEffectList, IM_ARRAYSIZE(addedEffectList)); im::SameLine(0, 20.f);
	im::SetNextItemWidth(70);
	im::InputInt("ID##Added effect", &at->addedEffect, 0, 0);



}

inline void AfDisplay(Frame_AF *af, int &selectedLayer)
{
	const char* const interpolationList[] = {
		"None",
		"Linear",
		"Slow->Fast",
		"Fast->Slow",
		"Fast middle",
		"Slow middle", //Never used, but it works.
	};

	const char* const animationList[] = {
		"Go to pattern",
		"Next frame",
		"Go to frame",
		"End"
	};

	constexpr float width = 50.f;

	im::SetNextItemWidth(width*3);
	const int maxLayers = af->layers.size();
	im::SliderInt("Layer", &selectedLayer, 0, maxLayers-1);
	if(selectedLayer >= maxLayers)
		selectedLayer = maxLayers-1;
	if(selectedLayer < 0)
		selectedLayer = 0;
	
	im::SameLine(0,20.f); im::SetNextItemWidth(width/2);
	if(im::Button("Add"))
	{
		af->layers.push_back({});
			selectedLayer++;
	}
	im::SameLine(0,0.f); im::SetNextItemWidth(width/2);
	if(im::Button("Del") && maxLayers > 1)
	{
		af->layers.erase(af->layers.begin()+selectedLayer);
		if(selectedLayer >= af->layers.size())
			selectedLayer--;
	}

	auto *layer = &af->layers[selectedLayer];

	im::SetNextItemWidth(width*3);
	im::InputInt("Sprite", &layer->spriteId);
	im::SameLine(0, 20.f);
	im::Checkbox("Use .pat", &layer->usePat);

	im::Separator();

	im::SetNextItemWidth(width);
	im::DragInt("X", &layer->offset_x);
	im::SameLine();
	im::SetNextItemWidth(width);
	im::DragInt("Y", &layer->offset_y);

	int mode = layer->blend_mode-1;
	if(mode < 1)
		mode = 0;
	if (im::Combo("Blend Mode", &mode, "Normal\0Additive\0Substractive\0"))
	{
		layer->blend_mode=mode+1;
	}
	im::ColorEdit4("Color", layer->rgba);

	im::DragFloat3("Rot XYZ", layer->rotation, 0.005); 
	im::DragFloat2("Scale", layer->scale, 0.1);
	im::InputInt("Layer priority", &layer->priority);
	im::Separator();

	im::Checkbox("Rotation keeps scale set by EF", &af->AFRT);
	im::Checkbox("AFJH", &af->afjh);
	im::InputScalarN("AF Params", ImGuiDataType_U8, af->param, 4, NULL, NULL, "%hhu", 0);
	im::SetNextItemWidth(width);
	im::InputInt("Frame id", &af->frameId, 0, 0);

	unsigned int flagIndex = -1;
	BitField("Animation flags", &af->aniFlag, &flagIndex, 8);
	switch (flagIndex)
	{
		case 0: Tooltip("Land to pattern?"); break;
		case 1: Tooltip("Check loop counter"); break;
		case 2: Tooltip("Go to relative offset"); break;
		case 3: Tooltip("Relative end of loop"); break;
		case 7: Tooltip("Unknown"); break;
	}

	im::Combo("Animation", &af->aniType, animationList, IM_ARRAYSIZE(animationList));

	im::SetNextItemWidth(width);
	im::InputInt("Go to", &af->jump, 0, 0); im::SameLine(0.f, 20); im::SetNextItemWidth(width);
	im::InputInt("Landing frame", &af->landJump, 0, 0);

	im::SetNextItemWidth(width);
	im::InputInt("Z-Priority", &af->priority, 0, 0); im::SetNextItemWidth(width);
	im::InputInt("Loop N times", &af->loopCount, 0, 0); im::SameLine(0,20); im::SetNextItemWidth(width);
	im::InputInt("End of loop", &af->loopEnd, 0, 0);
	im::InputInt("Duration", &af->duration, 1, 0);

	
	im::Combo("Interpolation", &af->interpolationType, interpolationList, IM_ARRAYSIZE(interpolationList));
}