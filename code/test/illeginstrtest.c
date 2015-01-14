#include "syscall.h"
void
func()
{
    Exit(123);
}
main()
{
    int a = 9;
    void(*func)(void) = &a;
    func();
    return 0;
}
