// ReSharper disable CppInconsistentNaming
#include <iostream>
#include <sstream>
#include <string>
#include <fstream>

struct Trap
{
	int BaseDamage = 0;
	int DamageMultiplier = 1;
	int SlowMultiplier = 0;

	int TotalDamage = 0;
	int ApplyFreeze = 0;
	int FreezePower = 0;
	char Mark = ' ';

	bool Frozen = false;
	void (*Ignite)(Trap*, int, int) = nullptr;
	void (*Freeze)(Trap*, int) = nullptr;
};


void IgniteTrap(Trap* trap, int dummy = 0, int dummy1 = 0)
{
}
void IgniteFireTrapI(Trap* trap, int dummy = 0, int dummy1 = 0)
{
	trap->DamageMultiplier = 2;
	trap->TotalDamage = trap->BaseDamage * (trap->SlowMultiplier + 1) * trap->DamageMultiplier;
}
void IgniteFireTrapII(Trap* trap, int dummy = 0, int dummy1 = 0)
{
	trap->DamageMultiplier = 2;
	trap->TotalDamage = trap->BaseDamage * (trap->SlowMultiplier + 1) * trap->DamageMultiplier;
}

void IgniteStrengthTowerI(Trap* trap, int dummy = 0, int dummy1 = 0)
{
	trap->BaseDamage = dummy1 * 2;
	trap->DamageMultiplier = dummy;
	trap->TotalDamage = trap->BaseDamage * (trap->SlowMultiplier + 1) * trap->DamageMultiplier;
}

void FreezeCell(Trap* trap, const int freezePower)
{
	if (freezePower == 0)
	{
		trap->SlowMultiplier = freezePower;
		trap->TotalDamage = trap->BaseDamage * (trap->SlowMultiplier + 1) * trap->DamageMultiplier;
		trap->Frozen = false;
	}
	else
	{
		trap->SlowMultiplier = freezePower;
		trap->TotalDamage = trap->BaseDamage * (trap->SlowMultiplier + 1) * trap->DamageMultiplier;
		trap->Frozen = true;
	}
}
void AntiFreezeCell(Trap* trap, int freezePower)
{

}

struct FireTrap : Trap
{
	FireTrap()
	{
		Mark = 'F';
		TotalDamage = BaseDamage = 50;
		Ignite = &IgniteFireTrapI;
		Freeze = &FreezeCell;
	}
};

struct FireTrapII : FireTrap
{
	FireTrapII()
	{
		Mark = 'F';
		TotalDamage = BaseDamage = 500;
		Ignite = &IgniteFireTrapII;
		Freeze = &FreezeCell;
	}

};

struct FrostTrap : Trap
{
	FrostTrap()
	{
		Mark = 'I';
		TotalDamage = BaseDamage = 10;
		ApplyFreeze = 3;
		FreezePower = 1;
		Ignite = &IgniteTrap;
		Freeze = &AntiFreezeCell;
	}
};


struct FrostTrapII : FrostTrap
{
	FrostTrapII()
	{
		Mark = 'I';
		TotalDamage = BaseDamage = 50;
		ApplyFreeze = 4;
		FreezePower = 1;
		Ignite = &IgniteTrap;
		Freeze = &AntiFreezeCell;
	}
};

struct FrostTrapIII : FrostTrapII
{
	FrostTrapIII()
	{
		Mark = 'I';
		TotalDamage = BaseDamage = 500;
		ApplyFreeze = 4;
		FreezePower = 1;
		Ignite = &IgniteTrap;
		Freeze = &AntiFreezeCell;
	}
};



struct StrengthTowerI : Trap
{
	StrengthTowerI()
	{
		Mark = 'T';
		BaseDamage = 50;
		Ignite = &IgniteStrengthTowerI;
		Freeze = &FreezeCell;
	}
};



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
static uint8_t Map[LevelCount][ColumnCount];
static uint8_t BestMap[LevelCount][ColumnCount];
static uint8_t DebugMap[LevelCount][ColumnCount] =
{
	{1,0,0,0,1},
	{2,0,0,0,0},
	{1,0,0,0,1},
	{2,0,0,0,0},
	{1,0,0,0,0},
	{1,0,0,0,0}
};

static Trap Traps[LevelCount][ColumnCount];
static std::stringstream outputString;


struct Spire
{
	int TotalDamage = 0;

	Spire()
	{
		Populate();
	}

	static void CopyToBestMap()
	{
		for (int j = 0; j < Locked + 1 && j < LevelCount; j++)
			for (int i = 0; i < ColumnCount; i++)
				BestMap[j][i] = Map[j][i];
	}

	static bool CompareMap()
	{
		static int closest = 0;
		int points = 0;
		for (int j = 0; j < LevelCount; j++)
			for (int i = 0; i < ColumnCount; i++)
			{
				if (DebugMap[j][i] != Map[j][i])
					return false;
				++points;
				if (points > closest && Locked == 1)
					closest = points;
			}
		return true;
	}

	static void CopyToMap()
	{
		towerTokens = MaxTowers;
		for (int j = 0; j < LevelCount; j++)
			for (int i = 0; i < ColumnCount; i++)
			{
				if (j < Locked)
				{
					Map[j][i] = BestMap[j][i];
					Build(j, i);
				}
				if (Map[j][i] == 2)
					--towerTokens;
			}
	}

