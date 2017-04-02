
import joefx
import time
import cProfile

class AbortTests(joefx.JoeTestBase):
    def test_abort_to_exit(self):
        self.startJoe()
        self.cmd("abort")
        self.assertExited()
    
    def test_abort_closes_buffer(self):
        self.startJoe()
        
        # Write text, split the window
        self.cmd("splitw")
        self.assertCursor(x=0, y=13)
        
        # Abort
        self.cmd("abort")
        self.assertCursor(x=0, y=1)
        
        # Abort to exit
        self.cmd("abort")
        self.assertExited()
    
    def test_abort_closes_prompt(self):
        self.startJoe()
        self.cmd("edit")
        self.assertTextAt("Name of file", x=0)
        y = self.joe.cursor.Y
        self.cmd("abort")
        self.assertTextAt("          ", x=0, y=y)
        self.cmd("abort")
        self.assertExited()
    
    def test_abort_prompts_on_change(self):
        self.startJoe()
        self.write("Hello world")
        self.cmd("abort")
        self.assertTextAt("Lose changes to this", x=0)
        self.write("y")
        self.assertExited()
    
    def test_abort_kills_process(self):
        self.startJoe()
        self.cmd("bknd")
        time.sleep(0.1) # TODO: Weird timing issues with subprocesses...
        self.cmd("bol")
        self.assertCursor(y=1, x=0)
        
        self.cmd("abort")
        self.assertTextAt("Kill program", x=0)
        self.write("y")
        time.sleep(0.1) # Same
        
        self.cmd("abort")
        self.assertTextAt("Lose changes", x=0)
        self.write("y")
        self.assertExited()

# TODO: abortbuf

class ArgTests(joefx.JoeTestBase):
    def test_arg(self):
        self.startJoe()
        self.cmd("arg")
        self.assertTextAt("No. times", x=0)
        self.write("5\r")
        self.write("f")
        self.assertTextAt("fffff", x=0)
    
    def test_uarg(self):
        self.startJoe()
        self.cmd("uarg")
        self.assertTextAt("Repeat ", y=-1)
        self.write("5")
        self.assertTextAt("Repeat 5", y=-1)
        self.write("\r")
        self.write("f")
        self.assertTextAt("fffff", x=0)

# TODO: ask

class BacksTests(joefx.JoeTestBase):
    def test_backs_at_bol(self):
        self.startJoe()
        self.write("first line\rsecond line")
        self.cmd("bol")
        self.cmd("backs")
        self.assertTextAt("first linesecond line", x=0)
        self.assertTextAt("second line")
    
    def test_backs_in_menu(self):
        self.startJoe()
        
        # Get expected menu text
        rootMenu = self.config.getMenu('root')
        moreItem = rootMenu.getItem(lambda item: 'more-options' in item.macro)
        moreMenu = self.config.getMenu('more-options')
        
        self.menu("root")
        self.selectMenu(moreItem.label)
        self.rtn()
        self.assertSelectedMenuItem(moreMenu.items[0].label)
        self.writectl("{bs}")
        self.selectMenu(moreItem.label)
        

class BackwTests(joefx.JoeTestBase):
    def test_backw_words(self):
        tests = [
            # Line, start pos, result
            ("hello", -1, ""),
            ("hello world", -1, "hello "),
            ("another", 2, "other"),
            ("whitespace           ", -1, "whitespace")
        ]
        
        self.workdir.fixtureData("test", "\n".join(t[0] for t in tests))
        self.startup.args = ("test",)
        self.startJoe()
        self.cmd("bof")
        self.assertCursor(x=0, y=1)
        
        for line, start, result in tests:
            if start < 0:
                self.cmd("eol")
            else:
                self.writectl("{right*%d}" % start)
            self.cmd("backw,dnarw,bol")
        
        self.save()
        self.exitJoe()
        self.assertFileContents("test", "\n".join(t[2] for t in tests))
    
    def test_backw_newlines(self):
        self.workdir.fixtureData("test", "foo\n\n\n  \n\n\nbar")
        self.startup.args = ("test",)
        self.startJoe()
        self.cmd("eof")
        self.writectl("{left*3}")
        self.cmd("backw")
        self.save()
        self.exitJoe()
        self.assertFileContents("test", "foobar")

# TODO: beep

class BeginMarkingTests(joefx.JoeTestBase):
    def test_begin_marking_right(self):
        self.startJoe()
        self.write("hello world")
        self.cmd("bol")
        for i in range(5):
            self.cmd("rtarw,ltarw,begin_marking,rtarw,toggle_marking")
        self.assertSelectedText(lambda s: s == 'hello', dx=-1)
        self.exitJoe()
    
    def test_begin_marking_left(self):
        self.startJoe()
        self.write("hello world")
        self.cmd("eol")
        for i in range(5):
            self.cmd("ltarw,rtarw,begin_marking,ltarw,toggle_marking")
        self.assertSelectedText(lambda s: s == 'world', dx=1)
        self.exitJoe()
    
    def test_begin_marking_back_forth(self):
        self.startJoe()
        self.write("hello world")
        self.cmd("bol")
        for i in range(5):
            self.cmd("rtarw,ltarw,begin_marking,rtarw,toggle_marking")
        for i in range(3):
            self.cmd("ltarw,rtarw,begin_marking,ltarw,toggle_marking")
        self.assertSelectedText(lambda s: s == 'he', dx=-1)
        self.exitJoe()

