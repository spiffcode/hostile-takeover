#include "game/ht.h"
#include "base/messagequeue.h"

// C++ support

#if 0
void *operator new(size_t cb)
{
    void *pv = malloc(cb);
    return pv;
}

void operator delete(void *pv)
{
    if (pv != 0)
        free(pv);
}

void *operator new[](size_t cb)
{
    void *pv = malloc(cb);
    return pv;
}

void operator delete[](void *pv)
{
    if (pv != 0)
        free(pv);
}

extern "C" void __cxa_pure_virtual(void)
{
    IPhone::Log("pure virtual method called\n");
}
#endif

int main(int argc, char **argv)
{
    return wi::IPhone::main(argc, argv);
}
