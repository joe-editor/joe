
import os
from .exceptions import *

GLOBAL_OPTS = set([
    'help_is_utf8', 'mid', 'marking', 'asis', 'force', 'nolocks', 'nomodcheck', 'nocurdir',
    'nobackups', 'nodeadjoe', 'break_hardlinks', 'break_links', 'lightoff', 'exask', 'beep',
    'nosta', 'keepup', 'pg', 'undo_keep', 'csmode', 'backpath', 'floatmouse', 'rtbutton',
    'nonotice', 'noexmsg', 'noxon', 'orphan', 'dopadding', 'lines', 'baud', 'columns',
    'help', 'skiptop', 'notite', 'nolinefeeds', 'usetabs', 'assume_color', 'assume_256color',
    'guess_non_utf8', 'guess_utf8', 'guess_utf16', 'guess_crlf', 'guess_indent', 'menu_above',
    'transpose', 'menu_explorer', 'menu_jump', 'notagsmenu', 'icase', 'wrap', 'autoswap',
    'joe_state', 'mouse', 'joexterm', 'brpaste', 'pastehack', 'square', 'text_color',
    'status_color', 'help_color', 'menu_color', 'prompt_color', 'msg_color', 'restore',
    'search_prompting', 'regex', 'lmsg', 'rmsg', 'smsg', 'zmsg', 'xmsg', 'highlight', 'istep',
    'wordwrap', 'autoindent', 'aborthint', 'helphint'
])

FILE_OPTS = set([
    'cpara', 'cnotpara', 'encoding', 'syntax', 'hex', 'highlight', 'smarthome', 'indentfirst',
    'smartbacks', 'tab', 'indentc', 'spaces', 'istep', 'purify', 'crlf', 'wordwrap', 'nobackup',
    'autoindent', 'overwrite', 'picture', 'lmargin', 'rmargin', 'flowed', 'french', 'linums',
    'rdonly', 'keymap', 'lmsg', 'rmsg', 'mfirst', 'mnew', 'mold', 'msnew', 'msold',
    'highlighter_context', 'single_quoted', 'no_double_quoted', 'c_comment', 'cpp_comment',
    'pound_comment', 'vhdl_comment', 'semi_comment', 'tex_comment', 'text_delimiters',
])

OPTS_WITH_ARGS = set([
    # Global
    'undo_keep', 'backpath', 'lines', 'baud', 'columns', 'skiptop', 'text_color', 'status_color',
    'help_color', 'menu_color', 'prompt_color', 'msg_color', 'lmsg', 'rmsg', 'smsg', 'zmsg',
    # File
    'cpara', 'cnotpara', 'encoding', 'syntax', 'tab', 'indentc', 'istep', 'lmargin', 'rmargin',
    'keymap', 'mfirst', 'mnew', 'mold', 'msnew', 'msold', 'text_delimiters'
])

class RCFile(object):
    def __init__(self):
        self.globalopts = Options(GLOBAL_OPTS)
        self.fileopts = []
        self.help = []
        self.menus = []
        self.macros = []
        self.bindings = []
    
    def serialize(self):
        result = []
        result.extend(self.globalopts.serialize())
        for section in (self.fileopts, self.help, self.menus, self.macros, self.bindings):
            for item in section:
                result.extend(item.serialize())
        return b'\n'.join((item.encode('utf-8') if isinstance(item, str) else item) for item in result)
    
    def clone(self):
        other = RCFile()
        other.globalopts = self.globalopts.clone()
        other.fileopts = [fopt.clone() for fopt in self.fileopts]
        other.help = [help.clone() for help in self.help]
        other.menus = [menu.clone() for menu in self.menus]
        other.macros = [macro.clone() for macro in self.macros]
        other.bindings = [binding.clone() for binding in self.bindings]
        return other

