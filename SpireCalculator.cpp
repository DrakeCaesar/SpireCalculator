// ReSharper disable CppInconsistentNaming
// ReSharper disable CppDefaultCaseNotHandledInSwitchStatement
// ReSharper disable CppClangTidyModernizeLoopConvert
#include <iostream>
#include <sstream>
#include <string>
#include <fstream>
#include <thread>
#include <mutex>
#include <chrono>

std::mutex mtx;


unsigned int maxDamage = 0;

constexpr bool output = true;
constexpr bool debug = false;

static constexpr uint_fast8_t LevelCount = 8;
static constexpr uint_fast8_t MaxTowers = 3;
static constexpr uint_fast8_t ColumnCount = 5;
static constexpr uint_fast8_t Offset = 4;
static constexpr uint_fast8_t Threads = 8;
static unsigned int mapIndex = 0;
static bool Exhausted = false;
static bool LockedSwitch = false;
static uint_fast8_t Locked = 0;
static uint_fast8_t MapArray[8][LevelCount][ColumnCount];
static uint_fast8_t TowerArray[8];
static uint_fast8_t BestMap[LevelCount][ColumnCount];
static std::stringstream outputString;

//7915089 map index
//New: Total iterations: 79691916
//Old: Total iterations : 9961472
static uint_fast16_t damageTable[12]
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

