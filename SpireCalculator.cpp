// SpireCalculatorC++.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

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
	void (*Ignite)(Trap*, int, int);
	void (*Freeze)(Trap*, int);
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

void FreezeCell(Trap* trap, int freezePower)
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



static const int LevelCount = 6;
static const int MaxTowers = 2;
static const int ColumnCount = 5;
static int _towerTokens = 2;
static long _mapIndex = 0;
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
static std::stringstream text;


struct Spire
{
	int TotalDamage = 0;

	Spire()
	{
		Populate();
	}

	const static int Offset = 4;

	static void CopyToBestMap()
	{
		for (int j = 0; j < Locked; j++)
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

	void CopyToMap()
	{
		_towerTokens = MaxTowers;
		for (int j = 0; j < LevelCount; j++)
			for (int i = 0; i < ColumnCount; i++)
			{
				if (j < Locked)
				{
					Map[j][i] = BestMap[j][i];
					Build(j, i);
				}
				if (Map[j][i] == 2)
					--_towerTokens;
			}
	}

	void IncrementList()
	{
		++_mapIndex;
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
				uint8_t oldTower = Map[j][i];
				uint8_t hadToken = Map[j][i] == 2;
				Map[j][i] += carryover;
				carryover = 0;

				if (Map[j][i] == 3)
				{
					carryover = 1;
					columnHasTower = false;
					Map[j][i] = 0;
					_towerTokens = hadToken ? ++_towerTokens : --_towerTokens;
				}
				else if (Map[j][i] == 1)
				{
					bool allowed = false;
					if (j == 0 && i == 0)
						allowed = true;
					else if (j == 0 && Map[0][i - 1] != 1)
						allowed = true;
					else if (i == 0 && Map[j - 1][ColumnCount - 1] != 1)
						allowed = true;
					else if (i != 0 && Map[j][i - 1] != 1)
						allowed = true;

					if (!allowed)
						Map[j][i]++;
				}
				if (Map[j][i] == 2)
				{
					bool optimalTowerPlacement = (i == 0 || Map[j][i - 1] != 0) && j % 2 == 1;
					if (_towerTokens > 0 && columnHasTower == false && optimalTowerPlacement)
					{
						--_towerTokens;
						columnHasTower = true;
					}
					else if ((_towerTokens == 0 || columnHasTower || !optimalTowerPlacement) && hadToken == false)
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


	void Build(int j, int i)
	{
		switch (Map[j][i])
		{
		case 0:
			Traps[j][i] = FireTrapII();
			break;
		case 1:
			Traps[j][i] = FrostTrapII();
			break;
		case 2:
			Traps[j][i] = StrengthTowerI();
			break;
		default:
			Traps[j][i] = Traps[j][i];
			break;
		}
	}

	void Populate()
	{
		if (_mapIndex == 0)
			for (int j = 0; j < LevelCount; j++)
				for (int i = 0; i < ColumnCount; i++)
					Build(j, i);
		IncrementList();

		auto freezeRounds = 0;
		auto freezePower = 0;
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

				Traps[j][i].Freeze(&Traps[j][i], 0);

				if (Traps[j][i].ApplyFreeze > 0)
				{
					freezeRounds = std::max(freezeRounds, Traps[j][i].ApplyFreeze);
					freezePower = Traps[j][i].FreezePower;
				}
				else if (freezeRounds > 0)
				{
					Traps[j][i].Freeze(&Traps[j][i], freezePower);
					freezeRounds = std::max(--freezeRounds, 0);
				}
				TotalDamage += Traps[j][i].TotalDamage;
			}
		}
	}

	static void FormatText(int trap)
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
		}
	}

	static void FormatText()
	{
		std::cout << "\x1b[39;49m";
		//Console.BackgroundColor = ConsoleColor.Black;
		//Console.ForegroundColor = ConsoleColor.White;
	}

	void PrintDamageToFile()
	{

		for (int j = LevelCount - 1; j >= 0; j--)
		{
			for (int i = 0; i < ColumnCount; i++)
			{
				auto round = Traps[j][i].BaseDamage * Traps[j][i].DamageMultiplier;
				auto mult = (Traps[j][i].SlowMultiplier + 1);
				auto formattedWord = " " + (padRight((mult > 1 ? (std::to_string(mult) + "x") : ""), 3) + padLeft(std::to_string(round) + " ", 5));
				text << formattedWord;
			}

			text << std::endl;
		}

		auto damageOutput = "\nTotal Damage: " + std::to_string(TotalDamage) + "\nIndex:        " + std::to_string(_mapIndex) + "\n\n";
		text << damageOutput;

	}

	std::string padLeft(std::string str, int n)
	{
		if (n <= str.size())
			return str;
		str.insert(0, n - str.size(), ' '); return str;
	}

	std::string padRight(std::string str, int n)
	{
		if (n <= str.size())
			return str;
		str.insert(str.size(), n - str.size(), ' ');
		return str;
	}

	void PrintDamageToConsole()
	{

		for (int j = LevelCount - 1; j >= 0; j--)
		{
			for (int i = 0; i < ColumnCount; i++)
			{
				FormatText(Map[j][i]);
				auto round = Traps[j][i].BaseDamage * Traps[j][i].DamageMultiplier;
				auto mult = (Traps[j][i].SlowMultiplier + 1);
				auto formattedWord = " " + padRight(((mult > 1 ? (std::to_string(mult) + "x") : "")), 3) + padRight(std::to_string(round) + " ", 5);
				std::cout << formattedWord;
			}

			FormatText();
			std::cout << std::endl;
		}

		auto damageOutput = "\nTotal Damage: " + std::to_string(TotalDamage) + "\nIndex:        " + std::to_string(_mapIndex) + "\n\n";
		std::cout << damageOutput;
	}


};


int main()
{
	// See https://aka.ms/new-console-template for more information

	int maxDamage = 0;

	bool output = true;
	bool debug = false;


	for (; ; )
	{
		Spire spire;
		if (spire.TotalDamage >= maxDamage)
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

			const auto actualOutput = text.str();
			std::ofstream ofFile("output.txt");
			ofFile << actualOutput;


			std::ifstream inFile("expectedOutput.txt");
			const std::string expectedOutput((std::istreambuf_iterator<char>(inFile)),
				(std::istreambuf_iterator<char>()));

			if (expectedOutput != actualOutput)
			{
				std::cout << "Output has changed.\n";
				std::cin;
				return 1;
			}
			return 0;
		}
	}
}
