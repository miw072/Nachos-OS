#include <syscall.h>

int
main()
{
    Exec("../test/pipe1", 0, 0, 2);
    Exec("../test/pipe2", 0, 0, 3);
    Exec("../test/pipe3", 0, 0, 4);
    Exit(123);
}
