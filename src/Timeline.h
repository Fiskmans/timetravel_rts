
#include <cstdint>
#include <vector>
#include <functional>

#include "tools/Event.h"

namespace fisk
{

	class Event
	{
	public:
		Event(uint64_t aAt, std::function<void()> aCallback);

		bool operator<(const Event& aOther) const;
		uint64_t GetWhen();

		void Resolve();
	private:
		uint64_t myAt;
		std::function<void()> myCallback;
	};

	class Action
	{
	public:
		Action(uint64_t aAt);

		virtual Event AsEvent() = 0;
		uint64_t GetWhen();
	private:
		uint64_t myAt;
	};

	class Timeline;

	class Unit
	{
	public:
		enum class Team
		{
			Friend,
			Foe
		};

		Unit(Team aTeam);
		
		void OnSpawned(Timeline& aTimeline);

		void Reset();
		bool IsDead();
		void Damage(int aAmount);
		void Attack(Timeline& aTimeline);
		float GetHealthPercent();

		tools::Event<> OnDeath;

		Team myTeam;
		std::string myName;

		static const int myHealth = 15;
		int myDamage = 0;
	};


	class SpawnAction : public Action
	{
	public:
		SpawnAction(uint64_t aAt, Timeline& aTimeline, Unit::Team aTeam);

		Event AsEvent() override;

	private:
		Timeline& myTimeline;
		std::shared_ptr<Unit> myUnit;
	};


	class Timeline
	{
	public:
		uint64_t GetTime(uint64_t aOffset = 0);

		void ImguiDrawTimeline();
		void Advance();

		void SpawnFriend();
		void SpawnFoe();

		void AddUnit(std::shared_ptr<Unit> aUnit);

		void QueueEvent(Event aEvent);

		std::vector<std::shared_ptr<Unit>>& GetUnits();
	private:

		void FlushPendingEvents();
		void Reset();
		void Goto(uint64_t aTime);

		uint64_t myMaxTime = 0;
		uint64_t myNow = 0;

		std::vector<Event> myEventsToAdd;
		std::vector<Event> myEvents;
		std::vector<Action*> myActions;
		std::vector<std::shared_ptr<Unit>> myUnits;

		std::vector<tools::EventReg> myEventHandles;
	};
}