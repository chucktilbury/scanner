# This document is my ramblings about regular expressions

The kind of regex that I am interested in is to match tokens from an input stream,rather than to match a specific string against a specific regular expression. So the idea is to match a set of regular expressions incrementally. It can be thought of as the expressions for all of the token names OR’ed together. They are all searched at the same time.

## The emitted scanner

Rather than creating tables to implement an NFA/DFA, I am going to use an array of recognizer functions. Each array element will have a flag for a possible match and a score. When a character is read, then the whole array is scanned and entries where the possible match flag is set will have the recognizer function run to see if the entry is a possible match. If it is, then the score is incremented and the flag is set. If the character cannot match, then the flag is cleared. When the total score no longer changes, then what ever has been matched is the actual match. The entry with the highest score represents the type of the token. 



For literal tokens, the recognizer has a simple array that represents the token to recognize from the input. If the character read matches at the string, then the recognizer succeeds and the score is incremented. 



For constructed tokens, a series of characters is in each set. Literal strings are acceptable as well. These have their own recognizer. There is a data structure on every element of the token that implements the operator and character set associated with it. In addition to the other flags there is a completed flag. The data structures are presented in order and only one is tested against the current character. 

## To implement this scanner

```C
// Returns true if the ch matches at the actual index into the str. 
typedef void RecFunc(int ch, RuleRec* ds);

// These are functions implemented as RecFunc().
// Checks a literal string at a particular index.
void rec_rule_lit(int ch, RuleRec* ds);

// With these, the token ID is given by the top level rule. The token ID that is
// in the lower level depermines which one of these functions is called.

// Checks if the ch is anywhere in the str. Zero or more.
void rec_rule_zom(int ch, RuleRec* ds);
// checks if the ch is anywhere in the str. One or more.
void rec_rule_oom(int ch, RuleRec* ds);
// Checks if the ch is anywhere in the str. Zero or one.
void rec_rule_zoo(int ch, RuleRec* ds);
// Checks if the ch is anywhere in the str. Exactly one.
void rec_rule_eo(int ch, RuleRec* ds);

// when there are constructed rules, the str is NULL. If the str is not
// NULL, then this is a top-level literal match.
typedef struct _rule_rec_ {
    int token_id;
    const char* str;
    struct _rule_rec_ rules[];
    bool possible;
    int score;
} RuleRec;

// This is the main table.
typedef struct {
    int score;
    int prev_score;
    RecRule table[];
} RecTable;
```



The issue is how to use the functions that implement the constructed rules. They need to be evaluated in order but the end score is used to decide the match and get the token_id. Each rule has its own array of RuleRec and evaluates according to other rules.