
import joefx

class EncodingTests(joefx.JoeTestBase):
    def test_hex_mode_and_back(self):
        """Regresion test.  Was not listed as a bug, but was fixed with [11618ed3b38f]"""
        self.workdir.fixtureData("test", "ร้อนนี้คงไม่มีอะไรสำคัญเท่าครีมกันแดดอีกแล้วนะสาวๆ")
        self.startup.args = ("test",)
        self.startJoe()
	
        self.writectl("{right*21}")
        self.assertCursor(16, 1)
        
        self.mode("hex")
        
        # Check cursor movement
        self.assertCursor(75, 4)
        self.writectl("{left}")
        self.assertCursor(74, 4)
        self.writectl("{right}")
        self.assertCursor(75, 4)
        
        self.mode("hex")

        # Check cursor movement
        self.assertCursor(16, 1)
        self.writectl("{left}")
        self.assertCursor(15, 1)
    
    def test_change_updates_cursor(self):
        """Regresion test.  Was not listed as a bug, but was fixed with [11618ed3b38f]"""
        self.workdir.fixtureData("test", "ร้อนนี้คงไม่มีอะไรสำคัญเท่าครีมกันแดดอีกแล้วนะสาวๆ")
        self.startup.args = ("test",)
        self.startJoe()
        
        self.writectl("{right*21}")
        self.assertCursor(16, 1)
        
        # Change encoding to ascii
        self.encoding("ascii")
        
        # Check cursor movement
        self.assertCursor(63, 1)
        self.writectl("{left}")
        self.assertCursor(62, 1)
        self.writectl("{right}")
        self.assertCursor(63, 1)
        
        # Back to utf-8
        self.encoding("utf-8")
        
        # Check cursor movement
        self.assertCursor(16, 1)
        self.writectl("{left}")
        self.assertCursor(15, 1)
    
    def test_utf16le_detection(self):
        """Tests UTF-16 detection + proper save with a UTF-16LE file"""
        self.config.globalopts.guess_utf16 = True
        self.workdir.fixtureData("test", "This is a UTF-16 file\n".encode('utf-16le'))
        self.startup.args = ("test",)
        self.startJoe()
        
        self.assertTextAt("This is a UTF-16 file")
        self.writectl("{down}Extra line{enter}")
        self.save()
        self.cmd("abort")
        self.assertExited()
        
        self.assertFileContents("test", "This is a UTF-16 file\nExtra line\n".encode('utf-16le'))
    
    def test_utf16be_detection(self):
        """Tests UTF-16 detection + proper save with a UTF-16BE file"""
        self.config.globalopts.guess_utf16 = True
        self.workdir.fixtureData("test", "This is a UTF-16 file\n".encode('utf-16be'))
        self.startup.args = ("test",)
        self.startJoe()
        
        self.assertTextAt("This is a UTF-16 file")
        self.writectl("{down}Extra line{enter}")
        self.save()
        self.cmd("abort")
        self.assertExited()
        
        self.assertFileContents("test", "This is a UTF-16 file\nExtra line\n".encode('utf-16be'))
    
    def test_utf16_detection_off(self):
        """Tests that UTF-16 detection is not performend when not enabled through joerc (+ test sanity check)"""
        self.config.globalopts.guess_utf16 = False
        self.workdir.fixtureData("test", "This is a UTF-16 file\n".encode('utf-16le'))
        self.startup.args = ("test",)
        self.startJoe()
        
        self.assertTextAt("T@h@i@s@ @i@s@ @a@ @U@T@F@-@1@6@ @f@i@l@e@")
    
    def test_external_cp1253_charmap(self):
        greek_text = "Δύο διαγωνισμοί πρόσληψης μόνιμου προσωπικού θα γίνουν προσεχώς,\nμέσω του ΑΣΕΠ, στα ΕΛΤΑ και στην ΕΥΔΑΠ. Πρόκειται για 810\nθέσεις με συμβάσεις αορίστου χρόνου. Οι διαγωνισμοί θα είναι\nδύο και θα ανακοινωθούν το επόμενο διάστημα. Ο "
        self.homedir.dir('.joe').dir('charmaps').fixtureFile('my-cp1253', 'cp1253')
        self.workdir.fixtureData('test', greek_text.encode('cp1253'))
        self.startup.args = "test",
        self.startJoe()
        
        self.encoding("my-cp1253")
        
        for i, line in enumerate(greek_text.split('\n')):
            self.assertTextAt(line, x=0, y=1 + i)
        
        self.exitJoe()
