# JOE Test Suite Design/Implementation

## Overview
JOE's test suite is written in Python and run using its builtin
[unittest](https://docs.python.org/3/library/unittest.html) framework.  The
`runtests` script in the root directory checks for python3 (the only
external dependency) and then runs the entire suite.  Python3 is
preinstalled on many systems and is available for almost every platform
supported by JOE.  Using the defacto unit test platform means that the test
suite should integrate easily with IDEs and CI systems.

Each test starts an instance of JOE in a pty, sends input, and verifies
output.  None of these tests are _unit tests_, but could be classified as
_functional_ or _integration_ tests.

The `tests` directory contains the test cases themselves as well as a few subdirectories:
* `pyte` is the [terminal emulation library](https://pypi.python.org/pypi/pyte)
  used by the tests to check JOE's output
* `joefx` contains some JOE-specific test framework pieces
* `fixtures` contains input files used in certain test cases

## Tests
The test suite is not fully fleshed-out yet so there's not much to write
about here.  It should be the section that lays out the organization of the
existing tests...  but there aren't enough of them yet.

TODO for now.  What I'd *like* to get to is:

* Commands: At least one test class for each command in the editor, most of
  which are specified in `commands.py`, but some sets of commands are broken
  out into other files, like `find.py` and `blocks.py`.
* Modes/options - test each mode and configuration option.
* Encodings
* Features
...

## Test framework
The test framework found in `joefx` is responsible for setting up the
environment for JOE, starting JOE, interacting with a virtual _screen_, and
finally stopping and cleaning up after JOE.  This is primarily implemented
in two modules: `testbase` and `controller`.  Test classes should all
inherit from `testbase.JoeTestBase`, which integrates the functions in
`controller` with the unittest lifecycle, and provides some additional
abstractions useful to the tests.  `joefx` has additional code that can read
and write `joerc` files, map from the keyboard to ANSI escape codes, and
manage files in the working directory; these are all generally exposed
through the base class.

### General test anatomy
To illustrate, here's a fairly boring test case that writes some text, saves
a file, exits, and checks that the file contains the expected text.

```python
class SomeFeatureTests(joefx.JoeTestBase):
    def test_feature_with_some_properties(self):
        # Setup editor environment (optional).
        #   - Write files
        #   - Set environment variables, set commandline parameters
        #   - Setup joerc configuration
        
        # Start the editor
        self.startJoe()

        # Perform some actions, make assertions
        self.write("Some sample text")
        self.assertTextAt("Some sample text", x=0)

        self.save("outfile")

        # Shutdown the editor (optional - see below)
        self.exitJoe()

        # Assert post-conditions (optional)
        self.assertFileContents("outfile", "Some sample text")
```

Note that the editor must be explicitly started with `startJoe`, which
allows the test an opportunity to set up the environment (which this test
does not do -- see the Fixtures and Configuration sections below).  The
editor can be exited later with `exitJoe` (which invokes the `killjoe`
command) followed by testing for postconditions.  Alternately, you can
manually exit the editor (e.g., `self.cmd("abort")`), or not exit the editor
(in which case it will be killed).

### Assertions
The assertions provided by the test base class are:

* `assertCursor(x, y)` asserts that the cursor is at the specified location
* `assertTextAt(text, x, y, dx, dy)` asserts the screen has the specified
  text at the specified location
* `assertSelectedTextEquals(text, x, y, dx, dy)` asserts that a block of
  selected (reverse attribute) text found at the location is equal to the
  specified text.
* `assertSelectedText(f, x, y, dx, dy)` is the same as
  `assertSelectedTextEquals` except a function is used to validate that the
  selected text satisfies some condition and returns true.
* `assertSelectedMenuItem(label)` asserts that a menu item matches a label. 
  Label text can be pulled directly from the joerc file.
* `assertFileContents(file, expected)` asserts that the specified file
  (found in the `work` directory) matches the expected contents.
* `assertExited()` asserts that the editor has terminated successfully with
  a zero exit code.

#### Cursor positions
Many of these assertion methods read from the screen and accept an input
position.  This position is specified with named parameters `x`, `y`, `dx`,
and `dy`.  Each parameter is optional; if `x` or `y` is omitted, that
coordinate is replaced by the cursor's current position.  `dx` and `dy`
default to zero, and are added to the `x` or `y` value.  Negative `x` and
`y` values are permitted and wrap around to the right and bottom
respectively.  Examples:

* `x=0` refers to the first column on the same row as the cursor
* `x=0, dy=-1` refers to the first column on the row immediately above the cursor
* `x=0, y=-1` refers to the first column on the last row of the screen

### Editor interaction
Data is written to the editor through two methods on the base class:

* `write` writes raw data to the editor.  If bytes are specified then they
  are passed directly onto the editor.  If a string is specified, then it is
  converted to UTF-8 before being passed to the editor.
* `writectl` processes its input string before passing it to the editor.
    * Keystrokes can be preceded by `^` to add a control modifier, and `+`
      to add a shift modifier.
    * Non-alphanumeric keys can be specified inside braces (`{}`).
    * Keys (inside braces) can be repeated with `*`, e.g. `{right*10}`.

The test base class also contains methods that shorten some common editor
interactions.  For example, `cmd` method invokes a command string.  This
works by writing `^\[X` to bring up the command prompt, waits for that prompt
to show up on the screen, and then writes the specified input and sends a
carriage return.  There are other methods, such as `save`, `mode`, `menu`,
`encoding`, `find`, `replace`, `selectMenu`, and those are worth a look in
the source.

### Editor output
The framework tries to read output from the editor at every opportunity, but
does not do so continuously or asynchronously.  Because the process runs
concurrently to the tests, there may be many intermediate terminal states
that don't match the desired state -- it may or may not have finished
processing the input we just sent to it, for example.  Assertions, under the
hood, work by passing a predicate function to the controller's `expect`
function.  `expect` runs the predicate against the current terminal state,
and if it returns false, waits for more input, updates the terminal and
tries again.  This keeps up until the predicate returns True or a timeout
expires.  The timeout defaults to 1 second, and can be set on the base test
case by modifying `self.timeout`.

This has a few ramifications: 
* Tests will typically delay for a second before they fail
* You should consider lengthening the timeout before embarking on a
  relatively expensive operation
* You should take care to not accidentally catch an intermediate state in
  your assertions: for example, when using relative cursor positions, you
  should avoid false-positives that might be read from the status line
  during a screen update.

### Fixtures
Before starting the editor process, the test framework sets up two temporary
directories: `home` contains editor's configuration and state files; `work`
contains input and output files.  The `HOME` environment variable is set to
the `home` directory and the current working directory is changed to the
`work` directory.  A _fixture_ is a data structure that models how these
directories should look.  The `fixtures` module implements this data
structure and ensures that reality matches the specification.

The `FixtureDir` class represents a directory, and the test base has two
instances as fields: `home` and `work` from before.  These can be populated
by calling methods on either:
* `fixtureFile(name, file)` copies `file` (which should be located in the
  `fixtures` subdirectory) to the output directory and calls it `name`
* `fixtureData(name, data)` writes the string or bytes in `data` to `name`
  in the output directory
* `fixtureFunc(name, func)` writes the string or bytes returned from `func`
  to `name` in the output directory.  `func` is evaluated only just before
  the editor is started.
* `dir(name)` creates a subdirectory called `name` and returns a new
  FixtureDir object for it.

As the framework writes out the fixture model to disk, it also deletes all
existing files that aren't in the model.  As the model is recreated for each
test case, this means that files leftover from previous invocations (such as
`backup~` files, `DEADJOE`s, and `.joe_state`s) are all removed.

### Configuration
The framework also reads and writes joerc files.  This allows tests to
programmatically modify the configuration before the editor is started and
inspect it while running (for example, to read menu strings).  Before the
test case is run, the `rc/joerc` file is read and parsed into a model.  The
root of the model is available in the `config` field of the test base class
and can be modified up to the point where `startJoe` is called.

The tests' `self.config` is an instance of `joefx.rcfile.RCFile`, which has
the following parts:
* `globalopts` is an instance of the `Options` class and contains the global
  options defined at the beginning of `joerc`.  They are specified as direct
  properties on the object, e.g.  `self.config.autoindent = False`.  Boolean
  options can have one of three values:
    * `None` to omit the option from the configuration file and take the default
    * `True` to explicitly enable the option
    * `False` to explicitly disable the option
    * Many settings can also have strings
* `fileopts` is an array of `FileOptions` instances and each maps to a file
  type generally found in `ftyperc`.  It contains a list of
  `FileExtensionRule` instances to match against files, and another instance
  of `Options` (like above) that contains the options for that file type.
* `help` is an array of `HelpScreen` instances, which map to help screens.
* `macros` is an array of `MarcroDefinition` instances, which represent
  macros defined by `:def macro <definition>`
* `bindings` is an array of `KeyBindingCollection` instances, which
  represent sections of keybindings.  Each has another property `bindings`,
  an array of `Binding` instances that map to individual bindings.

Once the test framework first loads the `joerc` file, it keeps one pristine
instance and clones out a copy each time a test case is executed.  The
`{home}/.joerc` is actually written by the fixture system, specifically by
`fixtureFunc`, which allows the model to be changed at any time before
startJoe is invoked.
