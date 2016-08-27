#ccollect
Tool used to collect structs and functions info from c/c++ sources to JSON.

##usage
Use tool like this:
```
ccolect example.c -I/your/include/path \
                  -D_AND_OTHER_COMPILER_OPTIONS_ > output.json
```

Do not separate compiler options. For example 
```
ccolect example.c -I /your/include/path \
                  -D _AND_OTHER_COMPILER_OPTIONS_ > output.json
```
this do not work.


##build
- depends from libclang-3.5, llvm-3.5, ncurses
  (debian: libclang-dev, llvm-dev, libncursesw5-dev)

##example
Input file example.c:
```
#include <stdio.h>
#include <stdlib.h>

typedef struct foo
{
    /// Comment about a
    int a;
    int b; ///< Comment about b
    int c;
}foo;

/*!
 * It is foo2 description
 * */
typedef struct foo2
{
    size_t numFoos;
    foo foos[10];
}foo2;

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
ccollect example.c
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
                    "comment": "Comment about a"
                },
                {
                    "field": "b",
                    "type": "int",
                    "comment": "Comment about b"
                },
                {
                    "field": "c",
                    "type": "int",
                    "comment": ""
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
                    "comment": ""
                },
                {
                    "field": "foos",
                    "type": "foo [10]",
                    "comment": "",
                    "array": {
                        "elemType": "foo",
                        "elemCount": "10"
                    }
                }
            ]
        }
    ],
    "functions": [
        {
            "name": "bar",
            "retval": "int",
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

##on errors
If error messages appear on standard headers (stddef.h, stdargs.h etc) 
you can specify COMPILER_PATH of your compiler for example:

```
./ccollect example.c -I/usr/lib/clang/3.5.2/include/
```
or 
```
./ccollect example.c -I/usr/lib/gcc/x86_64-pc-linux-gnu/6.1.1/include
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

