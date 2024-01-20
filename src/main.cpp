
#include "GraphicsFramework.h"
#include "ImguiHelper.h"
#include "MatrixUtils.h"
#include "Model.h"
#include "RenderCall.h"
#include "ShaderBuffer.h"
#include "Shaders.h"
#include "VertexBuffer.h"
#include "Window.h"
#include "Gameworld.h"

#include "Arcospheres.h"

#include "tools/Logger.h"

#include "imgui/imgui.h"

#include <vector>
#include <chrono>

int main(int argc, char** argv)
{
	fisk::Vertex::Validate();

	fisk::tools::SetFilter(fisk::tools::Type::All);
	fisk::tools::SetHalting(fisk::tools::Type::AnyError);

	

	fisk::Window window(fisk::tools::V2ui(108,902));
	fisk::GraphicsFramework graphicsFramework(window);

	fisk::ImguiHelper imguiHelper(graphicsFramework, window);

	fisk::GameWorld gameworld(imguiHelper);

	fisk::Shaders shaders(graphicsFramework);

	fisk::COMObject<ID3D11PixelShader> pixelShader = shaders.GetPixelShaderByPath("shaders/pixel/Depth.hlsl");
	fisk::COMObject<ID3D11VertexShader> vertexShader = shaders.GetVertexShaderByPath("shaders/vertex/SimpleTransform.hlsl");
	fisk::COMObject<ID3D11InputLayout> inputLayout = shaders.GetInputLayout("shaders/vertex/SimpleTransform.hlsl", fisk::VertexView::Layout());

	fisk::COMObject<ID3D11Buffer> shaderBuffer = shaders.GetShaderBuffer(fisk::ShaderBuffer::DataSize);

	std::optional<fisk::Model> model = fisk::Model::FromFile(graphicsFramework, "models/Cube_3d_printing_sample.stl");


	auto ArcoState = [](const arcospheres::State& aState)
	{
		ImDrawList* list = ImGui::GetWindowDrawList();
	};

	static arcospheres::FuturePath* path = nullptr;

	static arcospheres::State BaseState;

	for (size_t i = 0; i < arcospheres::Count; i++)
		BaseState.myCounts[i] = 5;

	static fisk::tools::V4f backgroundColor = fisk::tools::V4f(0.2,0.2,0.2,1.f);
	static arcospheres::Operation FoldingDataOutcome1 = { .myTakes = { arcospheres::Lambda, arcospheres::Xi }, .myMakes = { arcospheres::Zeta, arcospheres::Theta } };
	static arcospheres::Operation FoldingDataOutcome2 = { .myTakes = { arcospheres::Lambda, arcospheres::Xi }, .myMakes = { arcospheres::Epsilon, arcospheres::Phi } };
	static arcospheres::Operation WarpingDataOutcome1 = { .myTakes = { arcospheres::Epsilon, arcospheres::Phi }, .myMakes = { arcospheres::Zeta, arcospheres::Theta} };
	static arcospheres::Operation WarpingDataOutcome2 = { .myTakes = { arcospheres::Epsilon, arcospheres::Phi }, .myMakes = { arcospheres::Omega, arcospheres::Gamma} };
	static arcospheres::Operation DilationDataOutcome1 = { .myTakes = { arcospheres::Zeta, arcospheres::Omega }, .myMakes = { arcospheres::Lambda, arcospheres::Lambda} };
	static arcospheres::Operation DilationDataOutcome2 = { .myTakes = { arcospheres::Zeta, arcospheres::Omega }, .myMakes = { arcospheres::Phi, arcospheres::Phi} };
	static arcospheres::Operation InjectionDataOutcome1 = { .myTakes = { arcospheres::Theta, arcospheres::Gamma}, .myMakes = { arcospheres::Epsilon, arcospheres::Epsilon} };
	static arcospheres::Operation InjectionDataOutcome2 = { .myTakes = { arcospheres::Theta, arcospheres::Gamma}, .myMakes = { arcospheres::Zeta, arcospheres::Zeta} };
	static arcospheres::Operation NaqTesseract1 = { .myTakes = { arcospheres::Lambda, arcospheres::Xi, arcospheres::Zeta }, .myMakes = { arcospheres::Theta, arcospheres::Epsilon, arcospheres::Phi } };
	static arcospheres::Operation NaqTesseract2 = { .myTakes = { arcospheres::Lambda, arcospheres::Xi, arcospheres::Zeta }, .myMakes = { arcospheres::Phi, arcospheres::Gamma, arcospheres::Omega } };
	static int StepSpeed = 1000;

	static int amount[10] = { 0 };

	fisk::tools::EventReg imguiDragRegistration = imguiHelper.DrawImgui.Register([]()
	{
		ImGui::Begin("Test");

		bool modified = false;

		if (ImGui::BeginTable("inputs", 4))
		{

			ImGui::TableNextColumn();
			modified |= ImGui::InputInt("Folding A", &amount[0]);
			ImGui::TableNextColumn();
			modified |= ImGui::InputInt("Warping A", &amount[2]);
			ImGui::TableNextColumn();
			modified |= ImGui::InputInt("Dilation A", &amount[4]);
			ImGui::TableNextColumn();
			modified |= ImGui::InputInt("Injection A", &amount[6]);

			ImGui::TableNextRow();
			ImGui::TableNextColumn();
			modified |= ImGui::InputInt("Folding B", &amount[1]);
			ImGui::TableNextColumn();
			modified |= ImGui::InputInt("Warping B", &amount[3]);
			ImGui::TableNextColumn();
			modified |= ImGui::InputInt("Dilation B", &amount[5]);
			ImGui::TableNextColumn();
			modified |= ImGui::InputInt("Injection B", &amount[7]);


			ImGui::TableNextRow();
			ImGui::TableNextColumn();
			modified |= ImGui::InputInt("Naq tesseract A", &amount[8]);
			
			ImGui::TableNextRow();
			ImGui::TableNextColumn();
			modified |= ImGui::InputInt("Naq tesseract B", &amount[9]);

			ImGui::EndTable();
		}

		arcospheres::State fromState = BaseState;

		for (int i = 0; i < amount[0]; i++)
			fromState = *fromState.Modify(FoldingDataOutcome1);
		for (int i = 0; i < amount[1]; i++)
			fromState = *fromState.Modify(FoldingDataOutcome2);
		for (int i = 0; i < amount[2]; i++)
			fromState = *fromState.Modify(WarpingDataOutcome1);
		for (int i = 0; i < amount[3]; i++)
			fromState = *fromState.Modify(WarpingDataOutcome2);
		for (int i = 0; i < amount[4]; i++)
			fromState = *fromState.Modify(DilationDataOutcome1);
		for (int i = 0; i < amount[5]; i++)
			fromState = *fromState.Modify(DilationDataOutcome2);
		for (int i = 0; i < amount[6]; i++)
			fromState = *fromState.Modify(InjectionDataOutcome1);
		for (int i = 0; i < amount[7]; i++)
			fromState = *fromState.Modify(InjectionDataOutcome2);
		for (int i = 0; i < amount[8]; i++)
			fromState = *fromState.Modify(NaqTesseract1);
		for (int i = 0; i < amount[9]; i++)
			fromState = *fromState.Modify(NaqTesseract2);

		if (modified)
		{
			if (path)
				delete path;

			path = new arcospheres::FuturePath(fromState, BaseState);
		}

		ImGui::InputInt("Steps per frame", &StepSpeed);

		arcospheres::State at = fromState;

		ImGui::Indent();
		ImGui::Text(at.ToString(5).c_str());
		ImGui::Unindent();
		ImGui::Separator();

		if (path)
		{
			path->Step(StepSpeed);

			ImGui::Text("Steps: %u", path->mySteps);

			if (path->Done())
			{
				ImGui::TextColored(ImColor(0, 255, 0), "Done");
				ImGui::Text("Length: %u", path->GetResult().size());

				ImGui::Text(arcospheres::PathToString(path->GetResult()).c_str());
				ImGui::Separator();

				for (arcospheres::OperationId op : path->GetResult())
				{
					at = *at.Modify(arcospheres::BaseOperations[op]);

					ImGui::Text(arcospheres::OperationToString(op).c_str());
					ImGui::Indent();
					ImGui::Text(at.ToString(5).c_str());
					ImGui::Unindent();
				}

			}
			if (path->Failed())
				ImGui::TextColored(ImColor(255, 0, 0), "Destination unreachable");
		}

		ImGui::End();
	});

	using clock = std::chrono::steady_clock;

	clock::time_point lastFrame = clock::now();

	while (window.ProcessEvents())
	{
		clock::time_point now = clock::now();
		float dt = std::chrono::duration_cast<std::chrono::duration<float, std::ratio<1, 1>>>(now - lastFrame).count();

		lastFrame = now;

		fisk::COMObject<ID3D11RenderTargetView> renderTarget = graphicsFramework.GetBackBufferRenderTarget();

		fisk::ShaderBuffer data;
		fisk::ShaderBufferView view = data.Structure();

		fisk::ClearTexture clearTexture(renderTarget, backgroundColor);
		fisk::ClearDepth clearDepth;
		fisk::RenderModel modelRenderCall(vertexShader, pixelShader, *model, inputLayout, renderTarget, shaderBuffer, data.AsGeneric());

		clearTexture.Render(graphicsFramework);
		clearDepth.Render(graphicsFramework);
		modelRenderCall.Render(graphicsFramework);

		graphicsFramework.Present(fisk::GraphicsFramework::VSyncState::OnVerticalBlank);
	}

	system("pause");

}