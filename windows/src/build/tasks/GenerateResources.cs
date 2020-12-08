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

List<string> files = new List<string>(this.Specs.Select(s => s.GetMetadata("FullPath")));

if (files.Count == 0) return true;

using (var outs = new StreamWriter(File.Open(this.OutputFile, FileMode.Create, FileAccess.Write)))
{
    for (int i = 0; i < files.Count; i++)
    {
        foreach (string line in File.ReadAllLines(files[i]))
        {
            string s = line.Trim();
            if (string.IsNullOrEmpty(s)) continue;
            
            if (s.StartsWith("#include"))
            {
                string incfile = s.Substring(s.IndexOf(' ')).Trim();
                files.Add(Path.Combine(Path.GetDirectoryName(files[i]), incfile));
                continue;
            }
            
            List<string> options = null;
            
            if (s.Contains('['))
            {
                int lb = s.IndexOf('[');
                int rb = s.IndexOf(']');
                string opts = s.Substring(lb + 1, rb - lb - 1);
                options = opts.Split().ToList();
                s = s.Substring(0, lb).Trim();
            }
            
            string srcfile, dstfile;
            
            if (s.Contains("=>"))
            {
                string[] parts = s.Split(new string[] { "=>" }, StringSplitOptions.None);
                srcfile = parts[0].Trim();
                dstfile = parts[1].Trim();
            }
            else
            {
                srcfile = dstfile = s;
            }
            
            string actualFile = Path.Combine(this.FileLocation, srcfile);
            if (!File.Exists(actualFile))
            {
                this.Success = false;
                Log.LogError("Input file does not exist: {0}", actualFile);
                return false;
            }
            
            if (options != null && options.Contains("compress"))
            {
                actualFile += ".dfl";
            }
            
            outs.WriteLine(string.Format("F:*{0} RCDATA \"{1}\"", dstfile, actualFile.Replace(@"\", @"\\")));
        }
    }
}