class Options(object):
    def __init__(self, properties):
        self._properties = properties
        self._values = {}
    
    def __getattr__(self, name):
        return self.getValue(name)
    
    def __setattr__(self, name, value):
        if name.startswith('_'):
            object.__setattr__(self, name, value)
        else:
            self.setValue(name, value)
    
    def getValue(self, name):
        if name not in self._properties:
            raise InvalidProperty(name)
        if name not in self._values:
            return None
        else:
            return self._values[name]
    
    def setValue(self, name, value):
        if name not in self._properties:
            raise InvalidProperty(name)
        else:
            if (name in OPTS_WITH_ARGS) == isinstance(value, bool):
                raise InvalidPropertyValue(name)
            self._values[name] = value
    
    def serialize(self):
        result = []
        for k, v in self._values.items():
            if v is True:
                result.append('-' + k)
            elif v is False:
                result.append('--' + k)
            elif v is not None:
                result.append('-%s %s' % (k, v))
        return result
    
    def clone(self):
        other = Options(self._properties)
        other._values.update(self._values)
        return other

class FileOptions(object):
    def __init__(self):
        self.name = ''
        self.extensions = []
        self.patterns = []
        self.options = Options(FILE_OPTS)
    
    def serialize(self):
        result = []
        result.append('[%s]' % self.name)
        result.extend(self.extensions)
        result.extend('+' + pat for pat in self.patterns)
        result.extend(self.options.serialize())
        return result
    
    def clone(self):
        other = FileOptions()
        other.name = self.name
        other.extensions = self.extensions[:]
        other.patterns = self.patterns[:]
        other.options = self.options.clone()
        return other

class Menu(object):
    def __init__(self):
        self.name = ''
        self.back = ''
        self.items = []
    
    def serialize(self):
        result = [':defmenu %s %s' % (self.name, self.back)]
        result.extend(item.serialize() for item in self.items)
        return result

    def clone(self):
        other = Menu()
        other.name = self.name
        other.back = self.back
        other.items = [item.clone() for item in self.items]
        return other

class MenuItem(object):
    def __init__(self):
        self.macro = ''
        self.label = ''
    
    def serialize(self):
        return '%s\t%s' % (self.macro, self.label)
    
    def clone(self):
        other = MenuItem()
        other.macro = self.macro
        other.label = self.label
        return other

class HelpScreen(object):
    def __init__(self):
        self.name = ''
        self.content = []
    
    def serialize(self):
        return ['{' + self.name] + self.content + ['}']
    
    def clone(self):
        other = HelpScreen()
        other.name = self.name
        other.content = self.content[:]
        return other

class KeyBindingCollection(object):
    def __init__(self):
        self.name = ''
        self.inherits = None
        self.bindings = []
    
    def serialize(self):
        if not self.name:
            # Uninitialized
            return []
        
        result = [':' + self.name]
        if self.inherits is not None:
            result.append(':inherit ' + self.inherits)
        result.extend([f.serialize() for f in self.bindings])
        return result
    
    def clone(self):
        other = KeyBindingCollection()
        other.name = self.name
        other.inherits = self.inherits
        other.bindings = [b.clone() for b in self.bindings]
        return other

class Binding(object):
    def __init__(self):
        self.macro = None
        self.keys = []
    
    def serialize(self):
        return self.macro + ' ' + ' '.join(self.keys)
    
    def clone(self):
        other = Binding()
        other.macro = self.macro
        other.keys = self.keys[:]
        return other

class MacroDefinition(object):
    def __init__(self):
        self.name = None
        self.macro = None
    
    def serialize(self):
        return [':def %s %s' % (self.name, self.macro)]
    
    def clone(self):
        other = MacroDefinition()
        other.name = self.name
        other.macro = self.macro
        return other

