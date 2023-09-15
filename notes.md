# Scanner

The scanner is generated C code that is generated from a very simple description file. 

## Directives

A directive controls the behavior of the scanner generator. A directive can be placed anywhere in the specification and has global effect. 

- %verbosity number

  Controls the amount of information that is printed out as the scanner generator reads the specification and generated the scanner. If this directive is not provided, then the default is to only print errors.

- %name symbol

  Specifies the name of the output file prefix. A header and a C source file is generated with the name formatted as ``%s_scanner.c`` and ``%s_scanner.h``. If this directive is not provided, then the name is simply ``_scanner.*``. 

- %prefix symbol

  Specify the prefix of the symbols of the global user interface. Same protocol as the %name directive.

- %code { arbitrary C code }

  Specify stand-alone arbitrary C code to be placed in the output before any of the actual scanner code is emitted. If any of the rules need to reference any code, this is where it's to be defined. 

## Rule syntax

A scanner rule consists of the following parts.

return_value ':' pattern '{' code '}' 

- The return value is normally all caps and an ``enum`` is created for each one. This ``enum`` is used as the token type and is placed in the returned data structure. Note that the return value is optional. If there is no return value specified, then no token is created or returned and the pattern is ignored.
- The ':' is the syntax divider between the return_value and the rule.
- A pattern is the sequence that is matched for the token.
  - A literal string starts and ends with a double-quote.
  - A regular expression is enclosed in parentheses.
- The code is executed when the pattern is matched and can be any arbitrary C code that can be placed in a function and enclosed in {}. This code can reference code given in the %code directive. The code section of a rule is optional.

## State machine

The scanner is a simple state machine. First a table is made of every acceptable character and those are assigned an index into the state tables. The state tables contain the next state, based upon the character.

For example, consider the following set of rules:
```C
AAB: "aab" 
AC: "ac"
CD: "cd"
```

There are 4 valid characters in all patterns. They are "a,b,c, and d". There are 3 rules. They are AAB, AC, and CD.  A table is created with 256 entries and all of the entries are marked as invalid characters except the 4 valid characters, where ``a=0, b=1, c=2, d=3``. There are also 2 other entries. There is a single entry for ``error`` (ERR), ``end of file`` (EOF), and ``end of input`` (EOI), where ``ERR=252, EOF=254 and EOI=255``.  These states are "special" and have their own global handlers. So the table for each state will have 4 entries for state changes. 

A range of characters is represented as an array of valid characters where the index of the character is a 1 or a zero. If the slot is a zero, then the character is not in the range.

The end of a rule is indicated by the token number of the rule that ended. 

- Characters are always a number less than 255.
- Tokens are numbered from 300 to how ever many there are.
- State numbers are from 1000 to how ever many there are. Since the states are kept in an array, arithmetic is done on the actual state number to index the array.

So the tables generated for the above rules would look like the following, not considering end of inputs:

```C 
// state 0 accepts 'a' or 'c'
state0 = [1, ERR, 2, ERR, ERR];
// state 1 accepts 'aa' or 'ac', where 'ac' is EOR.
state1 = [3, ERR, AC, ERR, ERR];
// state 2 accepts 'cd' and EOR
state2 = [ERR, ERR, ERR, CD];
// state 3 accepts 'aab' as EOR 
state3 = [ERR, AAB, ERR, ERR];
// all other combinations are errors.
```

We can see that patterns that have duplicate letters create more states so that they can be separated. 

Regular expressions are handled as states as well, but instead of individual letters causing state changes, different kinds of ranges cause state changes.

Consider the following:
```C 
ASD: "asd"
SYM: (a[scef])
```

Valid characters are ``[adefs]`` and there is exactly one character accepted from the range.  So the states would look like:

```C
// the order of the letters does not matter, but it's easier to think about 
// if they are in lexical order.
// a=0, c=1, d=2, e=3, f=4, s=5
// accept 'a'
state0 = [1, ERR, ERR, ERR, ERR, ERR];
// accept 'ac', 'ae', and 'af', but it's unknown if 'as' part of the regex
// or not.
state1 = [ERR, SYM, SYM, SYM, 2];
// return the ASD symbol
state2 = [ERR, ERR, ASD, ERR, ERR];
// Note that there is a potentional bug here. Does the state machine signal 
// completion on the character transition or after it?
```

