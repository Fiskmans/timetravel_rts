#pragma once

#include "ImguiHelper.h"
#include "Timeline.h"

namespace fisk
{
	class GameWorld
	{
	public:
		GameWorld(ImguiHelper& aImgui);

	private:
		void Imgui();

		Timeline myTimeline;

		tools::EventReg myImgui;
	};
} 