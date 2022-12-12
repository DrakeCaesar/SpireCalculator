// See https://aka.ms/new-console-template for more information
using System.Collections.Generic;
using System;
using System.Linq;
using System.Threading;


var maxDamage = 0;

for (; ; )
{
    var spire = new Spire();
    if (spire.TotalDamageWithBonus > maxDamage)
    {
        Spire.copyToBestMap();
        maxDamage = spire.TotalDamageWithBonus;
        //spire.print();
        spire.PrintDamage();
        spire.PrintDamageWithBonus();
    }

    if (Spire.Exhausted)
    {
        Console.ReadLine();
        return;
    }
}





internal class Spire
{
    public static readonly int LevelCount = 5;
    private static readonly int MaxTowers = 2;

    private static readonly int ColumnCount = 5;

    private readonly List<List<Trap>> _traps = new();
    public int TotalDamage = 0;
    public int TotalDamageWithBonus = 0;

    public static int[,] Map = new int[LevelCount, ColumnCount];
    public static int[,] bestMap = new int[LevelCount, ColumnCount];
    public static int bestTowerTokens = 0;

    internal static bool Exhausted = false;

    public List<Type> Pool = new()
    {
        typeof(FireTrap),
        typeof(FrostTrapII),
        typeof(StrengthTower),
    };

    public Spire()
    {
        Populate();

    }
    private static long _mapIndex = 0;
    private static readonly long MaxMapIndex = (long)Math.Pow(3.0, LevelCount * ColumnCount);

    private static int _towerTokens = MaxTowers;
    private static int offset = 2;
    public static int locked = 0;

    public static void copyToBestMap()
    {
        
        for (var j = 0; j < LevelCount; j++)   
            for (var i = 0; i<ColumnCount; i++)
                bestMap[j, i] = Map[j, i];
        bestTowerTokens = _towerTokens;


    }

    public static void copyToMap()
    {
        for (var j = 0; j <= locked - 1; j++)
            for (var i = 0; i < ColumnCount; i++)
                Map[j, i] = bestMap[j, i];
        //_towerTokens = bestTowerTokens;
    }

    private static void IncrementList()
    {

         
        ++_mapIndex;


        
        var carryover = 1;
        
        for (var j = locked; j < LevelCount; j++)
        {
            if (j == 4 && carryover == 1)
            {
                var bug = true;
            }

            var columnHasTower = false;
            if (j > offset  && carryover == 1)
            {
                if (j - offset > locked)
                {

                    locked = j - offset;
                    copyToMap();
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
                    if (_towerTokens > 0 && columnHasTower == false)
                    {
                        --_towerTokens;
                        columnHasTower = true;
                    }
                    else if ((_towerTokens == 0 || columnHasTower) && hadToken == false)
                    {
                        Map[j, i] = 0;
                        carryover = 1;
                    }
                }
            }
        }

        if (carryover <= 0) return;
        Console.WriteLine("Tested All");
        Spire.Exhausted = true;
        PrintMap();
    }

    private static void PrintMap()
    {
        for (var j = 0; j < LevelCount; j++)
        {
            for (var i = 0; i < ColumnCount; i++)
            {
                Console.Write(Map[j, i].ToString().PadRight(6));
            }
            Console.WriteLine();
        }
        Console.WriteLine();
    }

    private void Populate()
    {
        IncrementList();

        for (var j = 0; j < LevelCount; j++)
        {
            var level = new List<Trap>();
            for (var i = 0; i < ColumnCount; i++)
            {
                level.Add((Trap)Activator.CreateInstance(Pool[Map[j, i]])!);
                if (level.Last() is StrengthTower)
                {
                    ((level.Last() as Trap)!).SortBonus = ColumnCount - i;
                }
            }

            _traps.Add(level!);
        }

        var freezeRounds = 0;
        var freezePower = 0;
        foreach (var level in _traps)
        {
            var fireTraps = level.FindAll(x => x is FireTrap);
            var tower = level.Find(x => x is StrengthTower);

            if (tower != null)
            {
                foreach (var fireTrap in fireTraps)
                {
                    (fireTrap as FireTrap)?.Ignite();
                }
                (tower as StrengthTower)?.Ignite(fireTraps.Count);
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


        TotalDamage = _traps.SelectMany(level => level).Sum(trap => trap.TotalDamage);
        TotalDamageWithBonus = _traps.SelectMany(level => level).Sum(trap => trap.TotalDamage + trap.SortBonus);
    }

    public void Print()
    {
        _traps.Reverse();
        foreach (var level in _traps)
        {
            foreach (var trap in level)
            {
                FormatText(trap);
                Console.Write(trap!.Mark.ToString().PadRight(6));
            }

            FormatText();
            Console.WriteLine();
        }
        Console.WriteLine();
        _traps.Reverse();
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

    public void PrintDamage()
    {
        _traps.Reverse();

        foreach (var level in _traps)
        {
            foreach (Trap trap in level)
            {
                FormatText(trap);
                var round = trap.BaseDamage * trap.DamageMultiplier;
                var mult = (trap.SlowMultiplier + 1);
                Console.Write(((mult > 1 ? (mult + "x") : "") + round).PadRight(6));
            }
            FormatText();
            Console.WriteLine();
        }

        Console.WriteLine();
        Console.WriteLine("Total Damage: " + TotalDamage);
        Console.WriteLine("Index:        " + _mapIndex + " / " + MaxMapIndex);
        Console.WriteLine();
        _traps.Reverse();

    }

    public void PrintDamageWithBonus()
    {
        _traps.Reverse();

        foreach (var level in _traps)
        {
            foreach (Trap trap in level)
            {
                FormatText(trap);
                //Console.Write(trap.TotalDamageWithBonus.ToString().PadRight(6));
                Console.Write((trap!.TotalDamage + trap.SortBonus).ToString().PadRight(6));

            }
            FormatText();
            Console.WriteLine();
        }

        Console.WriteLine();
        Console.WriteLine("Total Damage: " + TotalDamageWithBonus);
        Console.WriteLine();
        _traps.Reverse();

    }

    internal class Trap
    {
        public int SpireCap = int.MaxValue;
        public int LevelCap = int.MaxValue;
        public int BaseDamage = 0;
        public int DamageMultiplier = 1;
        public int SlowMultiplier = 0;
        public int SortBonus = 0;

        public int TotalDamage = 0;
        public int ApplyFreeze = 0;
        public int FreezePower = 0;
        public char Mark;



        public bool Frozen;
        public bool Ignited;

        public void Freeze(int freezePower)
        {
            if (freezePower == 0) return;
            if (Frozen) return;
            SlowMultiplier = freezePower;
            TotalDamage = BaseDamage * (SlowMultiplier + 1) * DamageMultiplier;
            Frozen = true;
        }

        public void Ignite(int dummy = 0)
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
            BaseDamage = 100;
            SortBonus = 0;
        }
        public new void Ignite(int FireTrapCount = 0)
        {
            DamageMultiplier = FireTrapCount;
            TotalDamage = BaseDamage * (SlowMultiplier + 1) * DamageMultiplier;
        }
    }

}

