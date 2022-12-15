#define FireTower

// See https://aka.ms/new-console-template for more information
using System;
using System.IO;
using System.Text;

var maxDamage = 0;
var debug = true;


for (; ; )
{
    var spire = new Spire();
    if (spire.TotalDamage >= maxDamage)
    {
        Spire.CopyToBestMap();
        maxDamage = spire.TotalDamage;
        spire.PrintDamageToConsole();
    }

    if (debug)
    {
        spire.PrintDamageToFile();
        if (Spire._mapIndex % 1000 == 0)
        {
            Console.WriteLine(Spire._mapIndex);
        }
    }

    if (Spire.Exhausted)
    {
        //Console.ReadLine();

        if (!debug)
            return;
        var expectedOutput = File.ReadAllText("../../../expectedOutput.txt");
        File.WriteAllText("../../../output.txt", Spire.text.ToString());
        if (!expectedOutput.Equals(Spire.text.ToString()))
        {
            Console.WriteLine("Output has changed.");
            Console.ReadLine();
        }
        return;
    }
}

internal class Spire
{
    public static readonly int LevelCount = 6;
    private static readonly int MaxTowers = 2;

    private static readonly int ColumnCount = 5;

    private static readonly Trap[,] Traps = new Trap[LevelCount, ColumnCount];
    public int TotalDamage;

    public static byte[,] Map = new byte[LevelCount, ColumnCount];
    public static byte[,] BestMap = new byte[LevelCount, ColumnCount];
    public static int BestTowerTokens;

    internal static bool Exhausted;
    public static StringBuilder text = new StringBuilder();

    public Spire()
    {
        Populate();
    }

    public static long _mapIndex;
    private static int _towerTokens = MaxTowers;
    private const int Offset = 2;
    public static int Locked;

    public static void CopyToBestMap()
    {
        for (var j = 0; j < LevelCount; j++)
            for (var i = 0; i < ColumnCount; i++)
                BestMap[j, i] = Map[j, i];
        BestTowerTokens = _towerTokens;
    }

    public void CopyToMap()
    {
        _towerTokens = 0;
        for (var j = 0; j < LevelCount; j++)
            for (var i = 0; i < ColumnCount; i++)
            {
                if (j < Locked)
                {
                    Map[j, i] = BestMap[j, i];
                    Build(j, i);
                }
                if (Map[j, i] == 2)
                    ++_towerTokens;
            }
    }

    private void IncrementList()
    {
        ++_mapIndex;
        byte carryover = 1;

        for (var j = Locked; j < LevelCount; j++)
        {
            var columnHasTower = false;
            if (j > Offset && carryover == 1)
            {
                if (j - Offset > Locked)
                {

                    Locked = j - Offset;
                    CopyToMap();
                }
            }

            for (var i = 0; i < ColumnCount; i++)
            {
                if (Map[j, i] == 2)
                    columnHasTower = true;
            }
            for (var i = 0; i < ColumnCount; i++)
            {
                var oldTower = Map[j, i];
                var hadToken = Map[j, i] == 2;
                Map[j, i] += carryover;
                carryover = 0;

                if (Map[j, i] == 3)
                {
                    carryover = 1;
                    columnHasTower = false;
                    Map[j, i] = 0;
                    _towerTokens = hadToken ? ++_towerTokens : --_towerTokens;
                }
                else if (Map[j, i] == 1)
                {
                    bool allowed = false;
                    if (j == 0 && i == 0)
                        allowed = true;
                    else if (j == 0 && Map[0, i - 1] != 1)
                        allowed = true;
                    else if (i == 0 && Map[j - 1, ColumnCount - 1] != 1)
                        allowed = true;
                    else if (i != 0 && Map[j, i - 1] != 1)
                        allowed = true;

                    if (!allowed)
                        Map[j, i]++;
                }
                if (Map[j, i] == 2)
                {
                    var optimalTowerPlacement = (i == 0 || Map[j, i - 1] != 0) && j % 2 == 1;
                    if (_towerTokens > 0 && columnHasTower == false && optimalTowerPlacement)
                    {
                        --_towerTokens;
                        columnHasTower = true;
                    }
                    else if ((_towerTokens == 0 || columnHasTower || !optimalTowerPlacement) && hadToken == false)
                    {
                        Map[j, i] = 0;
                        carryover = 1;
                    }
                }
                if (oldTower != Map[j, i])
                    Build(j, i);
                if (carryover == 0)
                    return;
            }
        }

        Exhausted = true;
    }


    private void Build(int j, int i)
    {
        Traps[j, i] = Map[j, i] switch
        {
            0 => new FireTrapII(),
            1 => new FrostTrapII(),
            2 => new StrengthTower(),
            _ => Traps[j, i]
            
        };
    }

