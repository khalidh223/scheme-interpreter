# About
An interpreter, written in C, for various Scheme expressions. It does so by first tokenizing a file containing the Scheme expression(s) into symbols and numbers, parsing these tokens into a nested linked list while nesting it based on the number of open and close parentheses, before finally interpreting the resulting tree. For simplicity, here is the expected syntax for the tokenizer expects at the moment:

For numbers - 
```
<number>   ->  <sign> <ureal> | <ureal>
<sign>     ->  -
<ureal>    ->  <uinteger> | <udecimal>
<uinteger> ->  <digit>+
<udecimal> ->  . <digit>+ | <digit>+ . <digit>*
<digit>    ->  0 | 1 | ... | 9
```
For symbols - 
```
<identifier> ->  <initial> <subsequent>* | + | -
<initial>    ->  <letter> | ! | $ | % | & | * | / | : | < | = | > | ? | ~ | _ | ^
<subsequent> ->  <initial> | <digit> | . | + | -
<letter>     ->  a | b | ... | z | A | B | ... | Z
<digit>      ->  0 | 1 | ... | 9 
```
In addition, here are the Scheme primitive functions that are supported at the moment:
```
All mathematical operators (+, -, <, >, =, *, /, %)
and
begin
car
cdr
cond
cons
define
if
lambda
let, let*, letrec
null?
or
quote
set!
```
## Usage
Run `make` in console to compile with the Makefile, then run `.\interpreter < test.scm`. This executes the interpreter on a given Scheme file of code. Due to different line endings that appear across different systems, the interpreter
only recognize Scheme code placed on a single line. For example:
```
(let* ((a b) (b 5)) b)
```
Any result is printed to the console.

## To do
- [ ] A REPL (read-eval-print-loop, allows one to type Scheme code directly in console)
- [ ] Add functionality for more Scheme primitive functions
- [ ] Clean up code in various places

## Contributing
Pull requests are welcome. For major changes, please open an issue first to discuss what you would like to change.
