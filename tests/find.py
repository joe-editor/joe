
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

