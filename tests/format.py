
import joefx

class FormatTests(joefx.JoeTestBase):
    def test_wordwrap_with_asterisks(self):
        """Regression test for #265: Version 3.7 treats "-" and "*" as quote characters"""
        self.maxDiff = None # Show whole string on failure
        self.config.globalopts.wordwrap = True
        self.config.globalopts.autoindent = False
        self.config.globalopts.pastehack = False
        self.startJoe()
        
        self.write("*all* work and no play makes jack a dull boy. all work and no play makes jack a dull boy. all work and no play makes jack a dull boy. all work and no play makes jack a dull boy.")
        self.save("testout")
        self.cmd("abort")
        self.assertExited()
        self.assertFileContents("testout", "*all* work and no play makes jack a dull boy. all work and no play makes\njack a dull boy. all work and no play makes jack a dull boy. all work and no\nplay makes jack a dull boy.")
