/*
  This file is part of Joe's Own Editor for Windows.
  Copyright (c) 2014-2020 John J. Jordan.

  Joe's Own Editor for Windows is free software: you can redistribute it 
  and/or modify it under the terms of the GNU General Public License as
  published by the Free Software Foundation, either version 2 of the 
  License, or (at your option) any later version.

  Joe's Own Editor for Windows is distributed in the hope that it will
  be useful, but WITHOUT ANY WARRANTY; without even the implied warranty
  of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with Joe's Own Editor for Windows.  If not, see
  <http://www.gnu.org/licenses/>.
*/

public override bool Execute()
{
    bool result = true;
    
    for (int i = 0; i < SourceFiles.Length; i++)
    {
        var srcfile = SourceFiles[i];
        string filename = srcfile.GetMetadata("FullPath");
        string dstfile;
        
        if (DestinationFolder != null)
        {
            dstfile = Path.Combine(DestinationFolder, Path.GetFileName(filename));
        }
        else
        {
            dstfile = DestinationFiles[i].GetMetadata("FullPath");
        }
        
        Log.LogMessage("Squeezing {0}", Path.GetFileName(filename));
        if (!ProcessFile(filename, dstfile))
        {
            result = false;
        }
    }
    
    return result;
}

private static Encoding Latin1 = Encoding.GetEncoding("iso-8859-1");

// NOTE: This needs to be used for whitespace instead of letting .Net auto-detect it
// in e.g. string.Split, string.Trim so that they don't catch the whitespace in the upper
// 128 bytes of the encoding (since we're *not* really dealing with Latin1 here, we just
// use it to map that stuff back and forth 1:1)
private static char[] whitespace = " \t\n\r\v\f".ToCharArray();

private bool ProcessFile(string infile, string outfile)
{
    var lines = new List<string>(File.ReadAllText(infile, Latin1).Split("\n".ToCharArray()));
    
    var a = new StateAnalyzer();
    int lineNo = 1;
    foreach (string line in lines)
    {
        try
        {
            a.Accept(line);
            lineNo++;
        }
        catch (Exception e)
        {
            Log.LogError("{0}:{1} {2}", Path.GetFileName(infile), lineNo, e.Message);
            return false;
        }
    }
    
    var t = new SyntaxTranslator() { Translation = a.AssignNames() };
    lines.ForEach(t.Accept);
    
    File.WriteAllText(outfile, t.result.ToString(), Latin1);
    return true;
}

abstract class SyntaxReader
{
    private bool seenColors, seenBacktrack, seenStates;
    
    public void Accept(string line)
    {
        line = StripLine(line);
        if (line.Length == 0)
            return;
        else if (line.StartsWith("-"))
        {
            if (seenColors || seenBacktrack || seenStates)
                throw new Exception("Got a backtrack specification too late");
            OnBacktrack(line);
            seenBacktrack = true;
        }
        else if (line.StartsWith("="))
        {
            if (seenStates)
                throw new Exception("Got a color specification too late");
            seenColors = true;
            var colors = line.Split(whitespace, 2);
            if (colors.Length == 1)
                OnColorDef(colors[0].Substring(1), null);
            else
                OnColorDef(colors[0].Substring(1), colors[1]);
        }
        else if (line.StartsWith(":"))
        {
            seenStates = true;
            var state = line.Split(whitespace, 2);
            if (state.Length == 1)
                OnStartState(state[0].Substring(1), null);
            else
                OnStartState(state[0].Substring(1), state[1]);
        }
        else if (line.StartsWith("."))
        {
            OnCondition(line);
        }
        else if (line == "done")
        {
            OnDone();
        }
        else
        {
            if (!seenStates)
                throw new Exception("Unknown line type before states");
            var parts = SplitTransition(line);
            var transition = parts[1].Split(whitespace, 2);
            if (transition.Length == 1)
                OnTransition(parts[0], parts[1], null);
            else if (transition.Length == 2)
                OnTransition(parts[0], transition[0], transition[1]);
            else
                throw new Exception("Invalid state transition");
        }
    }
    