class BkndTests(joefx.JoeTestBase):
    def test_bknd_shell(self):
        self.startJoe()
        self.cmd("bknd")
        time.sleep(0.1)
        self.write("echo hello world\r")
        self.assertTextAt("hello world", x=0, dy=-1)
        self.cmd("uparw,abort")
        self.write("y")
        time.sleep(0.1)
        self.exitJoe()

class BkwdcTests(joefx.JoeTestBase):
    def test_bkwdc(self):
        self.startJoe()
        self.write('hello world')
        self.cmd('bkwdc')
        self.assertTextAt("Backward to char", x=0)
        self.write('w')
        self.assertCursor(x=6, y=1)
        self.exitJoe()

# blk* commands in blocks.py

class BofTests(joefx.JoeTestBase):
    def test_bof_tw(self):
        text = "\n".join(("line %d" % i) for i in range(30))
        self.workdir.fixtureData("test", text)
        self.startup.args = ("test",)
        self.startJoe()
        
        self.cmd("dnarw,dnarw,dnarw,dnarw")
        self.assertCursor(x=0, y=5)
        self.cmd("bof")
        self.assertCursor(x=0, y=1)
        self.assertTextAt("line 0  ", x=0)
        
        self.cmd(",".join("dnarw" for i in range(30)))
        self.assertCursor(x=0, y=-1)
        self.assertTextAt("line 29")
        
        self.cmd("bof")
        self.assertCursor(x=0, y=1)
        self.assertTextAt("line 0  ", x=0)
        self.exitJoe()
    
    def test_bof_pw(self):
        self.startJoe()
        
        # Fill up find history buffer
        self.find("first")
        self.find("second")
        self.find("third")
        self.find("fourth")
        self.find("fifth")
        
        self.cmd("ffirst,bof")
        self.assertTextAt("first")
        self.assertCursor(y=-1)
        self.exitJoe()
    
    def test_bof_menu(self):
        self.startJoe()
        
        rootMenu = self.config.getMenu('root')
        self.menu("root")
        self.assertSelectedMenuItem(rootMenu.items[0].label)
        self.writectl("{down*3}")
        self.assertSelectedMenuItem(rootMenu.items[3].label)
        self.writectl("^KU")
        self.assertSelectedMenuItem(rootMenu.items[0].label)
        self.writectl("^C")
        self.exitJoe()

class BolTests(joefx.JoeTestBase):
    def test_bol_tw(self):
        text = "\n".join(("line %d" % i) for i in range(5))
        self.workdir.fixtureData("test", text)
        self.startup.args = ("test",)
        self.startJoe()
        
        for i in range(5):
            self.writectl("{right*%d}" % i)
            self.assertCursor(x=i, y=i+1)
            self.cmd("bol")
            self.assertCursor(x=0, y=i+1)
            self.writectl("{down}")
        self.exitJoe()
    
    def test_bol_pw(self):
        self.startJoe()
        
        # Get cursor position at empty prompt
        self.cmd("ffirst")
        self.assertTextAt("Find", x=0)
        pos = self.joe.cursor
        
        # Write text and wait for it to show back up
        self.write("find text")
        self.assertTextAt("find text", x=pos.X)
        
        # Test bol returns to earlier position
        self.cmd("bol")
        self.assertCursor(x=pos.X, y=pos.Y)
        self.exitJoe()
    
    def test_bol_menu(self):
        menu = joefx.rcfile.Menu()
        menu.name = 'testmenu'
        for i in range(15):
            item = joefx.rcfile.MenuItem()
            item.macro = 'rtn'
            item.label = "Item %d" % i
            menu.items.append(item)
        self.config.menus.append(menu)
        self.config.globalopts.transpose = False
        self.startJoe()
        
        self.menu("testmenu")
        for i in range(0, 9):
            self.writectl("{right*%d}" % i)
            self.assertSelectedMenuItem("Item %d" % i)
            self.writectl("^A")
            self.assertSelectedMenuItem("Item 0")
        self.writectl("{down}")
        for i in range(9, 15):
            self.writectl("{right*%d}" % (i - 9))
            self.assertSelectedMenuItem("Item %d" % i)
            self.writectl("^A")
            self.assertSelectedMenuItem("Item 9")

