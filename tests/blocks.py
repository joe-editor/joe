
import joefx

class BlockMoveTests(joefx.JoeTestBase):
    def test_blkmove_1(self):
        self.startJoe()
        self.write("this is some text")
        self.cmd("bol,markb,nextword,rtarw,markk,rtarw,rtarw,rtarw,blkmove")
        self.assertTextAt("is this some text", x=0)
        self.assertCursor(x=3, y=1)
        self.assertSelectedTextEquals("this ")
        self.exitJoe()
    
    def test_blkmove_inside_block(self):
        self.startJoe()
        self.write("this is some text")
        self.cmd("bol,markb,rtarw,rtarw,rtarw,rtarw,rtarw,markk,ltarw,ltarw,ltarw,blkmove")
        self.assertTextAt("No block", y=-1, x=0)
        self.assertTextAt("this is some text", x=0)
        self.assertCursor(x=2, y=1)
        self.exitJoe()
    
    def test_blkmove_no_block(self):
        self.startJoe()
        self.cmd("blkmove")
        self.assertTextAt("No block", y=-1, x=0)
        # Make sure no change
        self.cmd("abort")
        self.assertExited()

class BlockCopyTests(joefx.JoeTestBase):
    def test_blkcpy_1(self):
        self.startJoe()
        self.write("hello world")
        self.cmd("bol,markb,nextword,markk,blkcpy,blkcpy")
        self.assertTextAt("hellohellohello world", x=0)
        self.assertCursor(x=5, y=1)
        self.assertSelectedTextEquals("hello")
        self.exitJoe()
    
    def test_blkcpy_inside_block(self):
        self.startJoe()
        self.write("hello world")
        self.cmd("bol,markb,nextword,markk,bol,rtarw,rtarw,blkcpy")
        self.assertTextAt("hehellollo world", x=0)
        self.assertCursor(x=2, y=1)
        self.assertSelectedTextEquals("hello")
        self.exitJoe()
    
    def test_blkcpy_no_block(self):
        self.startJoe()
        self.cmd("blkcpy")
        self.assertTextAt("No block", y=-1, x=0)
        # Make sure no changes that hang up exit.
        self.cmd("abort")
        self.assertExited()

class BlockDeleteTests(joefx.JoeTestBase):
    def test_blkdel_1(self):
        self.startJoe()
        self.write("hello world")
        self.cmd("bol,markb,nextword,markk,blkdel")
        self.assertTextAt(" world", x=0)
        self.assertCursor(x=0, y=1)
        self.exitJoe()
    
    def test_blkdel_no_block(self):
        self.startJoe()
        self.write("hello world")
        self.cmd("blkdel")
        self.assertTextAt("No block", x=0, y=-1)
        self.assertTextAt("hello world", x=0)
        self.exitJoe()

class BlockSaveTests(joefx.JoeTestBase):
    def test_blocksave_1(self):
        self.startJoe()
        self.write("hello world")
        self.cmd('bol,markb,nextword,markk,blksave,"outfile",rtn')
        self.assertTextAt("Block written to", x=0, y=-1)
        self.assertSelectedTextEquals("hello")
        self.exitJoe()
        self.assertFileContents("outfile", "hello")

class CommentTests(joefx.JoeTestBase):
    def test_comment_block_python(self):
        self.workdir.fixtureData("foo.py", "one\ntwo\nthree\n")
        self.startup.args = ('foo.py',)
        self.startJoe()

        self.cmd("bol,markb,dnarw,dnarw,markk,comment")
        self.assertTextAt("# one", x=0, y=1)
        self.assertTextAt("# two", x=0, y=2)
        self.assertTextAt("three", x=0, y=3)

        # Toggle: commenting again uncomments
        self.cmd("comment")
        self.assertTextAt("one", x=0, y=1)
        self.assertTextAt("two", x=0, y=2)
        self.assertTextAt("three", x=0, y=3)
        self.exitJoe()

    def test_comment_block_c_uses_slashes(self):
        self.workdir.fixtureData("foo.c", "int a;\nint b;\n")
        self.startup.args = ('foo.c',)
        self.startJoe()

        self.cmd("bol,markb,dnarw,markk,comment")
        self.assertTextAt("// int a;", x=0, y=1)
        self.assertTextAt("int b;", x=0, y=2)

        self.cmd("comment")
        self.assertTextAt("int a;", x=0, y=1)
        self.exitJoe()

    def test_comment_single_line(self):
        self.workdir.fixtureData("foo.py", "one\ntwo\n")
        self.startup.args = ('foo.py',)
        self.startJoe()

        self.cmd("comment")
        self.assertTextAt("# one", x=0, y=1)
        self.assertTextAt("two", x=0, y=2)
        self.exitJoe()

    def test_comment_skips_blank_lines(self):
        self.workdir.fixtureData("foo.py", "one\n\ntwo\n")
        self.startup.args = ('foo.py',)
        self.startJoe()

        self.cmd("bol,markb,dnarw,dnarw,eol,markk,comment")
        self.assertTextAt("# one", x=0, y=1)
        self.assertTextAt("", x=0, y=2, to_eol=True)
        self.assertTextAt("# two", x=0, y=3)

        # Blank lines do not block toggling back off
        self.cmd("comment")
        self.assertTextAt("one", x=0, y=1)
        self.assertTextAt("two", x=0, y=3)
        self.exitJoe()

    def test_comment_prompt_for_prefix(self):
        self.workdir.fixtureData("foo.txt", "hello\n")
        self.startup.args = ('foo.txt',)
        self.startJoe()

        self.cmd("comment")
        self.assertTextAt("Comment prefix", x=0, y=-1)
        self.write("--")
        self.rtn()
        self.assertTextAt("-- hello", x=0, y=1)
        self.exitJoe()
