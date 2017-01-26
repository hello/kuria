/**
 * @file
 *
 * @brief Overloaded new and delete operators
 */

#include <cstdlib>
//#include <iostream>
//#include <string>
#include "xt_new_delete.h"
#include "FreeRTOS.h"
#include "task.h"

using namespace std;

void* operator new(size_t size) {
    void *p = pvPortMalloc(size);
//    if (!p)
//        throw "operator new() error";
    return p;
}

void* operator new[](size_t size) {
    void *p = pvPortMalloc(size);
//    if (!p)
//        throw "operator new() error";
    return p;
}

void operator delete(void *p) {
    vPortFree(p);
}

void operator delete[](void *p) {
    vPortFree(p);
}


/*
void* operator new (std::size_t size) throw (std::bad_alloc) {
    void *p = pvPortMalloc(size);
    totalForNew += size;
    return p;
}

void* operator new (std::size_t size, const std::nothrow_t& nothrow_constant) throw() {
    return pvPortMalloc(size);
}

void* operator new[] (std::size_t size) throw (std::bad_alloc) {
    void *p = pvPortMalloc(size);
    return p;
}

void* operator new[] (std::size_t size, const std::nothrow_t& nothrow_constant) throw() {
    return pvPortMalloc(size);
}

void operator delete (void* ptr) throw () {
    vPortFree(ptr);
}

void operator delete (void* ptr, const std::nothrow_t& nothrow_constant) throw() {
     vPortFree(ptr);
}

void operator delete[] (void* ptr) throw () {
    vPortFree(ptr);
}

void operator delete[] (void* ptr, const std::nothrow_t& nothrow_constant) throw() {
    vPortFree(ptr);
}
*/
