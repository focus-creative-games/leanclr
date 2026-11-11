#include "vm/gchandle.h"
#include "vm/reflection.h"
#include <cstdlib>
#include <vector>
using namespace leanclr;

static void check(bool value)
{
    if (!value)
        std::abort();
}

struct Marks
{
    std::vector<vm::RtObject*> objects;
};

static bool marked(vm::RtObject* object, void* data)
{
    for (auto* candidate : static_cast<Marks*>(data)->objects)
        if (candidate == object)
            return true;
    return false;
}

static void mark(vm::RtObject* object, void* data)
{
    if (!marked(object, data))
        static_cast<Marks*>(data)->objects.push_back(object);
}

int main()
{
    for (uintptr_t invalid : {uintptr_t{1}, uintptr_t{3}, ~uintptr_t{0}})
    {
        const auto* handle = reinterpret_cast<const void*>(invalid);
        check(vm::Reflection::get_type_sig_from_net10_type_handle(handle).is_err());
        check(vm::Reflection::get_class_from_net10_method_table(handle).is_err());
        check(vm::Reflection::get_runtime_type_from_handle_arg(handle).is_err());
        check(vm::Reflection::get_method_info_from_handle_arg(handle).is_err());
        check(vm::Reflection::get_field_info_from_handle_arg(handle).is_err());
        check(vm::Reflection::get_property_info_from_handle_arg(handle).is_err());
        check(vm::Reflection::get_event_info_from_handle_arg(handle).is_err());
    }
    // Stack objects model a graph; callbacks exercise the same ephemeron API
    // used by GcRoots, without depending on a particular CoreLib installation.
    vm::RtObject a{}, b{}, c{}, dead_key{}, dead_value{};
    void* first = vm::GCHandle::new_dependent_handle(&a, &b);
    void* second = vm::GCHandle::new_dependent_handle(&b, &c);
    void* cycle = vm::GCHandle::new_dependent_handle(&dead_key, &dead_value);
    void* back = vm::GCHandle::new_dependent_handle(&dead_value, &dead_key);
    check(first && second && cycle && back);
    Marks roots;
    vm::GCHandle::foreach_strong_handles(mark, &roots);
    check(roots.objects.empty());
    mark(&a, &roots);
    size_t iterations = 0;
    while (vm::GCHandle::visit_dependent_handles(marked, mark, &roots))
        check(++iterations <= 3);
    check(marked(&b, &roots) && marked(&c, &roots));
    check(!marked(&dead_key, &roots) && !marked(&dead_value, &roots));
    vm::GCHandle::sweep_weak_handles(marked, &roots, false);
    check(vm::GCHandle::get_dependent_handle_target(cycle) == nullptr);
    check(vm::GCHandle::get_dependent_handle_dependent(cycle) == nullptr);
    check(vm::GCHandle::get_dependent_handle_dependent(first) == &b);
    auto** slot = vm::GCHandle::get_target_slot(first);
    std::vector<void*> handles;
    for (int i = 0; i < 2048; ++i)
    {
        void* handle = vm::GCHandle::new_handle(&a, false);
        check(handle != nullptr);
        handles.push_back(handle);
    }
    check(*slot == &a && vm::GCHandle::get_handle_by_target_slot(slot) == first);
    for (void* handle : handles)
        vm::GCHandle::free_handle(handle);
    vm::GCHandle::set_dependent_handle_target_to_null(first);
    vm::GCHandle::set_dependent_handle_dependent(first, &c);
    check(vm::GCHandle::get_dependent_handle_dependent(first) == nullptr);
    check(vm::GCHandle::free_dependent_handle(first));
    check(vm::GCHandle::free_dependent_handle(second));
    check(vm::GCHandle::free_dependent_handle(cycle));
    check(vm::GCHandle::free_dependent_handle(back));
    check(vm::GCHandle::get_handle_by_target_slot(slot) == nullptr);
    return 0;
}
