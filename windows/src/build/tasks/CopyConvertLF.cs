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

for (int i = 0; i < SourceFiles.Length; i++)
{
    var srcfile = SourceFiles[i];
    string filename = srcfile.GetMetadata("FullPath");
    byte[] infile = File.ReadAllBytes(filename);
    string outfilename;
    
    if (DestinationFolder != null)
    {
        outfilename = Path.Combine(DestinationFolder, Path.GetFileName(filename));
    }
    else
    {
        outfilename = DestinationFiles[i].GetMetadata("FullPath");
    }
    
    using (var outf = File.Open(outfilename, FileMode.Create, FileAccess.Write))
    {
        int lastWrite = 0;
        for (int t = 0; t < infile.Length; t++)
        {
            if (infile[t] == 10 && infile[Math.Max(0, t - 1)] != 13)
            {
                infile[t] = 13; // I'm all about avoiding that extra Write =p
                outf.Write(infile, lastWrite, t - lastWrite + 1);
                infile[t] = 10;
                lastWrite = t;
            }
        }
    
        outf.Write(infile, lastWrite, infile.Length - lastWrite);
    }
}