class ExsaveTests(joefx.JoeTestBase):
    def __init__(self, *args, **kwargs):
        super().__init__(*args, **kwargs)
        self.text = "\n".join("line %d" % i for i in range(10))
    
    def setUp(self):
        super().setUp()
        self.workdir.fixtureData("test", self.text)
        self.startup.args = ("test",)
    
    def test_exsave_nomodify(self):
        self.startJoe()
        self.cmd("exsave")
        self.assertExited()
    
    def test_exsave_modified(self):
        self.startJoe()
        
        self.cmd("eof")
        self.write(" - more text")
        self.cmd("exsave")
        
        self.assertExited()
        self.assertFileContents("test", self.text + " - more text")
    
    def test_exsave_block(self):
        self.startJoe()
        
        self.cmd("bof,markb")
        self.cmd("eof,markk")
        self.cmd("exsave")
        
        self.assertExited()
    
    def test_exsave_modified_block(self):
        self.startJoe()
        
        self.cmd("eof")
        self.write(" - more text")
        self.cmd("markk,bof,markb")
        self.cmd("exsave")
        
        self.assertExited()
        self.assertFileContents("test", self.text + " - more text")

# TODO: bol
# TODO: bolmenu
# TODO: bop
# TODO: bos
# TODO: brpaste
# TODO: brpaste_done
# TODO: bufed
# TODO: build
# TODO: byte
# TODO: cancel
# TODO: cd
# TODO: center
# TODO: charset
# TODO: ctrl
# TODO: col
# TODO: complete
# TODO: copy
# TODO: crawll
# TODO: crawlr
# TODO: defmdown
# TODO: defmup
# TODO: defmdrag
# TODO: defm2down
# TODO: defm2up
# TODO: defm2drag
# TODO: defm3down
# TODO: defm3up
# TODO: defm3drag
# TODO: defmiddledown
# TODO: defmiddleup
# TODO: delbol
# TODO: delch
# TODO: deleol
# TODO: dellin
# TODO: delw
# TODO: dnarw
# TODO: dnarwmenu
# TODO: dnslide
# TODO: dnslidemenu
# TODO: drop
# TODO: dupw
# TODO: edit
# TODO: else
# TODO: elsif
# TODO: endif
# TODO: eof
# TODO: eofmenu
# TODO: eol
# TODO: eolmenu
# TODO: eop
# TODO: execmd
# TODO: explode
# TODO: exsave
# TODO: extmouse
# TODO: ffirst
# TODO: filt
# TODO: finish
# TODO: fnext
# TODO: format
# TODO: fmtblk
# TODO: fwrdc
# TODO: gomark
# TODO: gparse
# TODO: grep
# TODO: groww
# TODO: if
# TODO: isrch
# TODO: jump
# TODO: killjoe
# TODO: killproc
# TODO: help
# TODO: home
# TODO: hnext
# TODO: hprev
# TODO: insc
# TODO: keymap
# TODO: insf
# TODO: language
# TODO: lindent
# TODO: line
# TODO: lose
# TODO: lower
# TODO: ltarw
# TODO: ltarwmenu
# TODO: macros
# TODO: debug_joe
# TODO: markb
# TODO: markk
# TODO: markl
# TODO: math
# TODO: maths
# TODO: menu
# TODO: mode
# TODO: msg
# TODO: mfit
# TODO: mwind
# TODO: name
# TODO: nbuf
# TODO: nedge
# TODO: nextpos
# TODO: nextw
# TODO: nextword
# TODO: nmark
# TODO: notmod
# TODO: nxterr
# TODO: open
# TODO: parserr
# TODO: paste
# TODO: pbuf
# TODO: pedge
# TODO: pgdn
# TODO: pgdnmenu
# TODO: pgup
# TODO: pgupmenu
# TODO: picokill
# TODO: play
# TODO: prevpos
# TODO: prevw
# TODO: prevword
# TODO: prverr
# TODO: psh
# TODO: pop
# TODO: popabort
# TODO: qrepl
# TODO: query
# TODO: querysave
# TODO: quote
# TODO: quote8
# TODO: record
# TODO: redo
# TODO: release
# TODO: reload
# TODO: reloadall
# TODO: retype
# TODO: rfirst
# TODO: rindent
# TODO: run
# TODO: rsrch
# TODO: rtarw
# TODO: rtarwmenu
# TODO: rtn
# TODO: save
# TODO: savenow
# TODO: scratch
# TODO: scratch_push
# TODO: secure_type
# TODO: select
# TODO: setmark
# TODO: shell
# TODO: showerr
# TODO: showlog
# TODO: shrinkw
# TODO: splitw
# TODO: stat
# TODO: stop
# TODO: swap
# TODO: switch
# TODO: sys
# TODO: tabmenu
# TODO: tag
# TODO: tagjump
# TODO: toggle_marking
# TODO: then
# TODO: timer
# TODO: tomarkb
# TODO: tomarkbk
# TODO: tomarkk
# TODO: tomatch
# TODO: tomouse
# TODO: tos
# TODO: tw0
# TODO: tw1
# TODO: txt
# TODO: type
# TODO: undo
# TODO: uparw
# TODO: uparwmenu
# TODO: upper
# TODO: upslide
# TODO: upslidemenu
# TODO: vtbknd
# TODO: xtmouse
# TODO: yank
# TODO: yapp
# TODO: yankpop
