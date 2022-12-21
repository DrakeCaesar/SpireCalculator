// ReSharper disable CppInconsistentNaming
// ReSharper disable CppDefaultCaseNotHandledInSwitchStatement
#include <iostream>
#include <sstream>
#include <string>
#include <fstream>

int TotalDamage;

constexpr bool output = true;
constexpr bool debug = false;

static constexpr int LevelCount = 7;
static constexpr int MaxTowers = 3;
static constexpr int ColumnCount = 5;
static constexpr int Offset = 4;
static int towerTokens = MaxTowers;
static long mapIndex = 0;
static bool Exhausted = false;
static int Locked = 0;
static int_fast8_t Map[LevelCount][ColumnCount];
static int_fast8_t BestMap[LevelCount][ColumnCount];

static short damageTable[12]
{
	50,  //Base
	100, //Tower x 1 or Fire w/ Tower
	200, //Tower x 2
	300, //Tower x 3
	400, //Tower x 4
	100, //Base w/ Freeze
	200, //Tower x 1 or Fire w/ Tower and Freeze
	400, //Tower x 2 w/ Freeze
	600, //Tower x 3 w/ Freeze
	800, //Tower x 4 w/ Freeze
	10,  //Frost
	3   //Freeze Cells
};

void updateDamageTable(uint_fast8_t fireTrapLevel, uint_fast8_t frostTrapLevel)
{
	if (fireTrapLevel == 2)
		for (int_fast8_t i = 0; i < 10; i++)
			damageTable[i] *= 10;

	if (frostTrapLevel == 3)
		for (int_fast8_t i = 5; i < 10; i++)
			damageTable[i] = damageTable[i] / 4 * 5;

	if (frostTrapLevel == 2)
	{
		damageTable[10] = 50;
		damageTable[11] = 4;
	}
	if (frostTrapLevel == 3)
	{
		damageTable[10] = 500;
		damageTable[11] = 4;
	}

}


static std::stringstream outputString;

void Populate()
{
	int freezeRounds = 0;

	for (int_fast8_t j = 0; j < LevelCount; j++)
	{
		uint_fast8_t fireTraps = 0;
		char tower = -1;
		for (int_fast8_t i = 0; i < ColumnCount; i++)
		{
			switch (Map[j][i])
			{
			case 0:
				++fireTraps;
				break;
			case 2:
				tower = i;
			}
		}
		for (int_fast8_t i = 0; i < ColumnCount; i++)
		{
			switch (Map[j][i])
			{
			case 0:
				TotalDamage += damageTable[(tower == -1 ? 0 : 1) + (freezeRounds ? 5 : 0)];
				freezeRounds = std::max(freezeRounds - 1, 0);
				break;
			case 1:
				freezeRounds = damageTable[11];
				TotalDamage += damageTable[10];
				break;
			case 2:
				TotalDamage += damageTable[fireTraps + (freezeRounds ? 5 : 0)];
				freezeRounds = std::max(freezeRounds - 1, 0);
				break;
			}
		}
	}
}