Question of how to represent zero or more of a range in this scenario. Instead of having misses represented as errors, then have them represented as state changes. For example, if there are multiple ranges in an expression. If it's a miss then switch to the next range/state. If it's a hit, then use it.

```C
ASD: "abx"
// note that the only way this rule can end is with an invalid char.
SYM: ([a-c][a-c0-2]*)
// valid characters: [a,b,c,x,0,1,2]

// 'a=1', 'b=2', 'c=2' 
state0 = [1, 2, 2, ERR, ERR, ERR, ERR, ERR];
// 'aa', 'ab', ac'. If it's 'ab' then it could be ASD or SYM.
state1 = [2, 3, 2, ERR, ERR, ERR, ERR];
// In the second range, which is optional
state2 = [2, 2, 2, ERR, 2, 2, 2]; // self-referencing states are an error
// 'abx' is a token
state3 = [2, 2, 2, ASD, 2, 2, 2];
```

The token SYM is self referencing because there are no valid characters to stop on, such as punctuation or white space. Self-referencing states hang the scanner and should be detected by the software. Also, the ending states have to be in a separate state because of optional characters in ranges. So the state now may or may not consume a character, depending on whether it returns a token or not.

```c
ASD: "abx"
SYM: ([a-c][a-c0-2]*)
SP: ([\n])
// valid chars: [a,b,c,x,0,1,2,\n]

// a, b, c
state0 = [2, 3, 3, E, E, E, E, SP];
// aa, ac, a0, a1, a2. ab is the possibly ASD token
state1 = [E, E, E, E, E, E, E, SP];
// gaaaa....

```



## Patterns

A pattern is a regular expression. Instead of comparing a reg-ex to a string to match it, a scanner needs to compare an expression to a partial string as it is read from the input. 

In this scanner, there are 2 types of pattern.
- The pattern is a literal string that is enclosed in quotes. These are recognized using a switch/case statement where the first one recognizes the first character, then second one recognizes the second character, etc. The token string is built as the string is read and the token type is returned at the end. For example, the pattern ``thestr``. This does not match ``thestring``, but it will match to part of ``thestr{asdf}``. This is because the ``'{'`` character is considered to be an operator, unless it appears inside of a pattern as a literal, because it is ``ispunct()`` instead of ``isalnum()``.
- The pattern can include ranges of characters and uses repetition operators. If an operator is preceded by a back-slash ``\`` then the character is taken literally. A range is enclosed in ``[]`` characters and accepts the ``-`` character to indicate a range. A range that is enclosed in ``()`` characters is not expanded, but is used to apply repetition to a sequence.
- Operators
  - ``?`` one or zero
  - ``*`` zero or more
  - ``+`` one or more

Whitespace characters are ignored except when they are explicitly included in a pattern. 

Tokens are a standard data structure and the scanner makes no effort to return a particular data type. The line number and column number are tracked and given as part of the token that was read.

```c
typedef struct _token_ {
    const char* str;
    TokenType type;
    void* data; // optional data
    const char* fname;
    int line_no;
    int col_no;
} Token;
```

The extra information in the token is presented to facilitate error reporting.

# Scanner interface

The scanner interface is kept to a minimum. All tokens are allocated memory and if a GC is not used, manual freeing them is required.

```c
// Creates internal file stack and primes the token stream. Note that there is no close
// file function because files are closed automatically and popped from the stack when 
// the end of it is reached.
void open_file(const char* fname); 

// Returns the current token. Multiple calls to this function returns the same token,
// in a new allocation. Returns NULL if there is no file currently open.
Token* get_token(); 

// Dispose of the previous token and return the new one in a new allocation. Returns
// NULL if there is no file currently open.
Token* consume_token();

// Simply free the memory associated with this token. This is a NOOP if GC is enabled.
void free_token(Token* tok); 
```

All of the plumbing of file IO is hidden from the scanner interface, but available in the token.