	static void IncrementList()
	{
		++mapIndex;
		uint8_t carryover = 1;

		for (int j = Locked; j < LevelCount; j++)
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

			for (int i = 0; i < ColumnCount; i++)
			{
				if (Map[j][i] == 2)
					columnHasTower = true;
			}
			for (int i = 0; i < ColumnCount; i++)
			{
				const uint8_t oldTower = Map[j][i];
				const uint8_t hadToken = Map[j][i] == 2;
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
				if (oldTower != Map[j][i])
					Build(j, i);
				if (carryover == 0)
					return;
			}
		}

		Exhausted = true;
	}


	static void Build(const int j, const int i)
	{
		switch (Map[j][i])
		{
		case 0:
			Traps[j][i] = FireTrapII();
			break;
		case 1:
			Traps[j][i] = FrostTrapIII();
			break;
		case 2:
			Traps[j][i] = StrengthTowerI();
			break;
		default:
			break;
		}
	}

	void Populate()
	{
		if (mapIndex == 0)
			for (int j = 0; j < LevelCount; j++)
				for (int i = 0; i < ColumnCount; i++)
					Build(j, i);
		IncrementList();

		int freezeRounds = 0;
		int static freezePower = 0;
		TotalDamage = 0;

		for (int j = 0; j < LevelCount; j++)
		{
			auto fireTraps = 0;
			auto tower = -1;
			for (int i = 0; i < ColumnCount; i++)
			{
				if (tower == -1 && Map[j][i] == 2)
					tower = i;
				if (Map[j][i] == 0)
				{
					if (tower == -1)
					{
						for (int k = 0; k < ColumnCount; k++)
							Traps[j][k].DamageMultiplier = 1;
						break;
					}
					fireTraps++;
					Traps[j][i].Ignite(&Traps[j][i], 0, 0);
				}
				if (tower == -1)
					break;
				if (i == ColumnCount - 1 && fireTraps != 0)
					Traps[j][tower].Ignite(&Traps[j][tower], fireTraps, 500);
			}

			for (int i = 0; i < ColumnCount; i++)
			{
				float static freezeMultiplier;
				Traps[j][i].Freeze(&Traps[j][i], 0);

				if (Traps[j][i].ApplyFreeze > 0)
				{
					freezeRounds = std::max(freezeRounds, Traps[j][i].ApplyFreeze);
					freezePower = Traps[j][i].FreezePower;
					freezeMultiplier = Traps[j][i].BaseDamage == 500 ? 1.25 : 1.0;
				}
				else if (freezeRounds > 0)
				{
					Traps[j][i].Freeze(&Traps[j][i], freezePower);
					freezeRounds = std::max(--freezeRounds, 0);
					Traps[j][i].TotalDamage *= freezeMultiplier;
				}
				TotalDamage += Traps[j][i].TotalDamage;
			}
		}
	}

	static void FormatText(const int trap)
	{
		FormatText();
		switch (trap)
		{
		case 0:
			std::cout << "\x1b[41;1m";

			//Console.BackgroundColor = ConsoleColor.DarkRed;
			break;
		case 1:
			std::cout << "\x1b[44;1m";
			//Console.BackgroundColor = ConsoleColor.DarkBlue;
			break;
		case 2:
			std::cout << "\x1b[43;1m";

			//Console.BackgroundColor = ConsoleColor.DarkYellow;
			//Console.ForegroundColor = ConsoleColor.Black;
			break;
		default:
			std::cout << "\x1b[39;49m";
			break;
		}
	}

	static void FormatText()
	{
		std::cout << "\x1b[39;49m";
		//Console.BackgroundColor = ConsoleColor.Black;
		//Console.ForegroundColor = ConsoleColor.White;
	}

	void PrintDamageToFile() const
	{

		for (int j = LevelCount - 1; j >= 0; j--)
		{
			for (int i = 0; i < ColumnCount; i++)
			{
				const auto round = Traps[j][i].BaseDamage * Traps[j][i].DamageMultiplier;
				const auto multiplier = Traps[j][i].SlowMultiplier + 1;
				auto formattedWord = " " + (padRight(multiplier > 1 ? std::to_string(multiplier) + "x" : "", 3) + padLeft(std::to_string(round) + " ", 5));
				outputString << formattedWord;
			}

			outputString << std::endl;
		}

		const auto damageOutput = "\nTotal Damage: " + std::to_string(TotalDamage) + "\nIndex:        " + std::to_string(
			mapIndex) + "\n\n";
		outputString << damageOutput;

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

	void PrintDamageToConsole() const
	{

		for (int j = LevelCount - 1; j >= 0; j--)
		{
			for (int i = 0; i < ColumnCount; i++)
			{
				FormatText(Map[j][i]);
				const auto multiplier = Traps[j][i].SlowMultiplier + 1;
				const auto round = Traps[j][i].TotalDamage / multiplier; // Traps[j][i].BaseDamage * Traps[j][i].DamageMultiplier;
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


};


int main()
{
	// See https://aka.ms/new-console-template for more information

	int maxDamage = 0;

	for (; ; )
	{
		Spire spire;
		if (spire.TotalDamage > maxDamage)
		{
			Spire::CopyToBestMap();
			maxDamage = spire.TotalDamage;
			if (output)
				spire.PrintDamageToConsole();
		}

		if (debug)
		{
			spire.PrintDamageToFile();
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
