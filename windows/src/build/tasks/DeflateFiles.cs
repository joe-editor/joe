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

foreach (var file in this.InputFiles)
{
    var infile = file.GetMetadata("FullPath");
    var outfile = Path.Combine(this.OutputDirectory, Path.GetFileName(infile)) + ".dfl";
    
    using (var zstream = File.Open(outfile, FileMode.Create, FileAccess.Write))
    {
        /* Header */
        byte[] hdr = { 5, 1 };
        zstream.Write(hdr, 0, 2);
        
        using (var dstream = new DeflateStream(zstream, CompressionLevel.Optimal))
        {
            using (var istream = File.Open(infile, FileMode.Open, FileAccess.Read))
            {
                istream.CopyTo(dstream, 8192);
            }
        }
    }
}
