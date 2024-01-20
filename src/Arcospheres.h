#pragma once 

#include <vector>
#include <optional>
#include <array>
#include <string>
#include <unordered_map>
#include <queue>

namespace arcospheres
{

	enum Polarization : int
	{
		Lambda,
		Xi,
		Epsilon,
		Phi,
		Zeta,
		Theta,
		Gamma,
		Omega,
		Count
	};

	using Collection = std::vector<Polarization>;

	struct Operation
	{
		Collection myTakes;
		Collection myMakes;
	};

	extern std::array<Operation, 10> BaseOperations;

	using OperationId = uint8_t;

	using CompactState = uint64_t;

	struct State
	{
		static constexpr uint32_t CompactFieldBitWidth = 8;
		static constexpr uint32_t CompactFieldBitMask = 0xFF;

		State();
		State(CompactState aCompact);
		CompactState Compact();

		std::optional<State> Modify(const Operation& aOperation);
		std::string ToString(int aBase);

		std::array<uint8_t, Polarization::Count> myCounts;
	};

	struct Node
	{
		std::array<std::optional<CompactState>, std::tuple_size_v<decltype(BaseOperations)>> myReachable;
		struct Link
		{
			CompactState myFromState;
			OperationId myOperation;
		};

		std::optional<Link> myFirstDiscoveredFrom;

		std::vector<CompactState> Explore(CompactState thisState, std::unordered_map<CompactState, Node>& aMap);
	};

	struct FuturePath
	{
		FuturePath(CompactState aStart, CompactState aEnd);
		FuturePath(State aStart, State aEnd);

		CompactState myStart;
		CompactState myEnd;

		std::vector<OperationId> GetResult();
		bool Failed();
		bool Done();

		void Step(uint32_t aIterations);

		uint32_t mySteps = 0;

	private:
		std::queue<CompactState> myQueue;
		std::unordered_map<CompactState, Node> myMap;
		std::optional<std::vector<OperationId>> myPath;
	};

	std::string OperationToString(OperationId aOperation);
	std::string OperationToStringCompact(OperationId aOperation);
	std::string PathToString(std::vector<OperationId> aPath);
}