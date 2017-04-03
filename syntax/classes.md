
# Common syntax classes

*TODO*: I need to expand this, but for now I just want to document the
common syntax classes...


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

