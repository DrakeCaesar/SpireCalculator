// See https://aka.ms/new-console-template for more information
using System.Collections.Generic;
using System;
using System.ComponentModel.DataAnnotations;
using System.Linq.Expressions;
using Microsoft.VisualBasic.CompilerServices;
using System.Reflection.Emit;
using System.Runtime.Serialization;
using System.Drawing;
using static Spire;
using System.Reflection.Metadata.Ecma335;


var maxDamage = 0;

for (; ; )
{
    var spire = new Spire();
    if (spire.totalDamageWithBonus > maxDamage)
    {
        maxDamage = spire.totalDamageWithBonus;
        //spire.print();
        spire.printDamage();
        spire.printDamageWithBonus();
    }
}




internal class Spire
{
    private static int levelCount = 4;
    private static int maxTowers = 2;

    private static int columnCount = 5;
    private int sortBonus = levelCount * columnCount + 1;

    private List<List<dynamic>> traps = new();
    public int totalDamage = 0;
    public int totalDamageWithBonus = 0;

    private static int[,] map = new int[levelCount, columnCount];


    static Random random = new();

    public List<Trap> pool = new()
    {
        new FireTrapI(),
        new FrostTrapII(),
        new StrengthTowerI(),
    };

    public Spire()
    {
        populate();

    }
    private static long mapIndex = 0;
    private static long maxMapIndex = (long)Math.Pow(3.0, levelCount * columnCount);

    private static int towerTokens = 2;

    private void IncrementList()
    {
        ++mapIndex;
        var carryover = 1;
        for (var j = 0; j < levelCount; j++)
        {
            var columnHasTower = false;

            for (var i = 0; i < columnCount; i++)
            {
                if (map[j, i] == 2)
                    columnHasTower = true;
            }

            for (var i = 0; i < columnCount; i++)
            {
                var hadToken = map[j, i] == 2;
                map[j, i] += carryover;
                carryover = 0;

                if (map[j, i] == 3)
                {
                    carryover = 1;
                    columnHasTower = false;
                    map[j, i] = 0;

                    towerTokens = hadToken ? ++towerTokens : --towerTokens;
                }
                else if (map[j, i] == 2)
                {
                    if (towerTokens > 0 && columnHasTower == false)
                    {
                        --towerTokens;
                        columnHasTower = true;
                    }
                    else if ((towerTokens == 0 || columnHasTower) && hadToken == false)
                    {
                        map[j, i] = 0;
                        carryover = 1;
                    }
                }
            }
        }

        if (carryover <= 0) return;
        Console.WriteLine("Tested All");
        printMap();
    }

    private void printMap()
    {
        for (var j = 0; j < levelCount; j++)
        {
            for (var i = 0; i < columnCount; i++)
            {
                Console.Write(map[j, i].ToString().PadRight(6));
            }
            Console.WriteLine();
        }
        Console.WriteLine();
    }

    private void populate()
    {
        IncrementList();

        for (var j = 0; j < levelCount; j++)
        {
            var levelPool = pool.ToList();
            var level = new List<dynamic?>();
            for (var i = 0; i < columnCount; i++)
            {
                levelPool.RemoveAll(x => x.SpireCap <= 0);
                var chosen = levelPool[map[j, i]];
                if (chosen is StrengthTowerI)
                {
                    levelPool.Remove(chosen);
                    chosen.SpireCap--;
                }
                var type = chosen.GetType();
                level.Add((Trap)Activator.CreateInstance(type)!);
                if (level.Last() is StrengthTowerI)
                {
                    ((level.Last() as Trap)!).sortBonus = columnCount - i;
                }
            }

            traps.Add(level!);
        }



        var freezeRounds = 0;
        var freezePower = 0;
        foreach (var level in traps)
        {
            var fireTraps = level.FindAll(x => x is FireTrapI);
            var tower = level.Find(x => x is StrengthTowerI);

            if (tower != null)
            {
                foreach (var fireTrap in fireTraps)
                {
                    fireTrap?.Ignite();
                }
                tower.Ignite(fireTraps.Count);
            }
            foreach (var trap in level)
            {
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
            }
        }


        totalDamage = traps.SelectMany(level => level.Cast<Trap>()).Sum(trap => trap!.TotalDamage);
        totalDamageWithBonus = traps.SelectMany(level => level.Cast<Trap>()).Sum(trap => trap!.TotalDamage + trap.sortBonus);
    }

