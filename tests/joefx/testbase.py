
import os
import unittest
import time
import re

from . import controller
from . import fixtures
from . import rcfile
from . import utils
from . import keys

RCFILES = {}
FIXTURESDIR = os.path.join(os.path.dirname(__file__), "../fixtures")
RCDIR = os.path.join(os.path.dirname(__file__), "../../rc")

class JoeTestBase(unittest.TestCase):
    """Base class for joe tests.  Provides shortcuts for common editor interactions."""

    #
    # Test setup/teardown
    #
    
    def __init__(self, *a, **ka):
        super().__init__(*a, **ka)
        self.joe = None
        
        self.workdir = fixtures.FixtureDir(controller.tmpfiles.workdir, FIXTURESDIR)
        self.homedir = fixtures.FixtureDir(controller.tmpfiles.homedir, FIXTURESDIR)
        self.startup = controller.StartupArgs()
        self.timeout = 1 # Second
        self.config = None
    
    def setUp(self):
        super().setUp()
        if self.config is None:
            self.loadConfig('joerc')
    
    def tearDown(self):
        if self.joe is not None:
            ex = self.joe.getExitCode()
            if ex is None:
                self.joe.kill()
            elif ex < 0:
                self.assertTrue(False, "JOE exited with signal %d" % -ex)
            
            self.joe.flushin()
            self.joe.close()
        
        super(JoeTestBase, self).tearDown()
    
    def run(self, result=None):
        if result:
            orig_err_fail = result.errors + result.failures
        super(JoeTestBase, self).run(result)
        if result and len(result.errors + result.failures) > len(orig_err_fail) and self.joe is not None:
            print('\n----- screen at exit -----\n%s\n-----\n' % self.joe.prettyScreen())
    
    def exitJoe(self):
        self.cmd("killjoe")
        self.assertExited()
    
    #
    # JOE startup
    #
    
    def startJoe(self, waitbanner=True):
        """Starts the editor"""
        self.workdir.setup()
        self.homedir.setup()
        self.joe = controller.startJoe("../joe/joe", self.startup)
        if waitbanner:
            banner = self.config.globalopts.xmsg
            banner = re.sub(r'\\.', '', banner[0:banner.index('%')])
            self.assertTextAt(banner, x=0, y=self.joe.size.Y - 1)
    
    def loadConfig(self, cfgfile, asfile='.joerc'):
        if cfgfile not in RCFILES:
            for path in (FIXTURESDIR, RCDIR):
                fname = os.path.join(path, cfgfile)
                if os.path.exists(fname):
                    RCFILES[cfgfile] = rcfile.parse(fname)
                    break
        
        if cfgfile in RCFILES:
            self.config = RCFILES[cfgfile].clone()
            self.homedir.fixtureFunc(asfile, lambda: self.config.serialize())
        else:
            self.fail("Could not find config file: " + cfgfile)
    
    
    #
    # Assertions
    #
    
    def _posFromInput(self, x=None, y=None, dx=0, dy=0):
        if y is None:
            y = self.joe.cursor.Y
        elif y < 0:
            y += self.joe.size.Y
        
        if x is None:
            x = self.joe.cursor.X
        elif x < 0:
            x += self.joe.size.X
        
        return controller.coord(dx + x, dy + y)
    
    def assertCursor(self, x=None, y=None):
        """Asserts cursor is at the specified position"""
        self.joe.expect(lambda: self._posFromInput(x, y) == self.joe.cursor)
        pos = self._posFromInput(x, y)
        self.assertEqual(pos.X, self.joe.cursor.X, "Cursor X pos")
        self.assertEqual(pos.Y, self.joe.cursor.Y, "Cursor Y pos")
    
    def assertTextAt(self, text, x=None, y=None, dx=0, dy=0, to_eol=False):
        """Asserts text at specified coordinate matches the input. If either coordinate is omitted, use the cursor's coordinate"""
        def check():
            pos = self._posFromInput(x, y, dx, dy)
            result = self.joe.checkText(pos.Y, pos.X, text)
            if to_eol and result:
                st = pos.X + len(text)
                result = self.joe.readLine(pos.Y, st, self.joe.size.X - st).strip() == ''
            return result
        
        if not self.joe.expect(check):
            pos = self._posFromInput(x, y, dx, dy)
            s = self.joe.readLine(pos.Y, pos.X, len(text))
            self.assertEqual(s, text, "Text at line=%d col=%d" % (pos.Y, pos.X))
            if to_eol:
                st = pos.X + len(text)
                self.assertEqual(self.joe.readLine(pos.Y, st, self.joe.size.X - st), '', "End of line %d" % pos.Y)
    
    def assertExited(self):
        """Asserts that the editor has exited"""
        result = self.joe.wait()
        self.assertIsNot(result, None, "Process was expected to have exited")
        self.assertEqual(result, 0, "Process exited with non-zero code")
    
    def assertFileContents(self, file, expected):
        """Asserts that file in workdir matches expected contents"""
        with open(os.path.join(controller.tmpfiles.workdir, file), 'rb') as f:
            if isinstance(expected, str):
                expected = expected.encode('utf-8')
            self.assertEqual(f.read(), expected)
    
    def assertSelectedText(self, f, x=None, y=None, dx=0, dy=0):
        def check():
            pos = self._posFromInput(x, y, dx, dy)
            text = self._readSelected(pos.X, pos.Y)
            return f(text)
        self.assertTrue(self.joe.expect(check))
    
    def assertSelectedMenuItem(self, label):
        return self.assertSelectedText(lambda x: utils.compareMenuLabel(label, x))
    
    def assertSelectedTextEquals(self, txt, **args):
        self.assertSelectedText(lambda t: t == txt, **args)
    
    #
    # Editor functional helpers
    #
    
    def write(self, text):
        """Write specified text to editor"""
        return self.joe.write(text)
    
    def writectl(self, text):
        """Write control code to editor"""
        return self.joe.writectl(text)
    
    def rtn(self):
        """Press the ENTER key"""
        self.writectl("{enter}")
    
    def replace(self, findstr, replace, params=''):
        """Initiates find/replace with the specified options"""
        self.find(findstr, params + 'r')
        self.assertTextAt("Replace with", x=0)
        self.write(replace)
        self.rtn()
    
    def find(self, findstr, params=''):
        """Initiates find with the specified options"""
        self.cmd("ffirst")
        self.assertTextAt("Find (^", x=0)
        self.findrow = self.joe.cursor.Y
        self.write(findstr)
        self.rtn()
        self.assertTextAt("(I)gnore (R)eplace", x=0)
        self.writectl(params + "{enter}")
    
    def answerReplace(self, responses):
        """Sends keystrokes as respones to ignore/replace prompt"""
        self.joe.flushin()
        cursor = None
        for response in responses:
            self.assertTrue(self.joe.expect(lambda: self.joe.cursor != cursor and self.joe.checkText(self.findrow, 0, "Replace (Y)es (N)o")))
            cursor = self.joe.cursor
            self.write(response)
    
    def cmd(self, s, scope='main'):
        """Executes a command or macro"""
        if ',' not in s:
            k = self.findCmd(s, scope)
            if k is not None:
                self.write(k)
                return
        
        execmdk = self.findCmd("execmd", scope)
        if execmdk is not None:
            self.write(execmdk)
        else:
            # Just default to meta-x
            self.writectl("^[X")
        
        self.assertTextAt("Command:", x=0)
        self.write(s)
        self.rtn()
    
    def mode(self, mode):
        """Sets a mode (option menu)"""
        self.cmd("mode")
        self.assertTextAt("Option:", x=0)
        self.write(mode)
        self.rtn()
    
    def menu(self, menuname):
        """Brings up menuname menu"""
        self.cmd("menu")
        self.assertTextAt("Menu:", x=0)
        self.write(menuname)
        self.rtn()
        
        # Wait for cursor to be on inverse (selected) text
        self.assertTrue(self.joe.expect(lambda: len(self._readSelected(self.joe.cursor.X, self.joe.cursor.Y)) > 0))
    
    def encoding(self, encoding):
        """Changes the encoding of the current file to specified encoding"""
        self.mode("encoding")
        self.assertTextAt("Select file character", x=0)
        self.write(encoding)
        self.rtn()
    
    def save(self, filename=None):
        """Saves the current file as filename, or current name if not specified"""
        self.cmd("save")
        self.assertTextAt("Name of file to save", x=0)
        if filename is not None:
            self.cmd("dellin")
            self.write(filename)
        self.rtn()
    
    def selectMenu(self, label, menuname='root'):
        """Selects a menu that matches the label (as specified in joerc) for that menu's text"""
        
        # If we're already on the item, then return it.
        cursor = self.joe.cursor
        if utils.compareMenuLabel(label, self._readSelected(cursor.X, cursor.Y)):
            return True
        
        # Jump to first menu item.
        self.cmd("bofmenu", scope="menu")
        menu = self.config.getMenu(menuname)
        self.joe.expect(lambda: utils.compareMenuLabel(menu.items[0].label, self._readSelected(self.joe.cursor.X, self.joe.cursor.Y)))
        menutop = self.joe.cursor
        
        # Move right until we find the item.
        while True:
            self.joe.flushin()
            seltext = self._readSelected(self.joe.cursor.X, self.joe.cursor.Y)
            if utils.compareMenuLabel(label, seltext):
                return True
            self.writectl("{right}")
            if not self.joe.expect(lambda: self.joe.cursor.Y >= menutop.Y and seltext != self._readSelected(self.joe.cursor.X, self.joe.cursor.Y)):
                self.assertTrue(False, "Could not find menu")
    
    def _readSelected(self, x, y):
        return self._readTextWithProperty(x, y, lambda c: c.reverse)
    
    def _readTextWithProperty(self, x, y, predicate):
        """Read all text in reverse around specified coordinate (cursor coordinates used in case one/both missing)"""
        start = end = x
        for i in range(max(0, x - 1), -1, -1):
            if predicate(self.joe.term.buffer[y][i]):
                start = i
            else:
                break
        for i in range(x, self.joe.size.X):
            if predicate(self.joe.term.buffer[y][i]):
                end = i + 1
            else:
                break
        result = self.joe.readLine(y, start, end - start)
        return result
    
    def findCmd(self, cmd, scope="main", fail=False):
        """Finds keybinding from configuration for 'cmd' in the specified kbd scope"""
        while scope is not None:
            bindings = self.config.getKeyBindings(scope)
            if bindings is None: break
            for b in bindings.bindings:
                if b.macro == cmd:
                    k = keys.fromRcFile(b.keys)
                    # Avoid things like mouse bindings or stuff we can't
                    # reproduce
                    if k is not None:
                        return k
            
            scope = bindings.inherits
        
        if fail:
            self.fail("Couldn't find keybindings for command %s" % cmd)
        
        return None

    def waitForNotEmpty(self, y, x=0, length=None):
        """Waits for any text to exist at the specified screen position"""
        if length is None:
            length = self.joe.size.X - x
        return self.joe.expect(lambda: len(self.joe.readLine(line=y, col=x, length=length).strip()) > 0)
