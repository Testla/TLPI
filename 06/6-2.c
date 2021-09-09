/*
Functions f() and g() are intentionally made simliar
so that the position of x will hopefully be the same.

When compiled with -O0, it prints:

setjmp, x = 0
f() = 0
Returned from longjmp, x = 1
g() = 1

f() did returned successfully using the x and return address of g().

When compiled with -O1, it prints:

setjmp, x = 0
f() = 0
Returned from longjmp, x = 0
Segmentation fault (core dumped)

1. In f(), x is replaced by 0 and not stored on stack.
So it prints x = 0 after longjmp().
2. g() is inlined into main.
When f() "return"s the second time,
it returns to a random address, causing segfault.

When compiled with -O2 or -O3, it prints:

setjmp, x = 0
f() = 0
Returned from longjmp, x = 0
setjmp, x = 0
f() = 0
Returned from longjmp, x = 0
setjmp, x = 0
f() = 0
...

Similar to -O1, this time it happens to return to somewhere before _start
and the program enters an infinite loop.
*/
#include <setjmp.h>
#include <stdio.h>

static jmp_buf env;

static int f(void) {
    int x = 0;
    if (setjmp(env) == 0) {
        printf("setjmp, x = %d\n", x);
    } else {
        printf("Returned from longjmp, x = %d\n", x);
    }
    return x;
}

static int g(void) {
    __attribute__((__unused__)) int x = 1;
    longjmp(env, 3);
    return 2;
}

int main(void) {
    printf("f() = %d\n", f());
    printf("g() = %d\n", g());
    return 0;
}
