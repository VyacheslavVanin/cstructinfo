# cstructinfo
Tool used to collect structs and functions info from c/c++ sources to JSON.

Collectable data:
- Functions:
  - name
  - prameters (names and types)
  - return type
  - comments (in Doxygen format):
    - function description
    - parameters description
    - return description
    
- Structs:
  - name
  - fields:
    - name
    - type
    - array size (if array)
    - bitfield width (if bitfield)
    - sizeof of primitive types
  - methods (same as functions)
  - comments (in Doxygen format)
    

## Usage
Use tool like this:
```
cstructinfo example.c -I/your/include/path \
                  -D_AND_OTHER_COMPILER_OPTIONS_ > output.json
```

Do not separate compiler options. For example
```
cstructinfo example.c -I /your/include/path \
                  -D _AND_OTHER_COMPILER_OPTIONS_ > output.json
```
this do not work.

## Parameters
- --main-only - do not write any structs or functions from included files output, only from files specified.
- --no-structs - do not write any struct info to output. Do not create "structs" section.
- --no-functions - do not write any function info to output. Do not create "functions" section.
- --no-sizes - do not add sizeofs of primitive types to structs descriptions (remove "builtin" field)
- --help - show help


## Example
Input file example.cpp:
```
#include <stdio.h>
#include <stdlib.h>

typedef int myint;

typedef struct foo
{
    /// Comment about a
    int a;
    int b; ///< Comment about b
    int c;
    myint d;
}foo;

/*!
 * It is foo2 description
 * */
typedef struct foo2
{
    size_t numFoos;
    foo foos[10][20][42];
}foo2;

typedef struct bitfieldExample
{
    unsigned value1;
    unsigned bit1   :3; 
    unsigned bit2   :6; 
    unsigned bit3   :23;
}bitfieldExample;

/**
 * @brief Print foo contents.
 * @param f struct to print
 * @return Your return message
 */
int bar(const foo* f)
{
    printf("foo contains: a = %d, b =%d, c = %d\n", f->a, f->b, f->c);
    return 0;
}
```
Command:
```
cstructinfo example.c
```

Output:
```
{
    "structs": [
        {
            "name": "foo",
            "comment": "",
            "fields": [
                {
                    "field": "a",
                    "type": "int",
                    "comment": "Comment about a",
                    "builtin": "32"
                },
                {
                    "field": "b",
                    "type": "int",
                    "comment": "Comment about b",
                    "builtin": "32"
                },
                {
                    "field": "c",
                    "type": "int",
                    "comment": "",
                    "builtin": "32"
                },
                {
                    "field": "d",
                    "type": "myint",
                    "comment": "",
                    "builtin": "32"
                }
            ],
            "methods": [
                {
                    "name": "method_example",
                    "rettype": "int",
                    "retcomment": "sum of foo.a and parameter g",
                    "comment": "test method description",
                    "params": [
                        {
                            "param": "g",
                            "type": "int",
                            "comment": "method parameter doc"
                        }
                    ]
                }
            ]
        },
        {
            "name": "foo2",
            "comment": "It is foo2 description",
            "fields": [
                {
                    "field": "numFoos",
                    "type": "int",
                    "comment": "",
                    "builtin": "32"
                },
                {
                    "field": "foos",
                    "type": "foo [10][20][42]",
                    "comment": "",
                    "array": {
                        "elemType": "foo",
                        "size": [
                            "10",
                            "20",
                            "42"
                        ]
                    }
                }
            ]
        },
        {
            "name": "bitfieldExample",
            "comment": "",
            "fields": [
                {
                    "field": "value1",
                    "type": "unsigned int",
                    "comment": "",
                    "builtin": "32"
                },
                {
                    "field": "bit1",
                    "type": "unsigned int",
                    "comment": "",
                    "bitfieldWidth": "3",
                    "builtin": "32"
                },
                {
                    "field": "bit2",
                    "type": "unsigned int",
                    "comment": "",
                    "bitfieldWidth": "6",
                    "builtin": "32"
                },
                {
                    "field": "bit3",
                    "type": "unsigned int",
                    "comment": "",
                    "bitfieldWidth": "23",
                    "builtin": "32"
                }
            ]
        }
    ],
    "functions": [
        {
            "name": "bar",
            "rettype": "int",
            "retcomment": "Your return message",
            "comment": "Print foo contents.",
            "params": [
                {
                    "param": "f",
                    "type": "const foo *",
                    "comment": "struct to print"
                }
            ]
        }
    ]
}
```

## Build
- depends from libclang version >=3.8, llvm version >=3.8, ncurses
  (debian: libclang-dev, llvm-dev, libncursesw5-dev)


## On errors
If error messages appear on standard headers (stddef.h, stdargs.h etc)
you can specify COMPILER_PATH of your compiler for example:

```
./cstructinfo example.c -I/usr/lib/clang/3.8.1/include/
```
or
```
./cstructinfo example.c -I/usr/lib/gcc/x86_64-pc-linux-gnu/6.1.1/include
```

You can know what path exactly you need by typing command:
```
echo | gcc  -E -v -
```
or
```
echo | clang  -E -v -
```
see section "#include <...> search starts here:"
