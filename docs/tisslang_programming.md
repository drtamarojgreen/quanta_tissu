# TissLang Programming Reference

---

## 1. Data Structures

### Lists (Bracket-Free)

- Start with `TTLS` and end with `SLTT`
- Items separated by spaces or new lines

Example:

List myList = TTLS 1 2 3 4 SLTT
List names = TTLS Alice Bob Charlie SLTT

- `List myList = ...`: Defines a list named `myList` containing the integers 1, 2, 3, and 4.
- `List names = ...`: Defines a list named `names` containing the strings "Alice", "Bob", and "Charlie".


---

### Maps / Dictionaries

- Use `TTLS` and `SLTT` to start/end
- Key-value pairs separated by `->`
- Can be multiline or single line

Example:

Map user = TTLS
name -> Alice
age -> 30
active -> true
SLTT


or

Map user = TTLS name->Alice age->30 active->true SLTT

- The first example defines a map named `user` with three key-value pairs: `name` is "Alice", `age` is 30, and `active` is true. The definition is spread across multiple lines for readability.
- The second example shows the same map defined on a single line.


---

### Tuples

- Fixed length, ordered values

Example:

Tuple point = TTLS 10 20 SLTT

- `Tuple point = ...`: Defines a tuple named `point` containing the ordered values 10 and 20.


---

### Error Codes

Named constants for error handling:

Error = E_NOT_FOUND = 404
Error = E_PERMISSION_DENIED = 403

- `Error = E_NOT_FOUND = 404`: Defines a named error constant `E_NOT_FOUND` with the integer value 404.


---

## 2. Switch Statements

### Syntax

Switch variable = <variable_name>
Cases = TTLS
<case_value1> -> <action1>
<case_value2> -> <action2>
...
SLTT
Default = <default_action>


### Example

Switch variable = status
Cases = TTLS
200 -> Print Success
404 -> Print Not Found
500 -> Print Server Error
SLTT
Default = Print Unknown Status

- This example evaluates the `status` variable. If `status` is 200, it prints "Success". If it's 404, it prints "Not Found". If no case matches, it executes the `Default` action and prints "Unknown Status".


---

## 3. Function Definitions

### Syntax

Function <function_name> Params = TTLS <param1> <param2> ... SLTT
Body =
<instruction_1>
<instruction_2>
...


### Example

Function greet Params = TTLS name SLTT
Body =
Print Hello, name

- This defines a function named `greet` that accepts a single parameter, `name`.
- The body of the function contains one instruction: to print a string that includes the value of the `name` parameter.


Quick inline example:

Function add Params = TTLS x y SLTT Body = TTLS Return x + y SLTT
- This shows a more compact, single-line function definition for `add` that takes two parameters, `x` and `y`, and returns their sum.


---

## 4. Import Commands

### Syntax

Single import:

Import <module_name>


Multiple imports:

Import TTLS
module1
module2
SLTT

- `Import <module_name>`: Loads a single module into the current scope.
- `Import TTLS ... SLTT`: Loads multiple specified modules (`module1`, `module2`) at once.


---

## 5. Variable Definitions

### Syntax

Var <variable_name> = <value>


or with structured data:

Var myList = TTLS 1 2 3 4 SLTT

Var user = TTLS
name->Alice
age->30
active->true
SLTT

- `Var <variable_name> = <value>`: The basic syntax for assigning a value to a variable.
- `Var myList = ...`: Assigns a newly created list to the variable `myList`.
- `Var user = ...`: Assigns a newly created map to the variable `user`.


---

## 6. Indentation Dependence

- Indentation defines block boundaries (functions, conditionals, switch cases)
- Increase indent to start block, reduce to close
- Use consistent spaces or tabs (not mixed)
- Works with TTLS/SLTT tokens for lists and complex blocks

Example:

Function greet Params = TTLS name SLTT
Body =
Print Hello, name
If name == "Alice" T
Print Welcome back, Alice!
Else T
Print Nice to meet you

- This `greet` function includes a conditional. The `If` block is executed only if `name` is "Alice".
- The `T` token appears to mark the beginning of a code block.
- The indented line `Print Welcome back, Alice!` belongs to the `If` block. The `Else` block has its own indented line.


---

## 7. Fibonacci Function Example

Function fibonacci Params = TTLS n SLTT
Body =
If n <= 1 T
Return n
Else T
Return fibonacci(n - 1) + fibonacci(n - 2)

- This example implements the classic recursive Fibonacci sequence.
- `If n <= 1 T`: This is the base case. If the input `n` is 1 or 0, the function returns `n`.
- `Else T`: This is the recursive step. The function calls itself with `n - 1` and `n - 2` and returns the sum of their results.

---

This document captures your current TissLang syntax ideas in a concise format for reference and future expansion.
