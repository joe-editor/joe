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
    StringBuilder result = new StringBuilder();
    State state = new State();
    int lineNo = 1;
    bool ret = true;
    
    foreach (string line in File.ReadAllText(infile, Latin1).Split("\n".ToCharArray()))
    {
        string s = null;
        
        try
        {
            s = SqueezeLine(line, state);
            lineNo++;
        }
        catch (Exception e)
        {
            Log.LogError("{0}:{1} {2}", Path.GetFileName(infile), lineNo, e.Message);
            ret = false;
        }
        
        if (!string.IsNullOrEmpty(s))
        {
            if (s.StartsWith(":include"))
                s = ":include *" + s.Substring(8).Trim(whitespace);
            
            if (result.Length > 0)
                result.Append('\n');

            result.Append(s);
        }
    }
    
    if (ret)
        File.WriteAllText(outfile, result.ToString(), Latin1);

    return ret;
}

private string SqueezeLine(string line, State s)
{
    if (string.IsNullOrEmpty(line))
        return null;
    if (!s.inBrace && whitespace.Contains(line[0]))
        return null;
    
    if (line[0] == '{')
        s.inBrace = true;
    if (s.inBrace)
    {
        if (line[0] == '}')
            s.inBrace = false;
        return line.TrimEnd("\r\n".ToCharArray());
    }
    
    var firstWord = line.Split(whitespace)[0];
    
    if (s.inSettings)
    {
        if (line[0] == '-')
        {
            var cmd = firstWord.Substring(1);
            if (cmd.StartsWith("-"))
                return firstWord;
            if (FlagSettings.Contains(cmd.ToLower()))
                return firstWord;
            else if (OneParamSettings.Contains(cmd.ToLower()))
                return string.Format("{0} {1}", firstWord, line.Split(whitespace)[1]);
            else
            {
                if (!LineParamSettings.Contains(cmd.ToLower()))
                    Log.LogWarning("Unsure of what to do with {0}, taking whole line", cmd);
                return line.Trim(whitespace);
            }
        }
        
        if (line[0] == '=')
            return string.Join(" ", line.Split(whitespace, StringSplitOptions.RemoveEmptyEntries));
        
        if (line[0] == '+')
            return line.Trim(whitespace);
    }
    
    if (firstWord.ToLower() == ":defmenu")
    {
        s.Clear();
        s.inMenus = true;
    }
    else if (KeySections.Contains(firstWord.ToLower()))
    {
        s.Clear();
        s.inKeys = true;
    }
    
    // Minify spaces and cut off when two consecutive spaces are found.
    StringBuilder result = new StringBuilder();
    bool quote = false, esc = false;
    int spaces = 0, maxspaces = 2;
    
    if (line.StartsWith("mode,") || line.StartsWith("menu,"))
        maxspaces = int.MaxValue;
    
    for (int i = 0; i < line.Length; i++)
    {
        char c = line[i];
        bool keep = true;
        
        if (quote)
        {
            if (c == '"')
                quote = false;
            else if (c == '\\')
                esc = true;
            else
                esc = false;
        }
        else
        {
            if (whitespace.Contains(c))
            {
                spaces++;
                if (spaces == maxspaces)
                    break;
                keep = false;
            }
            else if (c == '"')
            {
                quote = true;
                esc = false;
            }
        }
        
        if (s.inKeys && spaces > 0 && !line.StartsWith(":"))
        {
            // Divert here and handle key input specially.
            var parts = line.Substring(i + 1).Split(whitespace, StringSplitOptions.RemoveEmptyEntries);
            foreach (var p in parts)
            {
                if (p.Length == 1 || KeyPrefixes.Any(k => p.StartsWith(k)) || SpecialKeys.Contains(p))
                    result.Append(" " + p);
                else
                    break;
            }
            
            break;
        }
        
        if (keep)
        {
            if (spaces > 0)
            {
                result.Append(' ');
                spaces = 0;
                if (s.inKeys)
                {
                    // Special key input: After the first set of spaces, cut off at 2.
                    maxspaces = 2;
                }
            }
            
            result.Append(c);
        }
    }
    
    return result.ToString();
}

class State
{
    public bool inBrace = false;
    public bool inSettings = true;
    public bool inMenus = false;
    public bool inKeys = false;
    
    public void Clear()
    {
        this.inBrace = this.inSettings = this.inMenus = this.inKeys = false;
    }
}

public static List<string> KeySections = new List<string> { ":windows", ":main", ":inherit", ":prompt", ":query", ":querya", ":querysr" };

public static List<string> KeyPrefixes = new List<string> { "^", ".k", ".f", ".F", ".@", "U+" };

public static List<string> SpecialKeys = new List<string> {
    "SP", "TO", "MDOWN", "MUP", "MDRAG", "M2DOWN", "M2UP", "M2DRAG", "M3DOWN", "M3UP", "M3DRAG", "MWDOWN", "MWUP",
    "MRUP", "MRDOWN", "MRDRAG", "MIDDLEUP", "MIDDLEDOWN", "MIDDLEDRAG"
};

public static List<string> FlagSettings = new List<string> {
    "asis", "assume_256color", "assume_color", "autoindent", "autoswap", "beep", "break_hardlinks", "break_links",
    "c_comment", "cpp_comment", "crlf", "csmode", "dopadding", "exask", "floatmouse", "flowed", "force", "french",
    "guess_crlf", "guess_indent", "guess_non_utf8", "guess_utf8", "helpon", "help_is_utf8", "hex", "highlight", "icase",
    "indentfirst", "joe_state", "joexterm", "keepup", "lightoff", "linums", "marking", "menu_above", "menu_explorer",
    "menu_jump", "mid", "mouse", "no_double_quoted", "nobackup", "nobackups", "nocurdir", "nolocks", "nomodcheck",
    "nonotice", "nosta", "notite", "noxon", "orphan", "overwrite", "picture", "pound_comment", "purify", "rdonly",
    "restore", "rtbutton", "search_prompting", "semi_comment", "single_quoted", "smarthome", "smartbacks", "spaces",
    "square", "tex_comment", "transpose", "usetabs", "vhdl_comment", "wordwrap", "wrap",
    "nodeadjoe", "noexmsg", "nolinefeeds", "highlighter_context", "guess_utf16", "brpaste", "pastehack", "regex",
    "title"
};

public static List<string> OneParamSettings = new List<string> {
    "baud", "columns", "encoding", "help_color", "indentc", "istep", "keymap", "lines", "lmargin", "menu_color",
    "msg_color", "pg", "prompt_color", "rmargin", "skiptop", "status_color", "syntax", "tab", "text_color",
    "undo_keep", "left", "right"
};

public static List<string> LineParamSettings = new List<string> {
    "backpath", "cpara", "lmsg", "mfirst", "mnew", "mold", "msnew", "msold", "rmsg", "text_delimiters",
    "smsg", "zmsg", "xmsg", "aborthint", "helphint"
};