class ParserState(object):
    def __init__(self, rcfile, filegen):
        self.rcfile = rcfile
        self.file = filegen
        self.curline = None
    
    def begin(self):
        try:
            self.parseglobal()
            self.parsefileopts()
            self.parsemenus()
            self.parsehelp()
            self.parsebindings()
        except StopIteration:
            pass
    
    def parseglobal(self):
        while True:
            line = self.nextnows()
            if line.startswith('-'):
                self.parseoption(self.rcfile.globalopts)
            else:
                break
    
    def parseoption(self, opts):
        mode = not self.curline.startswith('--')
        parts = self.curline.split(None, 1)
        optionName = parts[0].lstrip('-')
        if len(parts) == 1 or optionName not in OPTS_WITH_ARGS:
            opts.setValue(optionName, mode)
        else:
            opts.setValue(optionName, self.curline[len(parts[0]) + 1:].rstrip('\r\n'))
    
    def parsemacro(self, line):
        i = 0
        q = False
        bs = False
        while i < len(line):
            c = line[i]
            if q:
                if bs:
                    bs = False
                elif c == '\\':
                    bs = True
                elif c == '"':
                    q = False
            elif c == '"':
                q = True
            elif c.isspace():
                return line[:i], line[i:].lstrip()
            
            i += 1
        
        return line, ''
    
    def parsefileopts(self):
        while self.curline.startswith('['):
            filetype = FileOptions()
            filetype.name = self.curline.strip().strip('[]')
            
            while True:
                line = self.nextnows()
                if line.startswith('*'):
                    filetype.extensions.append(line.strip())
                elif line.startswith('+'):
                    filetype.patterns.append(line[1:].strip())
                elif line.startswith('-'):
                    self.parseoption(filetype.options)
                else:
                    break
            
            self.rcfile.fileopts.append(filetype)
    
    def parsemenus(self):
        while self.curline.startswith(':defmenu'):
            menu = Menu()
            parts = self.curline.strip().split(None, 2)
            menu.name = parts[1]
            if len(parts) == 3:
                menu.back = parts[2]
            
            while True:
                line = self.nextnows()
                if line.startswith(':') or line.startswith('{'):
                    break
                
                macro, rest = self.parsemacro(line)
                item = MenuItem()
                item.macro = macro
                item.label = rest.strip()
                menu.items.append(item)
            
            self.rcfile.menus.append(menu)
    
    def parsehelp(self):
        while self.curline.startswith('{'):
            screen = HelpScreen()
            screen.name = self.curline[1:].strip()
            while not self.nextbytes().startswith(b'}'):
                screen.content.append(self.curline.rstrip(b'\r\n'))
            
            self.rcfile.help.append(screen)
            self.nextnows()
    
    def parsebindings(self):
        currentSection = None
        while True:
            if self.curline.startswith(':def '):
                # Macro
                macro = MacroDefinition()
                _def, macro.name, macro.macro = self.curline.split(None, 2)
                self.rcfile.macros.append(macro)
            elif self.curline.startswith(':inherit '):
                # Inheritance specification
                currentSection.inherits = self.curline[len(':inherit '):].strip()
            elif self.curline.startswith(':'):
                # New section
                currentSection = KeyBindingCollection()
                self.rcfile.bindings.append(currentSection)
                parts = self.curline.split()
                currentSection.name = parts[0][1:]
            else:
                # Binding
                binding = Binding()
                binding.macro, keystr = self.parsemacro(self.curline)
                
                # Split out keys
                keys = keystr.split()
                for k in keys:
                    if self.iskey(k):
                        binding.keys.append(k)
                    else:
                        break
                
                currentSection.bindings.append(binding)
            
            self.nextnows()
    
    def iskey(self, k):
        if len(k) == 1: return True
        if k.startswith('U+'): return True
        if k.startswith('^') and len(k) == 2: return True
        if k.startswith('.k') and len(k) == 3: return True
        if k in ('MDOWN', 'MDRAG', 'MUP', 'M2DOWN', 'M2DRAG', 'M2UP', 'M3DOWN', 'M3DRAG',
                 'M3UP, MWDOWN', 'MWUP', 'SP', 'TO'):
            return True
        return False
    
    def nextbytes(self):
        self.curline = next(self.file)
        return self.curline
    
    def next(self):
        self.curline = next(self.file).decode('utf-8').strip('\r\n')
        return self.curline
    
    def nextnows(self):
        while True:
            line = self.next()
            if len(line.strip()) > 0 and not line[0].isspace():
                return line

def readFile(filename):
    with open(filename, 'rb') as f:
        for line in f:
            if line.startswith(b':include'):
                args = line.decode('utf-8').split()
                for included in readFile(os.path.join(os.path.dirname(filename), args[1])):
                    yield included
            else:
                yield line

def parse(filename):
    result = RCFile()
    ParserState(result, readFile(filename)).begin()
    return result
