
# Colors in JOE

Colors are no longer specified by syntax files, but are instead pulled from
color schemes.  This requires syntax and color scheme files to share a
common convention so that colors can neatly propagate into syntax
highlighting as intended.  All of the builtin syntax files and schemes
follow the convention that is described here.

To review, the color classes in syntax files must either match a class found
in the color scheme, or reference one.  For example:

    =Constant
    =String +Constant

`String` will be the first of these to apply:

* `<syntax name>.String` as defined in the color scheme
* `String` as defined in the color scheme
* `<syntax name>.Constant` as defined in the color scheme
* `Constant` as defined in the color scheme
* The default text color

Multiple "fallback" classes can be referenced in a class definition; JOE
will take the first that exists.  This example is fairly straightforward, as
the constant class can be thought of as a superset of strings.  In this
manner, a hierarchy of language elements emerges.

Generally, a color scheme should be able to get good mileage by defining a
very minimal set of classes: `Constant`, `Comment`, `Keyword`, `Type`, and
`Preproc` will produce a satisfactory result for most languages.

Naturally, there are outliers.  `diff` and `xml` are both very common and
don't fit neatly into the hierarchy that works for most other languages. 
This is worked around by making new classes specific to each's respective
syntaxes and falling back to color classes that are likely to be distinct. 
In these specific cases, fallback color classes were chosen based on which
classes were inherited from in vim for those elements.  (Though in practice,
the existing schemes specify classes for xml and diff specifically).

In general, it's OK for each syntax file to define classes that are not on
this list as the designer sees fit, as long as they can be mapped to
something sensible from the list.  The intended design is that the syntax
files proffer up as many of the language elements as feasible, the color
scheme provides a palette, and the class definitions map between the two.

## Overview of color classes

What follows are the most common classes used among the existing syntaxes:
what entities in languages they describe, what other classes they can
inherit their colors from.  New syntaxes should use these class definitions
verbatim unless there's a good reason to inherit from something else
(exceptions abound in the built-in syntaxes).

While it's best to study the built-in syntaxes, this list should serve as a
reasonable summary and reference, especially where the intent is otherwise
unclear.

### General classes

```
=Idle
```
* The default coloring. Normal text.

```
=Ident
```
* Identifiers; this is *any* identifier as opposed to definitions.

```
=Bad
```
* Used to indicate a parse error. Generally red or yellow text or background.

### Specific identifiers

```
=DefinedIdent +Ident
```
* An identifier that is being defined at this point.  This might be a class,
function, or struct (... or union or enum, etc).

```
=DefinedFunction +DefinedIdent
```
* More specific than `DefinedIdent`. This is the identifier in a function
definition.

```
=DefinedType +DefinedIdent
```
* More specific than `DefinedIdent`. This is the identifier in a type
definition.

```
=Label +DefinedIdent
```
* Jump labels, case statements.


### Preprocessor

```
=Preproc
```
* Preprocessor directives.

```
=Precond +Preproc
```
* Conditional preprocessor directives, e.g. #if, #ifdef, #else, #endif, etc

```
=Define +DefinedIdent +Preproc
```
* Preprocessor definitions.  This is the *identifier* that is being defined.

```
=Include +Preproc
```
* Include directives

```
=IncLocal +String +Preproc
```
* The file being included if it is *local*, e.g. quoted in C.

```
=IncSystem +Preproc +Keyword
```
* The file being included if it is a system include, e.g. angle brackets in
C.

```
=Attribute +Define +Preproc
```
* A decorator in Python, an attribute in Elixir.


### Comments

```
=Comment
```
* Comments.

```
=TODO +Comment
```
* Flags TODO, FIXME, HACK, NOTE, BUG, XXX in comments.


### Constants

```
=Constant
```
* Base syntax class for any constant.

```
=Number +Constant
```
* Numeric constants.

```
=Boolean +Constant
```
* `true` or `false`

```
=String +Constant
```
* String constants

```
=Escape
```
* Escape characters.  Generally line continuations or the base for special
characters in strings.

```
=StringEscape +Escape +String
```
* Special characters in a string: \\n, %s, etc

```
=Character +String +Constant
```
* Literal character constants

```
=CharacterEscape +StringEscape +Character
```
* Special character in a character constant: \n, %s, etc

```
=Regex +String
```
* A literal regular expression

```
=RegexEscape +StringEscape
```
* An escape sequence in a regular expression

```
=Variable +Escape
```
* A variable in languages that prefix it, e.g. `$foo` or `%bar%`

```
=Docstring +Comment
```
* Documentation strings

```
=DocstringLabel +Docstring
```
* `TODO` in docstrings in Python. Should be renamed.

```
=DocEscape +Escape +Docstring
```
* Escapes in documentation strings.

```
=StringVariable +StringEscape
```
* A variable interpolated in a string

```
=RegexVariable +StringEscape
```
* A variable interpolated in a string


### Language constructs

```
=Keyword
```
* Base class for a reserved keyword in the language

```
=Statement +Keyword
```
* A statement keyword, e.g. `for`, `if`, `switch`, etc in C.

```
=Loop +Statement
```
* A statement keyword that defines a loop.

```
=Conditional +Statement
```
* A statement keyword that definese a conditional.

```
=Operator +Keyword
```
* An operator keyword. Different languages use this one differently and I'm
not sure what is the correct usage. Should it be *all* operators or only
those that have names. E.g., in python this would include `in`, `is`, etc
but not `+`, `*`, `-`, `/`, etc.  Those are typically handled by `Control`.

```
=Type
```
* A builtin type

```
=StorageClass +Type +Keyword
```
* Keywords that modify the storage class, e.g. `volatile`, `static`,
`const`, etc.

```
=Structure +Type +Keyword
```
* Keywords that declare aggregate data types, `struct`, `class`, etc.

```
=Brace
```
* Braces, in languages that use those to delimit blocks: `{`, `}`

```
=Control
```
* Other symbolic characters that have meaning in the language, generally
operators.

```
=Builtin +DefinedFunction
```
* Builtin or primitive functions, e.g. `len`.


### Misc/special

(Yes, the bases are weird for diff)

```
=diff.DelLine +Escape
```
* Deleted line in a diff

```
=diff.AddLine +DefinedIdent
```
* Added line in a diff

```
=diff.ChgLine +Preproc
```
* Changed line in a diff

```
=CppKeyword +Keyword
```
* Specific to C: a C++ keyword

```
=Tag +DefinedFunction +DefinedIdent
```
* Tag edge (< and >) in XML and HTML

```
=TagName +Tag
```
* Tag name

```
=TagEnd +Tag
```
* End tag (the whole thing)

```
=Attr +Type
```
* Attribute name in XML/HTML tag

```
=Entity		+Escape
```
* XML and HTML entities

```
=StringEntity	+StringEscape +Entity
```
* Entites found inside of attribute values, which are Strings

```
=Title
```
* Titles found, e.g. mail subjects or markdown headers

```
=Namespace	+Comment
```
* XML namespace prefixes

More in XML and HTML:
* `PIStart`, `PIBody`, `PIEnd`
* `CdataEdge`, `CdataBody`
* `Decl`
