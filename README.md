#ccollect
Tool used to collect structs and functions info from c/c++ sources to JSON.

##usage
```
ccolect example.c -I/your/include/path \
                  -D_AND_OTHER_COMPILER_OPTIONS_ > output.json
```

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
