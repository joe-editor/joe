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

public enum BuildType
{
    Official,
    Nightly,
    Developer
}

public struct Revision
{
    public string Parent;
    public string Branch;
    public bool Valid;
}

public override bool Execute()
{
    Version v = new Version(this.VersionNumber);
    Revision rev = GetMercurialRev();
    string revstr = rev.Valid ? string.Format("hg {0} rev {1}, ", rev.Branch, rev.Parent) : "";
    string revtag = rev.Valid ? "-" + rev.Parent : "";
    string buildName = "", desc = "", shortDesc = "";

    if (v.Revision <= 0)
    {
        v = new Version(v.Major, Math.Max(0, v.Minor), Math.Max(0, v.Build), GetDateBuild());
    }
    
    switch (GetBuildType())
    {
    case BuildType.Official:
        buildName = string.IsNullOrEmpty(this.VersionDesignation) ? "Final" : this.VersionDesignation;
        desc = string.Format("{0} {1}", this.VersionNumber, buildName);
        shortDesc = desc;
        break;

    case BuildType.Nightly:
        desc = string.Format("{0} Nightly build ({1}{2:dd MMM yyyy}", this.VersionNumber, revstr, DateTime.Now);
        shortDesc = string.Format("{0}-nightly{1}", this.VersionNumber, revtag);
        break;
    
    default:
        desc = string.Format("{0} Developer build ({1}{2:dd MMM yyyy})", this.VersionNumber, revstr, DateTime.Now);
        shortDesc = string.Format("{0}-dev{1}", v, revtag);
        break;
    }
    
    var parameters = new Dictionary<string, object>() {
        { "v.major", v.Major },
        { "v.minor", v.Minor },
        { "v.build", v.Build },
        { "v.revision", v.Revision },
        { "v", v },
        { "desc", desc },
        { "shortDesc", shortDesc },
        { "year", DateTime.Now.Year },
        { "version", this.VersionNumber },
    };
    
    WriteFile(Path.Combine(this.OutputDirectory, "jwversion.h"),
        Template(parameters,
@"#ifndef __JWVERSION_H__
#define __JWVERSION_H__

#define JW_VERSION_MAJ @v.major@
#define JW_VERSION_MIN @v.minor@
#define JW_VERSION_REV @v.build@
#define JW_VERSION_BLD @v.revision@

#define JW_VERSION_STR ""@v@""
#define JW_VERSION_DESC ""@desc@""
#define JW_COPYRIGHT ""Copyright \xa9 2015-@year@ John J. Jordan, and others (see About...)""

#define JW_SHORTVERSION ""@version@""

#define JW_VERSION_BANNER ""** Joe's Own Editor for Windows v@shortDesc@ ** Copyright \xc2\xa9 @year@ **""
#define VERSION ""@shortDesc@""

#endif // __JWVERSION_H__
"));
            
    WriteFile(Path.Combine(this.OutputDirectory, "jwversion.wxi"),
        Template(parameters,
@"<?xml version=""1.0"" encoding=""utf-8""?>
<Include>
  <?define JoeWinVersion = ""@v@"" ?>
  <?define JoeWinShortVersion = ""@version@"" ?>
  <?define JoeWinDescVersion = ""@shortDesc@"" ?>
</Include>
"));

    return true;
}

private string Template(Dictionary<string, object> parameters, string content)
{
    return parameters.Aggregate(content, (s, p) => s.Replace("@" + p.Key + "@", p.Value.ToString()));
}

private BuildType GetBuildType()
{
    if (Environment.GetEnvironmentVariable("JOEWIN_OFFICIAL") != null)
        return BuildType.Official;
    else if (Environment.GetEnvironmentVariable("JOEWIN_NIGHTLY") != null)
        return BuildType.Nightly;
    else
        return BuildType.Developer;
}

private Revision GetMercurialRev()
{
    var p = Process.Start(new ProcessStartInfo()
    {
        FileName = "hg",
        Arguments = "id -nib",
        UseShellExecute = false,
        RedirectStandardOutput = true,
        RedirectStandardError = true,
    });
    
    var output = p.StandardOutput.ReadToEnd();
    var err = p.StandardError.ReadToEnd();
    p.WaitForExit();
    
    if (p.ExitCode != 0)
    {
        Log.LogWarning("Mercurial terminated with exit code={0}", p.ExitCode);
        Log.LogWarning("stderr follows:\n{0}", err);
        return new Revision() { Valid = false };
    }
    
    var parts = output.Trim().Split();
    if (parts.Length != 3)
    {
        return new Revision() { Valid = false };
    }
    
    return new Revision() { Parent = parts[0], Branch = parts[2], Valid = true };
}

private int GetDateBuild()
{
    var now = DateTime.Now;
    return (10000 * ((now.Year % 10) / 2)) +
           (100 * ((now.Year % 2 == 1 ? 12 : 0) + now.Month)) +
           now.Day;
}

private void WriteFile(string fname, string content)
{
    if (File.Exists(fname))
    {
        if (content == File.ReadAllText(fname, Encoding.ASCII))
        {
            return;
        }
    }
    
    File.WriteAllText(fname, content, Encoding.ASCII);
}
