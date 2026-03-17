import re
import sys
import unittest

# Platform detection constants
IS_MAC = sys.platform == "darwin"
IS_LINUX = sys.platform.startswith("linux")
IS_CYGWIN = sys.platform == "cygwin"

def skip_on_macos(reason="Currently unstable on macOS"):
    return unittest.skipIf(IS_MAC, reason)

def skip_on_linux(reason="Currently unstable on Linux"):
    return unittest.skipIf(IS_LINUX, reason)

def skip_on_cygwin(reason="Currently unstable on Cygwin"):
    return unittest.skipIf(IS_CYGWIN, reason)

def skip_unless_linux(reason="Test only supported on Linux"):
    return unittest.skipUnless(IS_LINUX, reason)

def compareMenuLabel(menuLabel, text):
    pattern = re.sub(r"\%Z[a-zA-Z]+\%", "(ON|OFF|([0-9]+))", re.escape(menuLabel.replace("% ", " ").strip()))
    return re.match(pattern, text.strip()) is not None