    public void print()
    {
        traps.Reverse();
        foreach (var level in traps)
        {
            foreach (Trap trap in level)
            {
                formatText(trap);
                Console.Write(trap!.Mark.ToString().PadRight(6));
            }

            formatText();
            Console.WriteLine();
        }
        Console.WriteLine();
        traps.Reverse();
    }

    public void formatText(dynamic trap)
    {
        formatText();
        switch (trap)
        {
            case FireTrapI:
                Console.BackgroundColor = ConsoleColor.DarkRed;
                break;
            case FrostTrapI:
                Console.BackgroundColor = ConsoleColor.DarkBlue;
                break;
            case StrengthTowerI:
                Console.BackgroundColor = ConsoleColor.DarkYellow;
                Console.ForegroundColor = ConsoleColor.Black;
                break;
        }
        if (trap.Frozen)
        {
            //Console.ForegroundColor = ConsoleColor.DarkCyan;
        }
    }

    public void formatText()
    {
        Console.BackgroundColor = ConsoleColor.Black;
        Console.ForegroundColor = ConsoleColor.White;
    }

    public void printDamage()
    {
        traps.Reverse();

        foreach (var level in traps)
        {
            foreach (Trap trap in level)
            {
                formatText(trap);
                var round = trap.BaseDamage * trap.DamageMultiplier;
                var mult = (trap.SlowMultiplier + 1);
                Console.Write(((mult > 1 ? (mult + "x") : "") + round).PadRight(6));
            }
            formatText();
            Console.WriteLine();
        }

        Console.WriteLine();
        Console.WriteLine("Total Damage: " + totalDamage);
        Console.WriteLine("Index:        " + mapIndex + " / " + maxMapIndex);
        Console.WriteLine();
        traps.Reverse();

    }

    public void printDamageWithBonus()
    {
        traps.Reverse();

        foreach (var level in traps)
        {
            foreach (Trap trap in level)
            {
                formatText(trap);
                //Console.Write(trap.TotalDamageWithBonus.ToString().PadRight(6));
                Console.Write((trap!.TotalDamage + trap.sortBonus).ToString().PadRight(6));

            }
            formatText();
            Console.WriteLine();
        }

        Console.WriteLine();
        Console.WriteLine("Total Damage: " + totalDamageWithBonus);
        Console.WriteLine();
        traps.Reverse();

    }

    internal class Trap
    {
        public int SpireCap = int.MaxValue;
        public int LevelCap = int.MaxValue;
        public int BaseDamage = 0;
        public int DamageMultiplier = 1;
        public int SlowMultiplier = 0;
        public int sortBonus = 0;

        public int TotalDamage = 0;
        public int ApplyFreeze = 0;
        public int FreezePower = 0;
        public char Mark;



        public bool Frozen;
        public bool Ignited;

        public void Freeze(int FreezePower)
        {
            if (FreezePower == 0) return;
            if (Frozen) return;
            SlowMultiplier = FreezePower;
            TotalDamage = BaseDamage * (SlowMultiplier + 1) * DamageMultiplier;
            Frozen = true;
        }

        public void Ignite(int fireTowerCount = 0)
        {

        }
    }

    internal class FireTrapI : Trap
    {
        public FireTrapI()
        {
            Mark = 'F';
            TotalDamage = BaseDamage = 50;
        }

        public new void Ignite(int fireTowerCount = 0)
        {
            DamageMultiplier = 2;
            TotalDamage = BaseDamage * (SlowMultiplier + 1) * DamageMultiplier;
            Ignited = true;
        }
    }

    internal class FrostTrapI : Trap
    {
        public FrostTrapI()
        {
            Mark = 'I';
            TotalDamage = BaseDamage = 10;
            ApplyFreeze = 3;
            FreezePower = 1;
        }

        public new void Freeze(int FreezePower)
        {
        }
    }

    internal class FrostTrapII : FrostTrapI
    {
        public FrostTrapII()
        {
            Mark = 'I';
            TotalDamage = BaseDamage = 50;
            ApplyFreeze = 4;
            FreezePower = 1;
        }
    }



    internal class StrengthTowerI : Trap
    {

        public StrengthTowerI()
        {
            Mark = 'T';
            SpireCap = maxTowers;
            LevelCap = 1;
            BaseDamage = 100;
            sortBonus = 0;
        }

        public new void Ignite(int fireTowerCount)
        {
            DamageMultiplier = fireTowerCount;
            TotalDamage = BaseDamage * (SlowMultiplier + 1) * DamageMultiplier;
            Ignited = true;

        }
    }

}

