#include "Timeline.h"
#include "imgui/imgui.h"
#include <algorithm>

namespace fisk
{
	Action::Action(uint64_t aAt)
	{
		myAt = aAt;
	}

	uint64_t Action::GetWhen()
	{
		return myAt;
	}

	uint64_t Timeline::GetTime(uint64_t aOffset)
	{
		return myNow + aOffset;
	}

	void Timeline::ImguiDrawTimeline()
	{

		ImDrawList* drawlist = ImGui::GetWindowDrawList();
		float width = 500;

		ImVec2 topleft = ImGui::GetCursorScreenPos();

		auto getX = [&](uint64_t aTime)
		{
			return topleft.x + static_cast<float>(aTime) / static_cast<float>(myMaxTime) * width;
		};

		auto poi = [&](uint64_t aTime, const char* aName, float aImportance, ImColor aColor = ImColor(200, 200, 200))
		{
			float x = getX(aTime);
			drawlist->AddLine(ImVec2(x, topleft.y), ImVec2(x, topleft.y + aImportance), aColor);
			drawlist->AddText(ImVec2(x, topleft.y + aImportance), aColor, aName);
		};

		drawlist->AddLine(topleft, ImVec2(topleft.x + width, topleft.y), ImColor(255, 255, 255));

		poi(myNow, "Now", 40);

		for (Action* act : myActions)
		{
			poi(act->GetWhen(), "S", 20, ImColor(255, 100, 70, 128));
		}

		for (Event e : myEvents)
		{
			poi(e.GetWhen(), "e", 10);
		}


		ImGui::Dummy(ImVec2(width, 50));
		ImGui::Separator();

		if (ImGui::Button("Go back"))
		{
			uint64_t target = 0;

			if (myNow > 600)
			{
				target = myNow - 600;
			}
			Reset();
			Goto(target);
		}

		if (ImGui::Button("Spawn Friend"))
			SpawnFriend();
		if (ImGui::Button("Spawn Foe"))
			SpawnFoe();

		ImGui::Separator();

		ImGui::Text("Units");

		for (std::shared_ptr<Unit> unit : myUnits)
		{
			switch (unit->myTeam)
			{
			case Unit::Team::Foe:
				ImGui::PushStyleColor(ImGuiCol_Button, ImColor(255, 100, 100).Value);
				ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImColor(255, 120, 120).Value);
				ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImColor(255, 150, 150).Value);
				break;
			}


			ImGui::Button(unit->myName.c_str());
			ImGui::SameLine();

			ImGui::PushStyleColor(ImGuiCol_ScrollbarGrab, ImColor(100, 255, 150).Value);
			ImGui::ProgressBar(unit->GetHealthPercent());
			ImGui::PopStyleColor(1);

			switch (unit->myTeam)
			{
			case Unit::Team::Foe:
				ImGui::PopStyleColor(3);
				break;
			}
		}
	}

	void Timeline::Advance()
	{
		myMaxTime++;
		Goto(myNow + 1);
	}

	void Timeline::SpawnFriend()
	{
		Action* spawn = new SpawnAction(myNow, *this, Unit::Team::Friend);
		myActions.push_back(spawn);

		QueueEvent(spawn->AsEvent());
	}

	void Timeline::SpawnFoe()
	{
		Action* spawn = new SpawnAction(myNow, *this, Unit::Team::Foe);
		myActions.push_back(spawn);

		QueueEvent(spawn->AsEvent());
	}

	void Timeline::AddUnit(std::shared_ptr<Unit> aUnit)
	{
		myUnits.push_back(aUnit);
		myEventHandles.push_back(aUnit->OnDeath.Register(
			[this, aUnit]()
		{
			QueueEvent(Event(myNow, [this, aUnit]()
			{
				myUnits.erase(std::find(myUnits.begin(), myUnits.end(), aUnit));
			}));
		}));

		aUnit->OnSpawned(*this);
	}

	void Timeline::QueueEvent(Event aEvent)
	{
		myEventsToAdd.push_back(aEvent);
	}

	std::vector<std::shared_ptr<Unit>>& Timeline::GetUnits()
	{
		return myUnits;
	}


	void Timeline::Goto(uint64_t aTime)
	{
		FlushPendingEvents();

		while (!myEvents.empty())
		{
			if (myEvents[0].GetWhen() > aTime)
				break;

			myNow = myEvents[0].GetWhen();

			myEvents[0].Resolve();
			std::pop_heap(myEvents.begin(), myEvents.end());
			myEvents.pop_back();

			FlushPendingEvents();
		}

		myNow = aTime;
	}

	void Timeline::FlushPendingEvents()
	{
		for (Event e : myEventsToAdd)
		{
			myEvents.push_back(e);
			std::push_heap(myEvents.begin(), myEvents.end());
		}
		myEventsToAdd.clear();
	}

	void Timeline::Reset()
	{
		myUnits.clear();
		myEvents.clear();
		myEventHandles.clear();
		myNow = 0;

		for (Action* act : myActions)
			myEvents.push_back(act->AsEvent());

		std::make_heap(myEvents.begin(), myEvents.end());
	}


	Event::Event(uint64_t aAt, std::function<void()> aCallback)
	{
		myAt = aAt;
		myCallback = aCallback;
	}

	bool Event::operator<(const Event& aOther) const
	{
		return aOther.myAt < myAt;
	}

	uint64_t Event::GetWhen()
	{
		return myAt;
	}

	void Event::Resolve()
	{
		myCallback();
	}

	Unit::Unit(Team aTeam)
	{
		static int counter = 0;
		const char* names[] = {
			"Bob",
			"Alice",
			"Tamara",
			"Pogo",
			"Tammy",
			"Benny",
			"Job",
			"Craig"
		};

		myName = names[counter++ % (sizeof(names) / sizeof(*names))];
		myTeam = aTeam;
	}

	void Unit::OnSpawned(Timeline& aTimeline)
	{
		Attack(aTimeline);
	}

	void Unit::Reset()
	{
		myDamage = 0;
	}

	bool Unit::IsDead()
	{
		return myDamage >= myHealth;
	}

	void Unit::Damage(int aAmount)
	{
		if (IsDead())
			return;
		
		myDamage += aAmount;

		if (IsDead())
			OnDeath.Fire();
	}

	void Unit::Attack(Timeline& aTimeline)
	{
		if (IsDead())
			return;

		aTimeline.QueueEvent(Event(aTimeline.GetTime(100), [this, &aTimeline]()
		{
			Attack(aTimeline);
		}));

		for (std::shared_ptr<Unit> unit : aTimeline.GetUnits())
		{
			if (unit->myTeam != myTeam)
			{
				unit->Damage(1);
				return;
			}
		}
	}

	float Unit::GetHealthPercent()
	{
		return 1.f - static_cast<float>(myDamage) / static_cast<float>(myHealth);
	}

	SpawnAction::SpawnAction(uint64_t aAt, Timeline& aTimeline, Unit::Team aTeam)
		: Action(aAt)
		, myTimeline(aTimeline)
		, myUnit(std::make_shared<Unit>(aTeam))
	{

	}

	Event SpawnAction::AsEvent()
	{
		return Event(GetWhen(), [this]()
		{
			myUnit->Reset();
			myTimeline.AddUnit(myUnit);
		});
	}
}