void updateDamageTable(const uint_fast8_t fireTrapLevel, const uint_fast8_t frostTrapLevel)
{
	if (fireTrapLevel == 2)
		for (uint_fast8_t i = 0; i < 10; i++)
			damageTable[i] *= 10;
	if (fireTrapLevel == 3)
		for (uint_fast8_t i = 0; i < 10; i++)
			damageTable[i] *= 10 * 5;

	if (frostTrapLevel == 3)
		for (uint_fast8_t i = 5; i < 10; i++)
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

uint_fast16_t max(const uint_fast16_t left, const uint_fast16_t right)
{
	return left < right ? right : left;
}

uint_fast8_t max(const uint_fast8_t left, const uint_fast8_t right)
{
	return left < right ? right : left;
}

void PopulatePrint(uint_fast8_t Map[LevelCount][ColumnCount], uint_fast16_t damageMap[LevelCount][ColumnCount][2])
{
	uint_fast16_t freezeRounds = 0;

	for (uint_fast8_t j = 0; j < LevelCount; j++)
	{
		uint_fast8_t fireTraps = 0;
		uint_fast8_t tower = 5;
		for (uint_fast8_t i = 0; i < ColumnCount; i++)
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
		for (uint_fast8_t i = 0; i < ColumnCount; i++)
		{
			damageMap[j][i][1] = 1;
			switch (Map[j][i])
			{
			case 0:
				damageMap[j][i][0] = damageTable[(tower == 5 ? 0 : 1) + (freezeRounds ? 5 : 0)];
				if (freezeRounds != 0)
					damageMap[j][i][1] = 2;
				freezeRounds = --freezeRounds != UINT_FAST16_MAX ? freezeRounds : 0;
				break;
			case 1:
				freezeRounds = damageTable[11];
				damageMap[j][i][0] = damageTable[10];
				break;
			case 2:
				damageMap[j][i][0] = damageTable[fireTraps + (freezeRounds ? 5 : 0)];
				if (freezeRounds != 0)
					damageMap[j][i][1] = 2; 
				freezeRounds = --freezeRounds != UINT_FAST16_MAX ? freezeRounds : 0;
				break;
			}
		}
	}
}


static void CopyToBestMap(uint_fast8_t Map[LevelCount][ColumnCount])
{
	for (uint_fast8_t j = 0; j < Locked + 1 && j < LevelCount; j++)
		for (uint_fast8_t i = 0; i < ColumnCount; i++)
			BestMap[j][i] = Map[j][i];
}

//static bool CompareMap()
//{
//	static uint_fast8_t closest = 0;
//	uint_fast8_t points = 0;
//	for (uint_fast8_t j = 0; j < LevelCount; j++)
//		for (uint_fast8_t i = 0; i < ColumnCount; i++)
//		{
//			++points;
//			if (points > closest && Locked == 1)
//				closest = points;
//		}
//	return true;
//}

static void CopyToMap()
{
	for (uint_fast8_t k = 0; k < Threads; k++) {

		TowerArray[k] = MaxTowers;
		for (uint_fast8_t j = 0; j < LevelCount; j++)
			for (uint_fast8_t i = 0; i < ColumnCount; i++)
			{
				if (j < Locked)
				{
					MapArray[k][j][i] = BestMap[j][i];
					if (MapArray[k][j][i] == 2)
						--TowerArray[k];
				} else
				{
					MapArray[k][j][i] = 0;
				}
			}
	}
}

static void IncrementList(uint_fast8_t Map[LevelCount][ColumnCount], uint_fast8_t* towerTokens, uint_fast8_t carryover, bool locking = true)
{
	if (carryover == 0)
		return;
	++mapIndex;
	for (uint_fast8_t j = Locked; j < LevelCount; j++)
	{
		bool columnHasTower = false;
		if (j > Offset && carryover >= 1)
		{
			if (j - Offset > Locked && locking)
			{
				LockedSwitch = true;
				return;
			}
		}

		for (uint_fast8_t i = 0; i < ColumnCount; i++)
		{
			if (Map[j][i] == 2)
				columnHasTower = true;
		}
		for (uint_fast8_t i = 0; i < ColumnCount; i++)
		{
			const uint_fast8_t hadToken = Map[j][i] == 2;
			++Map[j][i];
			--carryover;

			if (Map[j][i] == 3)
			{
				++carryover;
				columnHasTower = false;
				Map[j][i] = 0;
				(*towerTokens) = hadToken ? ++(*towerTokens) : --(*towerTokens);
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
				if ((*towerTokens) > 0 && columnHasTower == false && optimalTowerPlacement)
				{
					--(*towerTokens);
					columnHasTower = true;
				}
				else if (((*towerTokens) == 0 || columnHasTower || !optimalTowerPlacement) && hadToken == false)
				{
					Map[j][i] = 0;
					++carryover;
				}
			}
			if (carryover == 0)
				return;
		}
	}

	Exhausted = true;
}


static void FormatText(const uint_fast8_t trap = 255)
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

void PrintDamageToFile(uint_fast8_t Map[LevelCount][ColumnCount], int TotalDamage)
{
	static uint_fast16_t PrintMap[LevelCount][ColumnCount][2];
	PopulatePrint(Map, PrintMap);
	for (uint_fast8_t j = LevelCount - 1; j != UINT_FAST8_MAX; j--)
	{
		for (uint_fast8_t i = 0; i < ColumnCount; i++)
		{
			const uint_fast16_t round = PrintMap[j][i][0] / PrintMap[j][i][1];
			const uint_fast16_t multiplier = PrintMap[j][i][1];
			const std::string formattedWord = " " + (padRight(multiplier > 1 ? std::to_string(multiplier) + "x" : "", 3) + padLeft(std::to_string(round) + " ", 5));
			outputString << formattedWord;
		}

		outputString << std::endl;
	}

	const std::string damageOutput = "\nTotal Damage: " + std::to_string(TotalDamage) + "\nIndex:        " + std::to_string(
		mapIndex) + "\n\n";
	outputString << damageOutput;
}

void PrintDamageToConsole(uint_fast8_t Map[LevelCount][ColumnCount], const unsigned int TotalDamage)
{
	static uint_fast16_t PrintMap[LevelCount][ColumnCount][2];
	PopulatePrint(Map, PrintMap);

	for (uint_fast8_t j = LevelCount - 1; j != UINT_FAST8_MAX; j--)
	{
		FormatText();

		for (uint_fast8_t i = 0; i < ColumnCount; i++)
		{
			FormatText(Map[j][i]);
			const uint_fast16_t round = PrintMap[j][i][0] / PrintMap[j][i][1];
			const uint_fast16_t multiplier = PrintMap[j][i][1];
			const std::string formattedWord = " " + padRight(multiplier > 1 ? std::to_string(multiplier) + "x" : "", 3) + padRight(std::to_string(round) + " ", 6);
			std::cout << formattedWord;
		}
		FormatText();
		std::cout << std::endl;
	}

	const std::string damageOutput = "\nTotal Damage: " + std::to_string(TotalDamage) + "\nIndex:        " + std::to_string(
		mapIndex) + "\n\n";
	std::cout << damageOutput;
}

int Populate(uint_fast8_t Map[LevelCount][ColumnCount])
{
	uint_fast8_t damageCountTable[11] = { 0 };
	uint_fast16_t freezeRounds = 0;
	for (uint_fast8_t j = 0; j < LevelCount; j++)
	{
		uint_fast8_t fireTraps = 0;
		uint_fast8_t tower = 5;
		for (uint_fast8_t i = 0; i < ColumnCount; i++)
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
		for (uint_fast8_t i = 0; i < ColumnCount; i++)
		{
			switch (Map[j][i])
			{
			case 0:
				++damageCountTable[(tower == 5 ? 0 : 1) + (freezeRounds ? 5 : 0)];
				freezeRounds = --freezeRounds != UINT_FAST16_MAX ? freezeRounds : 0;
				break;
			case 1:
				freezeRounds = damageTable[11];
				++damageCountTable[10];
				break;
			case 2:
				++damageCountTable[fireTraps + (freezeRounds ? 5 : 0)];
				freezeRounds = --freezeRounds != UINT_FAST16_MAX ? freezeRounds : 0;
			}
		}
	}
	int TotalDamage = 0;
	for (uint_fast8_t i = 0; i < 10; i++)
		TotalDamage += damageTable[i] * damageCountTable[i];
	return TotalDamage;
}

// A dummy function
void foo(const int i)
{
	for (;;) {
		const unsigned int TotalDamage = Populate(MapArray[i]);
		if (TotalDamage > maxDamage)
		{
			mtx.lock();
			if (TotalDamage > maxDamage) {
				CopyToBestMap(MapArray[i]);
				maxDamage = TotalDamage;
				if (output)
					PrintDamageToConsole(MapArray[i], TotalDamage);
			}
			mtx.unlock();
		}
		if (LockedSwitch || Exhausted)
			return;
		for (uint_fast8_t j = 0; j < Threads; j++)
			IncrementList(MapArray[i], &TowerArray[i], 1);
	}
}



int main()
{
	std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();

	std::thread threads[Threads];
	// See https://aka.ms/new-console-template for more information

	updateDamageTable(3, 3);
	std::fill_n(TowerArray, Threads, MaxTowers);

	for (uint_fast8_t i = 0; i < Threads; i++)
	{
		for (uint_fast8_t j = 0; j < i; j++)
		{
			IncrementList(MapArray[i], &TowerArray[i], 1);
		}
	}

	for (; ; ){
	
		for (uint_fast8_t i = 0; i < Threads; i++)
		{
			threads[i] = std::thread(foo, i);
		}

		for (; ; ) {
			std::this_thread::sleep_for(std::chrono::milliseconds(100));
			if (LockedSwitch || Exhausted)
				break;
		}
		for (uint_fast8_t i = 0; i < Threads; i++)
		{
			threads[i].join();
		}

		if (Exhausted)
		{
			//Console.ReadLine();
			std::cout << "Total iterations: " << mapIndex << std::endl;
			std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
			std::cout << "Elapsed time:     " << std::chrono::duration_cast<std::chrono::seconds>(end - begin).count() << " seconds" << std::endl;


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
				// ReSharper disable once CppExpressionWithoutSideEffects
				std::cin;  // NOLINT(clang-diagnostic-unused-value)
				return 1;
			}
			return 0;
		}

		if (LockedSwitch)
		{
			++Locked;
			LockedSwitch = false;
			CopyToMap();
			for (uint_fast8_t i = 0; i < Threads; i++)
			{
				for (uint_fast8_t j = 0; j < i; j++)
				{
					IncrementList(MapArray[i], &TowerArray[i], 1);

				}
			}

		}
	}
}
