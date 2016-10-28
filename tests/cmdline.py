
import joefx

class ArgumentTests(joefx.JoeTestBase):
    def test_dash_dash(self):
        self.workdir.fixtureData("-foo", "This is -foo")
        self.startup.args = ('--', '-foo')
        self.startJoe()
        
        self.assertTextAt("This is -foo")
