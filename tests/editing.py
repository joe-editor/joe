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
        
        # Next two lines should be blank
        self.assertTextAt("      ", dx=-1, dy=1)
        self.assertTextAt("      ", dx=-1, dy=2)
