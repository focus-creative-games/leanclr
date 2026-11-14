#include "alloc/general_allocation.h"
#include "utils/hashmap.h"
#include "utils/hashset.h"
#include "vm/gchandle.h"
#include "vm/class.h"
#include "metadata/pdb_image.h"
#include "utils/rt_vector.h"
#include <cstdlib>
#include <vector>
using namespace leanclr;
static alloc::MemoryCallbacks original;
static bool fail;
static size_t allocations;

static void check(bool b)
{
    if (!b)
        std::abort();
}

static void* allocate(size_t n)
{
    if (fail)
        return nullptr;
    ++allocations;
    return original.malloc(n);
}

static void* zero(size_t n, size_t s)
{
    if (fail)
        return nullptr;
    ++allocations;
    return original.calloc(n, s);
}

int main()
{
    original = alloc::GeneralAllocation::get_memory_callbacks();
    auto cb = original;
    cb.malloc = allocate;
    cb.calloc = zero;
    check(alloc::GeneralAllocation::set_memory_callbacks(cb));
    {
        utils::Vector<int> values;
        const size_t before_empty = allocations;
        values.push_range(nullptr, 0);
        utils::Vector<int> empty_copy(values);
        check(values.empty() && values.data() == nullptr && empty_copy.empty());
        check(allocations == before_empty);
        values.push_back(7);
        const size_t before_nonempty = allocations;
        values.push_range(nullptr, 0);
        check(values.size() == 1 && values[0] == 7 && allocations == before_nonempty);
        const int more[] = {11, 13};
        values.push_range(more, 2);
        check(values.size() == 3 && values[1] == 11 && values[2] == 13);
    }
    {
        metadata::RtClass empty{};
        size_t bytes = 42;
        vm::Class::get_gc_bitmap(&empty, nullptr, bytes);
        check(bytes == 0);
        size_t bits = 5, copied = 0;
        empty.gc_bitmap = &bits;
        empty.gc_bitmap_word_count = 1;
        vm::Class::get_gc_bitmap(&empty, &copied, bytes);
        check(bytes == sizeof(bits) && copied == bits);
    }
    {
        alloc::MemPool pool;
        const uint8_t image[] = {0};
        metadata::PdbImage pdb(pool, image, sizeof(image));
        metadata::RtMethodInfo methods[2]{};
        metadata::MethodIR2ILMapInfo empty{{nullptr, 0}};
        pdb.add_ir2il_map_for_method(&methods[0], empty);
        metadata::IR2ILMapEntry entry{3, 7};
        metadata::MethodIR2ILMapInfo populated{{&entry, 1}};
        pdb.add_ir2il_map_for_method(&methods[1], populated);
    }

    {
        utils::HashMap<int, int> map;
        utils::HashSet<int> set;
        size_t before = allocations;
        for (int i = 0; i < 100; ++i)
        {
            map[i] = i * 3;
            set.insert(i);
        }
        check(allocations > before && map[67] == 201 && set.count(99) == 1);
    }
    std::vector<void*> handles;
    std::vector<vm::GCHandleId> ids;
    for (int i = 0; i < 12; ++i)
    {
        auto* h = vm::GCHandle::new_handle(nullptr, false);
        check(h);
        handles.push_back(h);
        ids.push_back(vm::GCHandle::get_handle_id(h));
    }
    fail = true;
    check(!vm::GCHandle::new_handle(nullptr, false));
    for (size_t i = 0; i < handles.size(); ++i)
        check(vm::GCHandle::get_handle_by_id(ids[i]) == handles[i]);
    fail = false;
    for (size_t i = 0; i < 6; ++i)
    {
        vm::GCHandle::free_handle(handles[i]);
        check(!vm::GCHandle::get_handle_by_id(ids[i]));
    }
    fail = true;
    check(!vm::GCHandle::new_handle(nullptr, false));
    fail = false;
    for (size_t i = 0; i < 6; ++i)
    {
        handles[i] = vm::GCHandle::new_handle(nullptr, false);
        check(handles[i] && vm::GCHandle::get_handle_id(handles[i]) > ids.back());
    }
    for (int round = 0; round < 200; ++round)
    {
        for (auto* h : handles)
            vm::GCHandle::free_handle(h);
        for (auto& h : handles)
        {
            h = vm::GCHandle::new_handle(nullptr, false);
            check(h);
        }
    }
    for (auto* h : handles)
        vm::GCHandle::free_handle(h);
    return 0;
}
