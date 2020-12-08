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
        ProcessFile(filename, dstfile);
    }
    
    return true;
}

private Encoding Latin1 = Encoding.GetEncoding("iso-8859-1");

// NOTE: This needs to be used for whitespace instead of letting .Net auto-detect it
// in e.g. string.Split, string.Trim so that they don't catch the whitespace in the upper
// 128 bytes of the encoding (since we're *not* really dealing with Latin1 here, we just
// use it to map that stuff back and forth 1:1)
private char[] whitespace = " \t\n\r\v\f".ToCharArray();

private void ProcessFile(string infile, string outfile)
{
    var lines = new List<string>(File.ReadAllText(infile, Latin1).Split("\n".ToCharArray()));
    lines.Add("");
    
    StringBuilder result = new StringBuilder();
    StringBuilder running = null;
    foreach (var line in lines)
    {
        var lstr = line.Trim(whitespace);
        if (running != null)
        {
            if (lstr.StartsWith("\""))
            {
                var end = lstr.LastIndexOf('"');
                running.Append(lstr.Substring(1, end - 1));
                continue;
            }
            else
            {
                running.Append("\"\n");
                result.Append(running.ToString());
                running = null;
            }
        }
        
        if (lstr.StartsWith("#") || lstr.Length == 0)
        {
            continue;
        }
        
        if (lstr.IndexOf('"') >= 0)
        {
            var q = lstr.IndexOf('\"');
            var rq = lstr.LastIndexOf('"');
            result.Append(lstr.Substring(0, q));
            running = new StringBuilder();
            running.Append(lstr.Substring(q, rq - q)); /* Quote included here */
        }
        else
        {
            result.Append(lstr);
            result.Append("\n");
        }
    }
    
    File.WriteAllText(outfile, result.ToString(), Latin1);
}
