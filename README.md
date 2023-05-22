# Simple Compiler
## Table of contents
- [Description](#description)
- [Requirements](#requirements)
- [How to run](#how-to-run)
- [Capabilities](#capabilities)
    - [datatypes](#datatypes)
    - [enum types](#enum-types)
    - [constant decleration](#constant-declaration)
    - [if-else statement](#if-else-statement)
    - [switch statement](#switch-statement)
    - [loops](#loops)
        - [for loop](#for-loop)
        - [while loop](#while-loop)
        - [repeat-until loop](#repeat-until-loop)
    - [functions](#functions)

## Description
A simple C-like compiler built using Flex and Bison.

[(return to top)](#simple-compiler)
## Requirements
- Linux (Ubuntu, ...etc)
- GCC
- Flex
- Bison

[(return to top)](#simple-compiler)
## How to run
- Install `gcc`
- Install `Flex` using `sudo apt-get install flex`
- Install `Bison` using `sudo apt-get install bison` 
- Run `make all` in the project folder
- Type the code in a text file (input.txt) then run `./run.out < input.txt`

[(return to top)](#simple-compiler)
## Capabilities
### datatypes
```c
int integer_number = 5;
float float_number = 10.0;
char character = 'c';
string str = "hello world";
bool istrue = true;
```

[(return to top)](#simple-compiler)
### enum types
- Must be defined in the global scope
- Defined values must start with `$`
```c
enum enum_type = {$hello, $goodbye};
enum enum_type x = $hello;
```

[(return to top)](#simple-compiler)
### constant declaration
```c
const int integer_number = 5;
const float float_number = 10.0;
char character = 'c';
const string str = "hello world";
const bool istrue = true;
```

[(return to top)](#simple-compiler)
### if-else statement
```c
int x = 0;
int y;
if (x >= 0 && x < 5)
{
    y = 1;
} else if (x >= 5 && x < 10)
{
    y = 2;
}
else
{
    y = 3;
}
```

[(return to top)](#simple-compiler)
### switch statement
- Works with all datatypes
```c
int x = 5;
switch(x)
{
    case 1:
    break;
    default:
    break;
}
```

[(return to top)](#simple-compiler)
### loops
#### for loop
```c
for (int i = 0; i < 5; i = i + 1)
{
}
```

[(return to top)](#simple-compiler)
#### while loop
```c
int x = 5;
while (x <= 10)
{
    i = i + 1;
}
```

[(return to top)](#simple-compiler)
#### repeat-until loop
```c
int x = 5;
repeat {
 x = x + 1;
} until(x <= 10);
```

[(return to top)](#simple-compiler)
### functions
- Must be defined in the global scope
- Must start with keyword `func`
- Can be of type `int`, `float`, `char`, `string`, `bool`, and `void`
- All non-void functions must have a return value
```c
func int sum (int x, int y)
{
    int result = x + y;
    return result;
}
```

[(return to top)](#simple-compiler)