    private string[] SplitTransition(string line)
    {
        StringBuilder firstArg = new StringBuilder();
        string rest = "";
        bool q = false, esc = false;
        
        for (int i = 0; i < line.Length; i++)
        {
            char c = line[i];
            if (c == '"')
            {
                firstArg.Append(c);
                q = esc || !q;
                esc = false;
            }
            else if (q && c == '\\')
            {
                esc = !esc;
                firstArg.Append(c);
            }
            else if (!q && whitespace.Contains(c))
            {
                rest = line.Substring(i + 1);
                break;
            }
            else
            {
                firstArg.Append(c);
                esc = false;
            }
        }
        
        if (q)
            throw new Exception("Unterminated string");
        
        return new string[] { firstArg.ToString(), rest };
    }
    
    private string StripLine(string line)
    {
        StringBuilder result = new StringBuilder();
        bool q = false, esc = false, sp = true;
        
        foreach (char c in line.Trim(whitespace))
        {
            if (c == '"')
            {
                result.Append(c);
                q = esc || !q;
                esc = sp = false;
            }
            else if (q && c == '\\')
            {
                esc = !esc;
                result.Append(c);
            }
            else if (!q && whitespace.Contains(c))
            {
                if (!sp)
                {
                    result.Append(c);
                    sp = true;
                }
            }
            else if (c == '#' && !q)
            {
                break;
            }
            else
            {
                result.Append(c);
                sp = esc = false;
            }
        }
        
        if (q)
            throw new Exception("Unterminated string");
        
        return result.ToString().TrimEnd(whitespace);
    }
    
    public abstract void OnBacktrack(string bt);
    public abstract void OnStartState(string state, string color);
    public abstract void OnColorDef(string name, string colors);
    public abstract void OnTransition(string c, string state, string options);
    public abstract void OnDone();
    public abstract void OnCondition(string text);
}

class StateAnalyzer : SyntaxReader
{
    private Dictionary<string, int> usage = new Dictionary<string, int>();
    
    public override void OnStartState(string state, string color)
    {
        AddUse(state);
    }
    
    public override void OnTransition(string c, string state, string options)
    {
        AddUse(state);
    }
    
    private void AddUse(string state)
    {
        if (this.usage.ContainsKey(state))
            this.usage[state]++;
        else
            this.usage[state] = 1;
    }
    
    public Dictionary<string, string> AssignNames()
    {
        var keys = this.usage.Keys.ToList();
        keys.Sort((x, y) => this.usage[y].CompareTo(this.usage[x]));
        
        Dictionary<string, string> result = new Dictionary<string, string>();
        int i = 0;
        foreach (var name in GetNames())
        {
            if (i >= keys.Count)
                break;
            result[keys[i++]] = name; // keys[i] to only remove comments+whitespace
        }
        
        return result;
    }
    
    private IEnumerable<string> AtoZ()
    {
        for (int i = 0; i < 26; i++)
            yield return new string((char)('A' + i), 1);
        for (int i = 0; i < 26; i++)
            yield return new string((char)('a' + i), 1);
    }
    
    private IEnumerable<string> GetNames()
    {
        foreach (string s in AtoZ())
            yield return s;
        foreach (string sub in GetNames())
            foreach (string s in AtoZ())
                yield return sub + s;
    }

    public override void OnBacktrack(string bt) { }
    public override void OnColorDef(string name, string colors) { }
    public override void OnDone() { }
    public override void OnCondition(string text) { }
}

class SyntaxTranslator : SyntaxReader
{
    public Dictionary<string, string> Translation { get; set; }
    public StringBuilder result = new StringBuilder();
    
    public override void OnBacktrack(string bt)
    {
        result.Append(bt);
        result.Append('\n');
    }
    
    public override void OnStartState(string state, string color)
    {
        result.Append(':');
        result.Append(this.Translation[state]);
        if (color != null)
        {
            result.Append(' ');
            result.Append(color);
        }
        
        result.Append('\n');
    }
    
    public override void OnColorDef(string name, string colors)
    {
        result.Append('=');
        result.Append(name);
        if (colors != null)
        {
            result.Append(' ');
            result.Append(colors);
        }
        
        result.Append('\n');
    }
    
    public override void OnTransition(string c, string state, string options)
    {
        result.Append(c);
        result.Append(' ');
        result.Append(this.Translation[state]);
        if (options != null)
        {
            result.Append(' ');
            result.Append(options);
        }
        
        result.Append('\n');
    }
    
    public override void OnDone()
    {
        result.Append("done\n");
    }
    
    public override void OnCondition(string text)
    {
        result.Append(text);
        result.Append('\n');
    }
}
