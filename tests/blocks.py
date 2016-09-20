
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
