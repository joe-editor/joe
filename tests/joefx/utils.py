
import re

def compareMenuLabel(menuLabel, text):
    pattern = re.sub("\\\\%Z[a-zA-Z]+\\\\%", "(ON|OFF|([0-9]+))", re.escape(menuLabel.replace("% ", " ").strip()))
    return re.match(pattern, text.strip()) is not None