void Populate(short damageMap[LevelCount][ColumnCount][2])
{
	int freezeRounds = 0;

	for (int_fast8_t j = 0; j < LevelCount; j++)
	{
		int_fast8_t fireTraps = 0;
		int_fast8_t tower = -1;
		for (int_fast8_t i = 0; i < ColumnCount; i++)
		{
			switch (Map[j][i])
			{
			case 0:
				++fireTraps;
				break;
			case 2:
				tower = i;
			}
		}
		for (int_fast8_t i = 0; i < ColumnCount; i++)
		{
			damageMap[j][i][1] = 1;
			switch (Map[j][i])
			{
			case 0:
				damageMap[j][i][0] = damageTable[(tower == -1 ? 0 : 1) + (freezeRounds ? 5 : 0)];
				if (freezeRounds != 0)
					damageMap[j][i][1] = 2;
				freezeRounds = std::max(freezeRounds - 1, 0);
				break;
			case 1:
				freezeRounds = damageTable[11];
				damageMap[j][i][0] = damageTable[10];
				break;
			case 2:
				damageMap[j][i][0] = damageTable[fireTraps + (freezeRounds ? 5 : 0)];
				if (freezeRounds != 0)
					damageMap[j][i][1] = 2;
				freezeRounds = std::max(freezeRounds - 1, 0);
				break;
			}
		}
	}
}





	static void CopyToBestMap()
	{
		for (int_fast8_t j = 0; j < Locked + 1 && j < LevelCount; j++)
			for (int_fast8_t i = 0; i < ColumnCount; i++)
				BestMap[j][i] = Map[j][i];
	}

	static bool CompareMap()
	{
		static int closest = 0;
		int points = 0;
		for (int_fast8_t j = 0; j < LevelCount; j++)
			for (int_fast8_t i = 0; i < ColumnCount; i++)
			{
				//if (DebugMap[j][i] != Map[j][i])
				//	return false;
				++points;
				if (points > closest && Locked == 1)
					closest = points;
			}
		return true;
	}

	static void CopyToMap()
	{
		towerTokens = MaxTowers;
		for (int_fast8_t j = 0; j < LevelCount; j++)
			for (int_fast8_t i = 0; i < ColumnCount; i++)
			{
				if (j < Locked)
					Map[j][i] = BestMap[j][i];
				if (Map[j][i] == 2)
					--towerTokens;
			}
	}

	static void IncrementList()
	{
		++mapIndex;
		int_fast8_t carryover = 1;

		for (int_fast8_t j = Locked; j < LevelCount; j++)
		{
			bool columnHasTower = false;
			if (j > Offset && carryover == 1)
			{
				if (j - Offset > Locked)
				{

					Locked = j - Offset;
					CopyToMap();
					return;
				}
			}

			for (int_fast8_t i = 0; i < ColumnCount; i++)
			{
				if (Map[j][i] == 2)
					columnHasTower = true;
			}
			for (int_fast8_t i = 0; i < ColumnCount; i++)
			{
				const int_fast8_t oldTower = Map[j][i];
				const int_fast8_t hadToken = Map[j][i] == 2;
				Map[j][i] += carryover;
				carryover = 0;

				if (Map[j][i] == 3)
				{
					carryover = 1;
					columnHasTower = false;
					Map[j][i] = 0;
					towerTokens = hadToken ? ++towerTokens : --towerTokens;
				}
				else if
					(Map[j][i] == 1 &&
						(j != 0 || i != 0) &&
						(j != 0 || Map[0][i - 1] == 1) &&
						(i != 0 || Map[j - 1][ColumnCount - 1] == 1) &&
						(i == 0 || Map[j][i - 1] == 1))
					++Map[j][i];
				if (Map[j][i] == 2)
				{
					const bool optimalTowerPlacement = (i == 0 || Map[j][i - 1] != 0) && j % 2 == 1;
					if (towerTokens > 0 && columnHasTower == false && optimalTowerPlacement)
					{
						--towerTokens;
						columnHasTower = true;
					}
					else if ((towerTokens == 0 || columnHasTower || !optimalTowerPlacement) && hadToken == false)
					{
						Map[j][i] = 0;
						carryover = 1;
					}
				}
				if (carryover == 0)
					return;
			}
		}

		Exhausted = true;
	}


	static void FormatText(const int trap = -1)
	{
		switch (trap)
		{
		case 0:
			std::cout << "\x1b[41;1m"; //White on DarkRed
			break;
		case 1:
			std::cout << "\x1b[44;1m"; //White on DarkBlue
			break;
		case 2:
			std::cout << "\x1b[43;1m"; //White on DarkYellow
			break;
		default:
			std::cout << "\x1b[39;49m"; //White on Black
			break;
		}
	}



	//void PrintDamageToFile() const
	//{

	//	for (int_fast8_t j = LevelCount - 1; j >= 0; j--)
	//	{
	//		for (int_fast8_t i = 0; i < ColumnCount; i++)
	//		{
	//			const auto round = Traps[j][i].BaseDamage * Traps[j][i].DamageMultiplier;
	//			const auto multiplier = Traps[j][i].SlowMultiplier + 1;
	//			auto formattedWord = " " + (padRight(multiplier > 1 ? std::to_string(multiplier) + "x" : "", 3) + padLeft(std::to_string(round) + " ", 5));
	//			outputString << formattedWord;
	//		}

	//		outputString << std::endl;
	//	}

	//	const auto damageOutput = "\nTotal Damage: " + std::to_string(TotalDamage) + "\nIndex:        " + std::to_string(
	//		mapIndex) + "\n\n";
	//	outputString << damageOutput;
	//}

	static std::string padLeft(std::string str, const unsigned long long n)
	{
		if (n <= str.size())
			return str;
		str.insert(0, n - str.size(), ' '); return str;
	}

	static std::string padRight(std::string str, const unsigned long long n)
	{
		if (n <= str.size())
			return str;
		str.insert(str.size(), n - str.size(), ' ');
		return str;
	}



	void PrintDamageToConsole()
	{
		static short PrintMap[LevelCount][ColumnCount][2];
		Populate(PrintMap);

		for (int_fast8_t j = LevelCount - 1; j >= 0; j--)
		{
			for (int_fast8_t i = 0; i < ColumnCount; i++)
			{
				FormatText(Map[j][i]);
				const auto round = PrintMap[j][i][0] / PrintMap[j][i][1];
				const auto multiplier = PrintMap[j][i][1];
				auto formattedWord = " " + padRight(multiplier > 1 ? std::to_string(multiplier) + "x" : "", 3) + padRight(std::to_string(round) + " ", 5);
				std::cout << formattedWord;
			}

			FormatText();
			std::cout << std::endl;
		}

		auto damageOutput = "\nTotal Damage: " + std::to_string(TotalDamage) + "\nIndex:        " + std::to_string(
			mapIndex) + "\n\n";
		std::cout << damageOutput;
	}



int main()
{
	// See https://aka.ms/new-console-template for more information

	int maxDamage = 0;
	updateDamageTable(2, 3);

	for (; ; )
	{
		TotalDamage = 0;
		IncrementList();
		TotalDamage = 0;
		Populate();
		if (TotalDamage > maxDamage)
		{
			CopyToBestMap();
			maxDamage = TotalDamage;
			if (output)
				PrintDamageToConsole();
		}

		if (debug)
		{
			//spire.PrintDamageToFile();
			//if (Spire._mapIndex % 1000 == 0)
			//{
			//    Console.WriteLine(Spire._mapIndex);
			//}
		}

		if (Exhausted)
		{
			//Console.ReadLine();

			if (!debug)
				return 0;

			const std::string actualOutput = outputString.str();
			std::ofstream ofFile("output.txt");
			ofFile << actualOutput;


			std::ifstream inFile("expectedOutput.txt");
			const std::string expectedOutput((std::istreambuf_iterator<char>(inFile)),
				(std::istreambuf_iterator<char>()));

			if (expectedOutput != actualOutput)
			{
				std::cout << "Output has changed.\n";
				std::cin;  // NOLINT(clang-diagnostic-unused-value)
				return 1;
			}
			return 0;
		}
	}
}
