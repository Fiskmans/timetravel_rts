

#include "Gameworld.h"
#include "imgui/imgui.h"

namespace fisk
{


	GameWorld::GameWorld(ImguiHelper& aImgui)
	{
		myImgui = aImgui.DrawImgui.Register(std::bind(&GameWorld::Imgui, this));
	}

	void GameWorld::Imgui()
	{
		ImGui::Begin("Gameworld");

		ImGui::Text("Time");
		myTimeline.Advance();
		myTimeline.ImguiDrawTimeline();

		ImGui::End();
	}

}