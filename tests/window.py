
import joefx

class WindowTests(joefx.JoeTestBase):
    def test_showlog_querysave(self):
        """Regression test for #326: joe locks up if you try querysave on the startup log"""
        self.startJoe()
        
        self.cmd("showlog,nextw,abort,querysave")
        self.joe.flushin()
        self.cmd("abort")
        self.assertExited()
    
    def test_resize_menu_narrower_doesnt_crash(self):
        """Regression test for #310: Segfault when resizing with ^T menu open"""
        self.startup.lines = 25
        self.startup.columns = 120
        self.startJoe()
        
        # Pull up menu and resize down to 40
        self.menu("root")
        
        for i in range(119, 39, -1):
            self.joe.resize(i, 25)
            self.joe.flushin()
        
        self.writectl("^C")
        self.cmd("abort")
        self.assertExited()
    
    def test_resize_menu_wider_doesnt_crash(self):
        """Regression test for #310: Segfault when resizing with ^T menu open"""
        self.startup.lines = 25
        self.startup.columns = 80
        self.startJoe()
        
        self.joe.resize(40, 25)
        
        # Pull up menu and resize up to 120
        self.menu("root")
        
        for i in range(40, 121):
            self.joe.resize(i, 25)
            self.joe.flushin()
        
        self.writectl("^C")
        self.cmd("abort")
        self.assertExited()
