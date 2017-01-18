
import joefx

class FindTests(joefx.JoeTestBase):
    def test_abracadabra(self):
        """Regression test for #295: infinite loop on search-and-replace"""
        self.startJoe()
        self.write("abracadabra")
        self.cmd("bol")
        self.replace("a", "X")
        self.answerReplace("yyy")
        self.writectl("^C")
        self.joe.writectl("{left*2}{bs}a")
        self.cmd("bol,fnext")
        self.answerReplace("yy")
        self.writectl("^C")
        self.assertTextAt("XbrXcXdXbra", x=0)
    
    def test_replace_zerolen_regex(self):
        """Regression test related to #295.  Check original functionality that broke"""
        self.workdir.fixtureData("test", "line 1\nline 2\nline 3\nline 4\n")
        self.startup.args = ("test",)
        self.startJoe()
        self.replace(r"\$", "")
        self.assertCursor(y=1)
        self.write("y")
        self.assertCursor(y=2)
        self.write("y")
        self.assertCursor(y=3)
        self.write("y")
        self.assertCursor(y=4)
        self.write("y")
        self.assertCursor(x=0, y=5)
    
    def test_replace_skip_zerolen_regex(self):
        """Regression test related to #295.  Check original functionality that broke"""
        self.workdir.fixtureData("test", "line 1\nline 2\nline 3\nline 4\n")
        self.startup.args = ("test",)
        self.startJoe()
        
        self.replace(r"\$", "")
        self.assertCursor(y=1)
        self.write("n")
        self.assertCursor(y=2)
        self.write("n")
        self.assertCursor(y=3)
        self.write("n")
        self.assertCursor(y=4)
        self.write("n")
        self.assertCursor(x=0, y=5)


class ISearchTests(joefx.JoeTestBase):
    def test_isearch_fwd(self):
        N = 10
        tstdata = '\n'.join(''.join(chr(ord('a') + t) for t in range(i)) for i in range(1, N + 1))
        self.workdir.fixtureData("test", tstdata)
        self.startup.args = ("test",)
        self.startJoe()
        
        self.cmd("isrch")
        self.assertTextAt("I-find:", x=0, y=-1)
        sstr = ''
        for i in range(N):
            ch = chr(ord('a') + i)
            self.write(ch)
            sstr += ch
            self.assertTextAt("I-find: " + sstr, x=0, y=-1)
            self.assertCursor(x=i + 1, y=i + 1)
        
        # Now backspace out
        for i in range(N - 2, -1, -1):
            sstr = sstr[:-1]
            self.writectl("{bs}")
            self.assertTextAt("I-find: " + sstr, x=0, y=-1)
            self.assertCursor(x=i + 1, y=i + 1)
        
        self.writectl("^C")
        self.exitJoe()
    
    def test_isearch_bkwd(self):
        N = 10
        tstdata = '\n'.join(''.join(chr(ord('a') + t) for t in range(i)) for i in range(N + 1, 0, -1))
        self.workdir.fixtureData("test", tstdata)
        self.startup.args = ("test",)
        self.startJoe()
        
        self.cmd("eof")
        self.cmd("rsrch")
        self.assertTextAt("I-find:", x=0, y=-1)
        sstr = ''
        for i in range(N):
            ch = chr(ord('a') + i)
            sstr += ch
            self.write(ch)
            self.assertTextAt("I-find: " + sstr, x=0, y=-1)
            self.assertCursor(x=0, y=N - i + 1)
        
        # Now backspace out
        for i in range(N - 2, -1, -1):
            sstr = sstr[:-1]
            self.writectl("{bs}")
            self.assertTextAt("I-find: " + sstr, x=0, y=-1)
            self.assertCursor(x=0, y=N - i + 1)
        
        self.writectl("^C")
        self.exitJoe()
    
    def test_isearch_resume(self):
        N = 10
        X = 5
        tstdata = '\n'.join(''.join(chr(ord('a') + t) for t in range(i)) for i in range(1, N + 1))
        self.workdir.fixtureData("test", tstdata)
        self.startup.args = ("test",)
        self.startJoe()
        
        self.cmd("isrch")
        self.assertTextAt("I-find:", x=0, y=-1)
        sstr = ''
        for i in range(X):
            ch = chr(ord('a') + i)
            self.write(ch)
            sstr += ch
            self.assertTextAt("I-find: " + sstr, x=0, y=-1)
            self.assertCursor(x=i + 1, y=i + 1)
        
        # Cancel
        self.writectl("^C")
        
        # Can't invoke command because it will not resume, need to use shortcut
        # TODO: Lookup shortcut
        self.writectl("^[s")
        
        # This re-invocation should find the next one.
        self.assertCursor(x=X, y=X + 1)
        
        # Finish up
        for i in range(X, N):
            ch = chr(ord('a') + i)
            self.write(ch)
            sstr += ch
            self.assertTextAt("I-find: " + sstr, x=0, y=-1)
            self.assertCursor(x=i + 1, y=i + 1)
        
        self.writectl("^C")
        self.exitJoe()
