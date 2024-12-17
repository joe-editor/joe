import joefx
import time

class EditingTests(joefx.JoeTestBase):
    def test_update_context_with_escapes(self):
        """Regression test for a memory overrun in get_context that caused a segfault"""
        contents = r"""//             |--- key ---|    =    |--- quoted ---|  |-!quot-|
const CCre = /(([a-zA-Z\-]+)\s*(=\s*(("([^"\\]|\\.)*")|([^"\s]+))?))*/;
"""
        self.config.globalopts.keepup = True
        self.workdir.fixtureData("test", contents)
        self.startup.args = "-title", "test"
        self.startJoe()
        self.writectl("{down}{right*16}")
        for c in ("?", ":", "{bs}"):
            self.writectl(c)
            time.sleep(0.1)
        self.save()
        self.exitJoe()
        self.assertExited()

    def test_paste_linum_garbage(self):
        """Regression test for invalid screen output with line numbers"""
        self.config.globalopts.brpaste = True
        self.startJoe()
        self.mode("linums")
        self.writectl("{enter*6}")
        self.cmd("bof")
        self.write("\033[200~Hello\rWorld\r!\033[201~")

        # Wait for it to write
        self.assertTextAt("!", dx=-1)
        self.assertTextAt("Hello", dx=-1, y=1)
        self.assertTextAt("World", dx=-1, y=2)
        
        # All remaining lines should be blank
        for y in range(4, 25):
            self.assertTextAt("      ", dx=-1, y=y)

    def test_paste_linum_garbage_2(self):
        """Regression test for invalid screen output with line numbers"""
        self.config.globalopts.brpaste = True
        self.startJoe()
        self.mode("linums")
        self.writectl("{enter*5}")
        self.cmd("bof")

        for i in range(4):
            self.write("\033[200~1\r2\r3\r4\r5\r\033[201~")
            for j in range(1, 6):
                for k in range(i + 1):
                    self.assertTextAt("{}".format(j), y=(k * 5 + j))
            for j in range(5 * i + 6, 25):
                if j < i + 5:
                    self.assertTextAt("{:2d}".format(j), dx=-3, y=j)
                self.assertTextAt("   ", dx=-1, y=j)

    def test_linums_shift(self):
        """Ensure that line numbers shift when a new column is added"""
        self.config.globalopts.brpaste = True
        self.startJoe()
        self.mode("linums")
        txt = "\033[200~    ABCD{enter}    EFGH{enter}    IJKL{enter}    MNOP{enter}\033[201~"

        self.writectl(txt)
        self.assertTextAt("MNOP", x=7, y=4)
        self.assertTextAt("ABCD", x=7, y=1)
        self.writectl(txt)
        self.assertTextAt("MNOP", x=7, y=8)
        self.assertTextAt("ABCD", x=7, y=5)
        self.writectl(txt)
        self.assertTextAt("MNOP", x=8, y=12)
        self.assertTextAt("ABCD", x=8, y=1)
        self.assertTextAt("ABCD", x=8, y=5)
