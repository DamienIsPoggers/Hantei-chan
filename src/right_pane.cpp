#include "right_pane.h"
#include "frame_disp.h"
#include <imgui.h>

void RightPane::Draw()
{	
	ImGui::Begin("Right Pane", 0);
	/*
	auto seq = frameData->get_sequence(currState.pattern);
	if(seq)
	{
		int nframes = seq->frames.size() - 1;
		if(nframes >= 0)
		{
			Frame &frame = seq->frames[currState.frame];
			if (ImGui::TreeNode("Attack data"))
			{
				AtDisplay(&frame.AT);
				if(im::Button("Copy AT")){
					currState.copied->at = frame.AT;
				}

				im::SameLine(0,20.f); 
				if(im::Button("Paste AT")){
					frame.AT = currState.copied->at;
				}

				ImGui::TreePop();
				ImGui::Separator();
			}
			if(ImGui::TreeNode("Effects"))
			{
				EfDisplay(&frame.EF, &currState.copied->efSingle);
				if(im::Button("Copy all")){
					CopyVectorContents<Frame_EF>(currState.copied->efGroup, frame.EF);
				}
				im::SameLine(0,20.f); 
				if(im::Button("Paste all")){
					CopyVectorContents<Frame_EF>(frame.EF, currState.copied->efGroup);
				}
				im::SameLine(0,20.f); 
				if(im::Button("Add copy")){
					frame.EF.push_back(currState.copied->efSingle);
				}
				ImGui::TreePop();
				ImGui::Separator();
			}
			if(ImGui::TreeNode("Conditions"))
			{
				IfDisplay(&frame.IF, &currState.copied->ifSingle);
				if(im::Button("Copy all")){
					CopyVectorContents<Frame_IF>(currState.copied->ifGroup, frame.IF);
				}
				im::SameLine(0,20.f); 
				if(im::Button("Paste all")){
					CopyVectorContents<Frame_IF>(frame.IF, currState.copied->ifGroup);
				}
				im::SameLine(0,20.f); 
				if(im::Button("Add copy")){
					frame.IF.push_back(currState.copied->ifSingle);
				}
				ImGui::TreePop();
				ImGui::Separator();
			}
		}
	}
	*/
	ImGui::End();
}

