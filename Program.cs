// See https://aka.ms/new-console-template for more information
using System.Collections.Generic;
using System;
using System.Linq.Expressions;
using Microsoft.VisualBasic.CompilerServices;
using System.Reflection.Emit;
using System.Runtime.Serialization;
using System.Drawing;


var maxDamage = 0;

for (;;)
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
    private int sortBonus = levelCount * columnCount +1;

    private List<List<dynamic>> traps = new();
    public int totalDamage = 0;
    public int totalDamageWithBonus = 0;

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

    private void populate()
    {
        
        for (var j = 0; j < levelCount; j++)
        {
            var levelPool = pool.ToList();
            var level = new List<dynamic?>();
            for (var i = 0; i < columnCount; i++)
            {
                levelPool.RemoveAll(x => x.SpireCap <= 0);
                var chosen = levelPool[random.Next(levelPool.Count)];
                if (chosen is StrengthTowerI)
                {
                    levelPool.Remove(chosen);
                    chosen.SpireCap--;
                }
                var type = chosen.GetType();
                level.Add((Trap)Activator.CreateInstance(type)!);
                ((level.Last() as Trap)!).sortBonus = --sortBonus;
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
        totalDamageWithBonus = (int)traps.SelectMany(level => level.Cast<Trap>()).Sum(trap => trap!.TotalDamage*trap.sortBonus*0.005);
        totalDamageWithBonus = totalDamage;
        //print();

        //printDamage();
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
                Console.Write(((int) (trap!.TotalDamage * trap.sortBonus * 0.005)).ToString().PadRight(6));

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
        public int TotalDamageWithBonus = 0;
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
            TotalDamageWithBonus = TotalDamage + sortBonus;
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
            TotalDamageWithBonus = TotalDamage + sortBonus;
        }

        public new void Ignite(int fireTowerCount = 0)
        {
            DamageMultiplier = 2;
            TotalDamage = BaseDamage * (SlowMultiplier + 1) * DamageMultiplier;
            TotalDamageWithBonus = TotalDamage + sortBonus;
            Ignited = true;
        }
    }

    internal class FrostTrapI : Trap
    {
        public FrostTrapI()
        {
            Mark = 'I';
            TotalDamage = BaseDamage = 10;
            TotalDamageWithBonus = TotalDamage + sortBonus;
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
            TotalDamageWithBonus = TotalDamage + sortBonus;
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
        }

        public new void Ignite(int fireTowerCount)
        {
            DamageMultiplier = fireTowerCount;
            TotalDamage = BaseDamage * (SlowMultiplier + 1) * DamageMultiplier;
            TotalDamageWithBonus = TotalDamage + sortBonus;
            Ignited = true;

        }
    }

}

