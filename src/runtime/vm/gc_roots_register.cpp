#include "gc_roots_register.h"

namespace leanclr
{
namespace vm
{

void register_exception_gc_roots();
void register_string_gc_roots();
void register_appdomain_gc_roots();
void register_environment_gc_roots();
void register_reflection_gc_roots();

void register_all_gc_roots()
{
    register_exception_gc_roots();
    register_string_gc_roots();
    register_appdomain_gc_roots();
    register_environment_gc_roots();
    register_reflection_gc_roots();
}

} // namespace vm
} // namespace leanclr
