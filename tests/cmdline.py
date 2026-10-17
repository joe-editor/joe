
import joefx

class ArgumentTests(joefx.JoeTestBase):
    def test_dash_dash(self):
        self.workdir.fixtureData("-foo", "This is -foo")
        self.startup.args = ('--', '-foo')
        self.startJoe()
        
        self.assertTextAt("This is -foo")

    def test_file_colon_line(self):
        self.workdir.fixtureData("foo.txt", "one\ntwo\nthree\nfour\n")
        self.startup.args = ('foo.txt:3',)
        self.startJoe()

        self.assertTextAt("three", x=0)
        self.assertTextAt("two", x=0, dy=-1)
        self.assertTextAt("four", x=0, dy=1)

    def test_file_colon_line_col(self):
        self.workdir.fixtureData("foo.txt", "abcdef\nghijkl\n")
        self.startup.args = ('foo.txt:2:3',)
        self.startJoe()

        self.assertTextAt("ghijkl", x=0)
        self.assertTextAt("ijkl")

    def test_file_colon_line_plus_overrides(self):
        self.workdir.fixtureData("foo.txt", "one\ntwo\nthree\nfour\n")
        self.startup.args = ('+2', 'foo.txt:4')
        self.startJoe()

        self.assertTextAt("two", x=0)
        self.assertTextAt("one", x=0, dy=-1)

    def test_file_colon_dashdash_literal(self):
        self.workdir.fixtureData("bar:2", "literal\nsecond\n")
        self.startup.args = ('--', 'bar:2')
        self.startJoe()

        self.assertTextAt("literal", x=0)

    def test_numeric_filename_literal(self):
        self.workdir.fixtureData("123", "numeric\n")
        self.startup.args = ('123',)
        self.startJoe()

        self.assertTextAt("numeric", x=0)

    def test_file_colon_line_leading_zero(self):
        # Line numbers are decimal: :03 means line 3, not octal
        self.workdir.fixtureData("foo.txt", "one\ntwo\nthree\nfour\n")
        self.startup.args = ('foo.txt:03',)
        self.startJoe()

        self.assertTextAt("three", x=0)
