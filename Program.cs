#define FireTower

// See https://aka.ms/new-console-template for more information
using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;

var maxDamage = 0;

for (; ; )
{
    var spire = new Spire();
    if (spire.TotalDamage >= maxDamage)
    {
        Spire.CopyToBestMap();
        maxDamage = spire.TotalDamage;
        spire.PrintDamageToFile();
    }

    if (Spire.Exhausted)
    {
        var expectedOutput = File.ReadAllText("../../../expectedOutput.txt");
        File.WriteAllText("../../../output.txt", Spire.text);
        if (expectedOutput != Spire.text )
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

    private readonly Trap[,] _traps = new Trap[LevelCount, ColumnCount];
    public int TotalDamage;

    public static byte[,] Map = new byte[LevelCount, ColumnCount];
    public static byte[,] BestMap = new byte[LevelCount, ColumnCount];
    public static int BestTowerTokens;

    internal static bool Exhausted;
    public static string text = string.Empty;


    public List<Type> Pool = new()
    {
        typeof(FireTrapII),
        typeof(FrostTrapII),
        typeof(StrengthTower),
    };

    public Spire()
    {
        Populate();

    }
    private static long _mapIndex;
    private static int _towerTokens = MaxTowers;
    private const int Offset = 2;
    public static int Locked;

    public static void CopyToBestMap()
    {
        
        for (var j = 0; j < LevelCount; j++)   
            for (var i = 0; i<ColumnCount; i++)
                BestMap[j, i] = Map[j, i];
        BestTowerTokens = _towerTokens;
    }

    public static void CopyToMap()
    {
        _towerTokens = 0;
        for (var j = 0; j < LevelCount; j++)
            for (var i = 0; i < ColumnCount; i++)
            {
                Map[j, i] = (byte)(j < Locked ? BestMap[j, i] : 0);
                if (Map[j, i] == 2) 
                    ++_towerTokens;
            }
    }

    private static void IncrementList()
    {
        ++_mapIndex;
        byte carryover = 1;
        
        for (var j = Locked; j < LevelCount; j++)
        {
            var columnHasTower = false;
            if (j > Offset  && carryover == 1)
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
                else if (Map[j, i] == 2)
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
                if (carryover == 0)
                    return;
            }
        }

        Exhausted = true;
    }



    private void Populate()
    {
        IncrementList();

        for (var j = 0; j < LevelCount; j++)
            for (var i = 0; i < ColumnCount; i++)
                _traps[j, i] = Map[j, i] switch
                {
                    0 => new FireTrapII(),
                    1 => new FrostTrapII(),
                    2 => new StrengthTower(),
                    _ => _traps[j, i]
                };

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
                    if (tower == -1)
                        break;
                    fireTraps++;
                    ((FireTrapII)_traps[j,i]).Ignite();
                }
                if (tower == -1)
                    break;
                if (i == ColumnCount - 1 && tower != -1 && fireTraps != 0)
                    ((StrengthTower)_traps[j,tower]).Ignite(fireTraps, new FireTrapII().BaseDamage);
            }

            for (var i = 0; i < ColumnCount; i++)
            {
                var trap = _traps[j,i];
                if (trap.ApplyFreeze > 0)
                {
                    freezeRounds = Math.Max(freezeRounds, trap!.ApplyFreeze);
                    freezePower = trap!.FreezePower;
                }
                else if (freezeRounds > 0)
                {
                    trap?.Freeze(freezePower);
                    freezeRounds = Math.Max(--freezeRounds, 0);
                }
                TotalDamage += _traps[j, i].TotalDamage;
            }
        }
    }



    public static void FormatText(dynamic trap)
    {
        FormatText();
        switch (trap)
        {
            case FireTrap:
                Console.BackgroundColor = ConsoleColor.DarkRed;
                break;
            case FrostTrap:
                Console.BackgroundColor = ConsoleColor.DarkBlue;
                break;
            case StrengthTower:
                Console.BackgroundColor = ConsoleColor.DarkYellow;
                Console.ForegroundColor = ConsoleColor.Black;
                break;
        }
        if (trap.Frozen)
        {
            //Console.ForegroundColor = ConsoleColor.DarkCyan;
        }
    }

    public static void FormatText()
    {
        Console.BackgroundColor = ConsoleColor.Black;
        Console.ForegroundColor = ConsoleColor.White;
    }

    public void PrintDamageToFile()
    {

        for (var j = LevelCount - 1; j >= 0 ; j--)
        {
            for (var i = 0; i < ColumnCount; i++)
            {
                //FormatText(trap);
                var round = _traps[j,i].BaseDamage * _traps[j, i].DamageMultiplier;
                var mult = (_traps[j, i].SlowMultiplier + 1);
                var formattedWord = " " + (((mult > 1 ? (mult + "x") : "")).PadRight(3) + (round + " ").PadLeft(5));
                //Console.Write(formattedWord);
                text += formattedWord;
            }

            //FormatText();
            //Console.WriteLine();
            text += "\n";
        }

        var damageOutput = $"\nTotal Damage: {TotalDamage}\nIndex:        {_mapIndex}\n\n";
        //Console.Write(damageOutput);
        text += damageOutput;

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
            if (freezePower == 0) return;
            if (Frozen) return;
            SlowMultiplier = freezePower;
            TotalDamage = BaseDamage * (SlowMultiplier + 1) * DamageMultiplier;
            Frozen = true;
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
            TotalDamage = BaseDamage* (SlowMultiplier + 1) * DamageMultiplier;
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

