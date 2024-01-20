
#include "Arcospheres.h"

namespace arcospheres
{
	std::array<Operation, 10> BaseOperations =
	{ {
		{.myTakes = { Zeta, Theta, Gamma, Omega },.myMakes = { Lambda, Xi, Epsilon, Phi }},
		{.myTakes = { Lambda, Xi, Epsilon, Phi },.myMakes = { Zeta, Theta, Gamma, Omega }},
		{.myTakes = { Lambda, Omega },.myMakes = { Xi, Theta }},
		{.myTakes = { Xi, Gamma },.myMakes = { Zeta, Lambda }},
		{.myTakes = { Xi, Zeta },.myMakes = { Theta, Phi }},
		{.myTakes = { Lambda, Theta },.myMakes = { Epsilon, Zeta }},
		{.myTakes = { Theta, Epsilon},.myMakes = { Phi, Omega}},
		{.myTakes = { Zeta, Phi },.myMakes = { Gamma, Epsilon }},
		{.myTakes = { Phi, Gamma },.myMakes = { Omega, Xi }},
		{.myTakes = { Epsilon, Omega },.myMakes = { Lambda, Gamma }}
	} };

	std::vector<OperationId> UnwindPath(CompactState aStart, CompactState aEnd, std::unordered_map<CompactState, Node>& aMap)
	{
		std::vector<OperationId> path;

		CompactState at = aEnd;
		while (at != aStart)
		{
			Node::Link from = *aMap[at].myFirstDiscoveredFrom;

			path.push_back(from.myOperation);
			at = from.myFromState;
		}

		std::reverse(path.begin(), path.end());

		return path;
	}

	std::string OperationToString(OperationId aOperation)
	{
		switch (aOperation)
		{
		case 0:
			return "Invert-left";
		case 1:
			return "Invert-right";
		case 2:
			return "Fold-1";
		case 3:
			return "Fold-2";
		case 4:
			return "Fold-3";
		case 5:
			return "Fold-4";
		case 6:
			return "Fold-5";
		case 7:
			return "Fold-6";
		case 8:
			return "Fold-7";
		case 9:
			return "Fold-8";
		}
		return "";
	}

	std::string OperationToStringCompact(OperationId aOperation)
	{
		switch (aOperation)
		{
		case 0:
			return "<";
		case 1:
			return ">";
		case 2:
			return "a";
		case 3:
			return "b";
		case 4:
			return "c";
		case 5:
			return "d";
		case 6:
			return "e";
		case 7:
			return "f";
		case 8:
			return "g";
		case 9:
			return "h";
		}
		return "";
	}

	std::string PathToString(std::vector<OperationId> aPath)
	{
		if (aPath.empty())
			return "";

		std::string out = OperationToString(aPath[0]);

		for (size_t i = 1; i < aPath.size(); i++)
			out += " -> " + OperationToString(aPath[i]);

		return out;
	}

	State::State()
		: myCounts{0}
	{
	}

	State::State(CompactState aCompact)
	{
		for (size_t i = 0; i < 8; i++)
			myCounts[i] = (aCompact >> (i * CompactFieldBitWidth)) & CompactFieldBitMask;
	}

	CompactState State::Compact()
	{
		CompactState compact = 0;

		for (size_t i = 0; i < 8; i++)
			compact |= (static_cast<CompactState>(myCounts[i]) << (i * CompactFieldBitWidth));

		return compact;
	}

	std::optional<State> State::Modify(const Operation& aOperation)
	{
		State cpy(*this);

		for (Polarization pol : aOperation.myTakes)
		{
			if (cpy.myCounts[pol] == 0)
				return {};

			cpy.myCounts[pol]--;
		}

		for (Polarization pol : aOperation.myMakes)
			cpy.myCounts[pol]++;

		return cpy;
	}

	std::string State::ToString(int aBase)
	{
		std::string out;

		for (uint8_t count : myCounts)
		{
			int delta = (int)count - aBase;

			if (delta >= 0)
				out += "  ";
			else
				out += " ";

			out += std::to_string(delta);
		}

		out += " [";
		
		int left = myCounts[Theta] +
			myCounts[Omega] +
			myCounts[Zeta] +
			myCounts[Gamma];

		int right = myCounts[Lambda] +
			myCounts[Epsilon] +
			myCounts[Xi] +
			myCounts[Phi];

		out += std::to_string(left) + ", " + std::to_string(right) + " = " + std::to_string((left - right + 800) % 8);

		out += "]";

		return out;
	}

	std::vector<CompactState> Node::Explore(CompactState thisState, std::unordered_map<CompactState, Node>& aMap)
	{
		std::vector<CompactState> out;
		State current(thisState);

		for (OperationId i = 0; i < BaseOperations.size(); i++)
		{
			std::optional<State> next = current.Modify(BaseOperations[i]);

			if (!next)
				continue;

			aMap[thisState].myReachable[i] = next->Compact();

			if (!aMap[next->Compact()].myFirstDiscoveredFrom)
			{
				out.push_back(next->Compact());
				aMap[next->Compact()].myFirstDiscoveredFrom = Node::Link{ .myFromState = thisState, .myOperation = i };
			}
		}

		return out;
	}

	FuturePath::FuturePath(CompactState aStart, CompactState aEnd)
	{
		myStart = aStart;
		myEnd = aEnd;

		myMap[myStart];
		myQueue.push(myStart);
	}

	FuturePath::FuturePath(State aStart, State aEnd)
	{
		myStart = aStart.Compact();
		myEnd = aEnd.Compact();

		myMap[myStart];
		myQueue.push(myStart);
	}

	std::vector<OperationId> FuturePath::GetResult()
	{
		return *myPath;
	}

	bool FuturePath::Failed()
	{
		return !Done() && myQueue.empty();
	}

	bool FuturePath::Done()
	{
		return static_cast<bool>(myPath);
	}

	void FuturePath::Step(uint32_t aIterations)
	{
		for (uint32_t i = 0; i < aIterations; i++)
		{
			if (myQueue.empty())
				break;

			mySteps++;

			CompactState current = myQueue.front();
			myQueue.pop();

			for (CompactState newNode : myMap[current].Explore(current, myMap))
			{
				if (newNode == myEnd)
				{
					myPath = UnwindPath(myStart, myEnd, myMap);
					myQueue = decltype(myQueue)();
					return;
				}

				myQueue.push(newNode);
			}
		}
	}
}