#include "main_pane.h"
#include "pattern_disp.h"
#include "frame_disp.h"
#include <imgui.h>


MainPane::MainPane(Render* render, FrameData *framedata, FrameState &fs, Parts *part) : DrawWindow(render, framedata, fs, part),
decoratedNames(nullptr)
{
	
}

void MainPane::RegenerateNames()
{
	delete[] decoratedNames;
	
	if(parts && parts->loaded)
	{
		decoratedNames = new std::string[frameData->get_sequence_count()];
		int count = frameData->get_sequence_count();

		for(int i = 0; i < count; i++)
		{
			decoratedNames[i] = frameData->GetDecoratedName(i);
		}
	}
	else
		decoratedNames = nullptr;
}

void MainPane::Draw()
{	
	namespace im = ImGui;
	im::Begin("Left Pane",0);
	if(parts->loaded)
	{
		frameData->updateSpr(sprNum);
		//if(im::BeginCombo("Sprite", ))
		if (im::TreeNode("Sprite Data"))
		{
			im::SetNextItemWidth(125.0f);
			im::InputInt("Sprite", &sprNum);
			im::SameLine();
			im::SetNextItemWidth(125.0f);
			im::InputInt("Layer", &layerNum);
			
			if (parts->GetPr(sprNum, layerNum) == nullptr)
				im::Text("Sprite and/or Layer doesnt exist");
			else
				PRDisplay(parts->GetPr(sprNum, layerNum), &parts->groupNames[sprNum]);

			im::Separator();

			if (im::Button("New Sprite")) parts->newSprite(sprNum);
			im::SameLine();
			if (im::Button("New Layer")) parts->newLayer(sprNum, layerNum);
			im::TreePop();
		}

		if (im::TreeNode("Particle Data"))
		{
			im::InputInt("Particle", &ptcNum);

			im::Separator();

			if (parts->GetPP(ptcNum) == nullptr)
				im::Text("Particle doesnt exist");
			else
				PPDisplay(parts->GetPP(ptcNum));
		}
		/*
		auto seq = frameData->get_sequence(currState.pattern);
		if(seq)
		{
			int nframes = seq->frames.size() - 1;
			if(nframes >= 0)
			{			
				float spacing = im::GetStyle().ItemInnerSpacing.x;
				im::SetNextItemWidth(im::GetWindowWidth() - 160.f);
				im::SliderInt("##frameSlider", &currState.frame, 0, nframes);
				im::SameLine();
				im::PushButtonRepeat(true);
				if(im::ArrowButton("##left", ImGuiDir_Left))
					currState.frame--;
				im::SameLine(0.0f, spacing);
				if(im::ArrowButton("##right", ImGuiDir_Right))
					currState.frame++;
				im::PopButtonRepeat();
				im::SameLine();
				im::Text("%d/%d", currState.frame+1, nframes+1);

				if(currState.frame < 0)
					currState.frame = 0;
				else if(currState.frame > nframes)
					currState.frame = nframes;
				
				if(im::Button("Animate"))
				{
					currState.animating = !currState.animating;
					currState.animeSeq = currState.pattern;
				}
			}
			else
			{
				im::Text("This pattern has no frames.");
				if(im::Button("Add frame"))
				{
					seq->frames.push_back({});
					seq->frames.back().AF.layers.resize(3);
					currState.frame = 0;
				}
			}

			im::BeginChild("FrameInfo", {0, im::GetWindowSize().y-im::GetFrameHeight()*4-8}, false, ImGuiWindowFlags_HorizontalScrollbar);
			if (im::TreeNode("Pattern data"))
			{
				if(PatternDisplay(seq))
				{
					decoratedNames[currState.pattern] = frameData->GetDecoratedName(currState.pattern);
				}

				if(im::Button("Copy pattern")){
					currState.copied->pattern = *seq;
				}
				im::SameLine(0,20.f); 
				if(im::Button("Paste pattern")){
					*seq = currState.copied->pattern;
					decoratedNames[currState.pattern] = frameData->GetDecoratedName(currState.pattern);
					nframes = seq->frames.size() - 1;
				}

				if(im::Button("Push pattern copy"))
				{
					patCopyStack.push_back(SequenceWId{currState.pattern, *seq});
				}
				im::SameLine(0,20.f);
				if(im::Button("Pop all and paste"))
				{
					PopCopies();
					RegenerateNames();
					nframes = seq->frames.size() - 1;
				}
				im::SameLine(0,20.f);
				im::Text("%llu copies", patCopyStack.size());

				im::TreePop();
				im::Separator();
			}
			if(nframes >= 0)
			{
				if(currState.frame > nframes)
					currState.frame = nframes;
				Frame &frame = seq->frames[currState.frame];
				if(im::TreeNode("State data"))
				{
					AsDisplay(&frame.AS);
					if(im::Button("Copy AS")){
						currState.copied->as = frame.AS;
					}

					im::SameLine(0,20.f); 
					if(im::Button("Paste AS")){
						frame.AS = currState.copied->as;
					}
					im::TreePop();
					im::Separator();
				}
				if (im::TreeNode("Animation data"))
				{
					AfDisplay(&frame.AF, currState.selectedLayer);
					if(im::Button("Copy AF")){
						currState.copied->af = frame.AF;
					}

					im::SameLine(0,20.f); 
					if(im::Button("Paste AF")){
						frame.AF = currState.copied->af;
					}
					im::TreePop();
					im::Separator();
				}
				if (im::TreeNode("Tools"))
				{
					im::Checkbox("Copy current frame when inserting", &copyThisFrame);
					
					if(im::Button("Append frame"))
					{
						if(copyThisFrame)
							seq->frames.push_back(frame);
						else
						{
							seq->frames.push_back({});
							seq->frames.back().AF.layers.resize(3);
						}
					}

					im::SameLine(0,20.f); 
					if(im::Button("Insert frame"))
					{
						if(copyThisFrame)
							seq->frames.insert(seq->frames.begin()+currState.frame, frame);
						else
						{
							seq->frames.insert(seq->frames.begin()+currState.frame, 1, {});
							seq->frames[currState.frame].AF.layers.resize(3);
						}
					}

					im::SameLine(0,20.f);
					if(im::Button("Range tool"))
					{
						afjcRangeVal = 0;
						ranges[0] = 0;
						ranges[1] = 0;
						rangeWindow = !rangeWindow;
					}

					if(im::Button("Copy frame"))
					{
						currState.copied->frame = frame;
					}

					im::SameLine(0,20.f);
					if(im::Button("Paste frame"))
					{
						frame = currState.copied->frame;
					}

					if(im::Button("Delete frame"))
					{
						seq->frames.erase(seq->frames.begin()+currState.frame);
						if(currState.frame >= seq->frames.size())
							currState.frame--;
					}
					
					im::TreePop();
					im::Separator();
				}
			}
			else
				rangeWindow = false;
			im::EndChild();
		} */
	}
	else
		im::Text("Load some data first.");

	im::End();
	/*
	if(rangeWindow)
	{
		auto seq = frameData->get_sequence(currState.pattern);
		if(ranges[0] < 0)
			ranges[0] = 0;
		if(ranges[1] < 0)
			ranges[1] = 0;
		if(ranges[0] > ranges[1])
			ranges[0] = ranges[1];
		if(ranges[0] + ranges[1] == 0)
			ranges[1] = seq->frames.size()-1;

		im::SetNextWindowSize(ImVec2{400,120}, ImGuiCond_FirstUseEver);
		im::Begin("Range paste", &rangeWindow);
		im::InputInt2("Range of frames", ranges);
		if(im::Button("Paste color"))
		{
			auto &iLayer = seq->frames[currState.frame].AF.layers[currState.selectedLayer];
			for(int i = ranges[0]; i <= ranges[1] && i >= 0 && i < seq->frames.size(); i++)
			{
				for(auto &oLayer : seq->frames[i].AF.layers)
				{
					memcpy(oLayer.rgba, iLayer.rgba, sizeof(float)*4);
					oLayer.blend_mode = iLayer.blend_mode;
				}
			}
		}
		if(im::Button("Set landing frame"))
		{
			for(int i = ranges[0]; i <= ranges[1] && i >= 0 && i < seq->frames.size(); i++)
				seq->frames[i].AF.landJump = afjcRangeVal;
		}
		im::SameLine(); im::SetNextItemWidth(100);
		im::InputInt("Value##AFJC", &afjcRangeVal);

		if(im::Button("Copy frames"))
		{
			currState.copied->frames.clear();
			for(int i = ranges[0]; i <= ranges[1] && i >= 0 && i < seq->frames.size(); i++)
				currState.copied->frames.push_back(seq->frames[i]);
		}
		im::SameLine();
		if(im::Button("Paste frames (inserted before end range pos)"))
		{
			if(!currState.copied->frames.empty())
			{
				int pos = ranges[1];
				if(pos > seq->frames.size())
					pos = seq->frames.size();
				seq->frames.insert(seq->frames.begin()+pos, currState.copied->frames.begin(), currState.copied->frames.end());
			}
		}
		if(im::Button("Paste transform (Current layer only)"))
		{
			for(int i = ranges[0]; i <= ranges[1] && i >= 0 && i < seq->frames.size(); i++)
			{
				if(seq->frames[i].AF.layers.size() > currState.selectedLayer)
				{
					seq->frames[i].AF.layers[currState.selectedLayer].offset_x = seq->frames[currState.frame].AF.layers[currState.selectedLayer].offset_x;
					seq->frames[i].AF.layers[currState.selectedLayer].offset_y = seq->frames[currState.frame].AF.layers[currState.selectedLayer].offset_y;

					memcpy(seq->frames[i].AF.layers[currState.selectedLayer].scale, seq->frames[currState.frame].AF.layers[currState.selectedLayer].scale, sizeof(float)*2);
					memcpy(seq->frames[i].AF.layers[currState.selectedLayer].rotation, seq->frames[currState.frame].AF.layers[currState.selectedLayer].rotation, sizeof(float)*2);
				}
			}
		}
		im::End();
	}*/
}

void MainPane::PopCopies()
{
	for(auto &pat : patCopyStack)
	{
		*frameData->get_sequence(pat.id) = pat.seq;
	}
	patCopyStack.clear();
}