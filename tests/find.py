
import joefx

class FindTests(joefx.JoeTestBase):
    def replace_in_rectangle(self, data, left, down, right, expected):
        self.workdir.fixtureData("test", data)
        self.startup.args = ("test",)
        self.startJoe()
        self.mode("square")
        self.cmd("bof")
        self.writectl("{right*%d}" % left)
        self.cmd("markb")
        self.writectl("{down*%d}{right*%d}" % (down, right - left))
        self.cmd("markk")
        self.cmd("tomarkb")
        self.replace("i", "I", "k")
        self.answerReplace("y")
        self.write("!")
        self.writectl("^C")
        self.save()
        self.exitJoe()
        self.assertFileContents("test", expected)

    def test_debug_rectangle_issue(self):
        """Regression test for rectangle search column cache across rows."""
        self.workdir.fixtureData("test", "row 1: i inside i outside\nrow 2: i another i outside\nrow 3: i third i outside\nrow 4: i final i outside\nrow 5: i outside-only\n")
        self.startup.args = ("test",)
        self.startJoe()
        self.mode("square")
        self.cmd("bof")
        self.writectl("{right*7}")
        self.cmd("markb")
        self.writectl("{down*3}{right*8}")
        self.cmd("markk")
        self.cmd("tomarkb")
        self.replace("i", "I", "k")
        self.answerReplace("y")
        self.write("!")
        self.writectl("^C")
        self.save()
        self.exitJoe()
        self.assertFileContents("test", "row 1: I InsIde i outside\nrow 2: I another i outside\nrow 3: I thIrd i outside\nrow 4: I fInal i outside\nrow 5: i outside-only\n")

    def test_rectangle_replace_multiple_adjacent_matches(self):
        """Restricted replacement handles adjacent matches and excluded columns."""
        self.replace_in_rectangle(
            "row 1: i  ii  i OUTSIDE i\n"
            "row 2: i  i   ii OUTSIDE i\n"
            "row 3: ii i   i OUTSIDE i\n"
            "row 4: i  ii  ii OUTSIDE i\n"
            "row 5: i  i   i OUTSIDE i\n"
            "row 6: i  ii  i OUTSIDE i\n",
            7, 4, 23,
            "row 1: I  II  I OUTSIDE i\n"
            "row 2: I  I   II OUTSIDE i\n"
            "row 3: II I   I OUTSIDE i\n"
            "row 4: I  II  II OUTSIDE i\n"
            "row 5: I  I   I OUTSIDE i\n"
            "row 6: i  ii  i OUTSIDE i\n")

    def test_rectangle_replace_nonzero_start_column(self):
        """Restricted replacement honors a rectangle that starts mid-field."""
        self.replace_in_rectangle(
            "row 1: i inside i outside\n"
            "row 2: i inside i outside\n"
            "row 3: i inside i outside\n"
            "row 4: i inside i outside\n"
            "row 5: i inside i outside\n",
            9, 3, 16,
            "row 1: i InsIde i outside\n"
            "row 2: i InsIde i outside\n"
            "row 3: i InsIde i outside\n"
            "row 4: i InsIde i outside\n"
            "row 5: i inside i outside\n")

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
    
    def test_find_with_escapes(self):
        """Tests escape sequences in find input"""
        self.workdir.fixtureData("test", "first\nsecond\nthird\n")
        self.startup.args = "test",
        self.startJoe()
        
        self.replace(r"\n", "-")
        self.answerReplace("r")
        
        self.save()
        self.exitJoe()
        self.assertFileContents("test", "first-second-third-")
    
    def test_replace_with_escapes(self):
        """Test replacing text with escape sequences"""
        self.workdir.fixtureData("test", "first-second-third\nfirst+second+third\nfirst=second=third\n")
        self.startup.args = ("test",)
        self.startJoe()
        
        self.replace("-", r"\n")
        self.answerReplace("r")
        
        self.replace("+", r"\t")
        self.answerReplace("yy")
        
        self.replace("=", r"\x7f")
        self.answerReplace("yy")
        
        self.save()
        self.exitJoe()
        self.assertFileContents("test", "first\nsecond\nthird\nfirst\tsecond\tthird\nfirst\x7fsecond\x7fthird\n")

class ISearchTests(joefx.JoeTestBase):
    def test_isearch_fwd(self):
        """Tests incremental search going forward"""
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
            self.assertTextAt("I-find: " + sstr, x=0, y=-1, to_eol=True)
            self.assertCursor(x=i + 1, y=i + 1)
        
        # Now backspace out
        for i in range(N - 2, -1, -1):
            sstr = sstr[:-1]
            self.writectl("{bs}")
            self.assertTextAt("I-find: " + sstr, x=0, y=-1, to_eol=True)
            self.assertCursor(x=i + 1, y=i + 1)
        
        self.writectl("^C")
        self.exitJoe()
    
    def test_isearch_bkwd(self):
        """Tests reverse incremental search"""
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
            self.assertTextAt("I-find: " + sstr, x=0, y=-1, to_eol=True)
            self.assertCursor(x=0, y=N - i + 1)
        
        # Now backspace out
        for i in range(N - 2, -1, -1):
            sstr = sstr[:-1]
            self.writectl("{bs}")
            self.assertTextAt("I-find: " + sstr, x=0, y=-1, to_eol=True)
            self.assertCursor(x=0, y=N - i + 1)
        
        self.writectl("^C")
        self.exitJoe()
    
    def test_isearch_resume(self):
        """Tests canceling and resuming incremental search"""
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
            self.assertTextAt("I-find: " + sstr, x=0, y=-1, to_eol=True)
            self.assertCursor(x=i + 1, y=i + 1)
        
        # Cancel
        self.writectl("^C")
        self.assertTextAt("", y=-1, x=0, to_eol=True)
        
        # Can't invoke command because it may not resume, need to use shortcut
        self.write(self.findCmd("isrch", fail=True))
        
        # This re-invocation should find the next one.
        self.assertCursor(x=X, y=X + 1)
        
        # Finish up
        for i in range(X, N):
            ch = chr(ord('a') + i)
            self.write(ch)
            sstr += ch
            self.assertTextAt("I-find: " + sstr, x=0, y=-1, to_eol=True)
            self.assertCursor(x=i + 1, y=i + 1)
        
        self.writectl("^C")
        self.exitJoe()
