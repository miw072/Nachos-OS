#include "syscall.h"

int
main()
{
    int result = 111;
    result = Exec("../test/reuseTest1", 0, 0, 0);
    Exit(result);
}