    private void Populate()
    {
        if (_mapIndex == 0)
        for (var j = 0; j < LevelCount; j++)
            for (var i = 0; i < ColumnCount; i++)
                    Build(j, i);
        IncrementList();
        var freezeRounds = 0;
        var freezePower = 0;
        TotalDamage = 0;

        for (var j = 0; j < LevelCount; j++)
        {
            var fireTraps = 0;
            var tower = -1;
            for (var i = 0; i < ColumnCount; i++)
            {
                if (tower == -1 && Map[j, i] == 2)
                    tower = i;
                if (Map[j, i] == 0)
                {
                    if (tower == -1){
                        for (var k = 0; k < ColumnCount; k++)
                            Traps[j, k].DamageMultiplier = 1;
                        break;
                    }
                    fireTraps++;
                    ((FireTrapII)Traps[j, i]).Ignite();
                }
                if (tower == -1)
                    break;
                if (i == ColumnCount - 1 && fireTraps != 0)
                    ((StrengthTower)Traps[j, tower]).Ignite(fireTraps, new FireTrapII().BaseDamage);
            }

            for (var i = 0; i < ColumnCount; i++)
            {

                Traps[j, i].Freeze(0);

                if (Traps[j, i].ApplyFreeze > 0)
                {
                    freezeRounds = Math.Max(freezeRounds, Traps[j, i]!.ApplyFreeze);
                    freezePower = Traps[j, i]!.FreezePower;
                }
                else if (freezeRounds > 0)
                {
                    Traps[j, i]?.Freeze(freezePower);
                    freezeRounds = Math.Max(--freezeRounds, 0);
                }
                TotalDamage += Traps[j, i].TotalDamage;
            }
        }
    }



    public static void FormatText(int trap)
    {
        FormatText();
        switch (trap)
        {
            case 0:
                Console.BackgroundColor = ConsoleColor.DarkRed;
                break;
            case 1:
                Console.BackgroundColor = ConsoleColor.DarkBlue;
                break;
            case 2:
                Console.BackgroundColor = ConsoleColor.DarkYellow;
                Console.ForegroundColor = ConsoleColor.Black;
                break;
        }
    }

    public static void FormatText()
    {
        Console.BackgroundColor = ConsoleColor.Black;
        Console.ForegroundColor = ConsoleColor.White;
    }

    public void PrintDamageToFile()
    {

        for (var j = LevelCount - 1; j >= 0; j--)
        {
            for (var i = 0; i < ColumnCount; i++)
            {
                var round = Traps[j, i].BaseDamage * Traps[j, i].DamageMultiplier;
                var mult = (Traps[j, i].SlowMultiplier + 1);
                var formattedWord = " " + (((mult > 1 ? (mult + "x") : "")).PadRight(3) + (round + " ").PadLeft(5));
                text.Append(formattedWord);
            }

            text.Append("\n");
        }

        var damageOutput = $"\nTotal Damage: {TotalDamage}\nIndex:        {_mapIndex}\n\n";
        text.Append(damageOutput);

    }

    public void PrintDamageToConsole()
    {

        for (var j = LevelCount - 1; j >= 0; j--)
        {
            for (var i = 0; i < ColumnCount; i++)
            {
                FormatText(Map[j, i]);
                var round = Traps[j, i].BaseDamage * Traps[j, i].DamageMultiplier;
                var mult = (Traps[j, i].SlowMultiplier + 1);
                var formattedWord = " " + (((mult > 1 ? (mult + "x") : "")).PadRight(3) + (round + " ").PadLeft(5));
                Console.Write(formattedWord);
            }

            FormatText();
            Console.WriteLine();
        }

        var damageOutput = $"\nTotal Damage: {TotalDamage}\nIndex:        {_mapIndex}\n\n";
        Console.Write(damageOutput);
    }

    internal class Trap
    {
        public static int SpireCap = int.MaxValue;
        public static int LevelCap = int.MaxValue;
        public int BaseDamage;
        public int DamageMultiplier = 1;
        public int SlowMultiplier;

        public int TotalDamage;
        public int ApplyFreeze;
        public int FreezePower;
        public char Mark;

        public bool Frozen;

        public void Freeze(int freezePower)
        {
            if (freezePower == 0)
            {
                SlowMultiplier = freezePower;
                TotalDamage = BaseDamage * (SlowMultiplier + 1) * DamageMultiplier;
                Frozen = false;
            }
            else
            {
                SlowMultiplier = freezePower;
                TotalDamage = BaseDamage * (SlowMultiplier + 1) * DamageMultiplier;
                Frozen = true;
            }
        }

        public void Ignite(int dummy = 0, int dummy1 = 0)
        {
        }
    }

    internal class FireTrap : Trap
    {
        public FireTrap()
        {
            Mark = 'F';
            TotalDamage = BaseDamage = 50;
        }
        public void Ignite(int dummy = 0)
        {
            DamageMultiplier = 2;
            TotalDamage = BaseDamage * (SlowMultiplier + 1) * DamageMultiplier;
        }
    }

    internal class FireTrapII : FireTrap
    {
        public FireTrapII()
        {
            Mark = 'F';
            TotalDamage = BaseDamage = 500;
        }
        public new void Ignite(int dummy = 0)

        {
            DamageMultiplier = 2;
            TotalDamage = BaseDamage * (SlowMultiplier + 1) * DamageMultiplier;
        }
    }

    internal class FrostTrap : Trap
    {
        public FrostTrap()
        {
            Mark = 'I';
            TotalDamage = BaseDamage = 10;
            ApplyFreeze = 3;
            FreezePower = 1;
        }

        public static new void Freeze(int freezePower)
        {
        }
    }


    internal class FrostTrapII : FrostTrap
    {
        public FrostTrapII()
        {
            Mark = 'I';
            TotalDamage = BaseDamage = 50;
            ApplyFreeze = 4;
            FreezePower = 1;
        }
    }



    internal class StrengthTower : Trap
    {

        public StrengthTower()
        {
            Mark = 'T';
            SpireCap = MaxTowers;
            LevelCap = 1;
            BaseDamage = 50;
        }
        public new void Ignite(int fireTrapCount = 0, int fireTrapDamage = 0)
        {
            BaseDamage = fireTrapDamage * 2;
            DamageMultiplier = fireTrapCount;
            TotalDamage = BaseDamage * (SlowMultiplier + 1) * DamageMultiplier;
        }
    }
}

