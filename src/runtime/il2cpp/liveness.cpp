#include "liveness.h"

#include <cstdint>

#include "alloc/general_allocation.h"
#include "utils/hashmap.h"
#include "utils/mem_op.h"
#include "vm/class.h"
#include "vm/field.h"
#include "vm/object.h"
#include "vm/type.h"
#include "vm/rt_array.h"

namespace leanclr
{
namespace il2cpp
{

template <size_t MIN_OBJECT_SIZE, size_t SEGMENT_SIZE_BYTE_COUNT>
class AddressMarker
{
  private:
    static_assert((MIN_OBJECT_SIZE & (MIN_OBJECT_SIZE - 1)) == 0, "MIN_OBJECT_SIZE must be a power of two");
    static_assert((SEGMENT_SIZE_BYTE_COUNT & (SEGMENT_SIZE_BYTE_COUNT - 1)) == 0, "SEGMENT_SIZE_BYTE_COUNT must be a power of two");
    static_assert(SEGMENT_SIZE_BYTE_COUNT % sizeof(size_t) == 0, "SEGMENT_SIZE_BYTE_COUNT must be a multiple of sizeof(size_t)");

    static constexpr size_t kBitsPerWord = sizeof(size_t) * 8;
    static constexpr size_t kSegmentWordCount = SEGMENT_SIZE_BYTE_COUNT / sizeof(size_t);
    static constexpr size_t kSegmentBitCount = kSegmentWordCount * kBitsPerWord;
    static constexpr size_t kInvalidSegmentIndex = static_cast<size_t>(-1);

    utils::HashMap<size_t, size_t*> _segment_map;
    mutable size_t _cached_segment_index = kInvalidSegmentIndex;
    mutable size_t* _cached_segment = nullptr;

    size_t* get_or_create_segment_slow(size_t segment_index)
    {
        auto it = _segment_map.find(segment_index);
        if (it == _segment_map.end())
        {
            size_t* segment = alloc::GeneralAllocation::calloc_any<size_t>(kSegmentWordCount);
            it = _segment_map.emplace(segment_index, segment).first;
        }
        _cached_segment_index = segment_index;
        _cached_segment = it->second;
        return _cached_segment;
    }

  public:
    ~AddressMarker()
    {
        for (auto it = _segment_map.begin(); it != _segment_map.end(); ++it)
        {
            alloc::GeneralAllocation::free(it->second);
        }
    }

    bool mark(void* address)
    {
        const size_t obj_index = reinterpret_cast<uintptr_t>(address) / MIN_OBJECT_SIZE;
        const size_t segment_index = obj_index / kSegmentBitCount;
        // kInvalidSegmentIndex is SIZE_MAX; no real segment_index can collide with it.
        size_t* segment = segment_index == _cached_segment_index ? _cached_segment : get_or_create_segment_slow(segment_index);
        const size_t bit_index_in_segment = obj_index % kSegmentBitCount;
        const size_t word_index = bit_index_in_segment / kBitsPerWord;
        const size_t bit_in_word = bit_index_in_segment % kBitsPerWord;
        const size_t mask = static_cast<size_t>(1) << bit_in_word;
        if (segment[word_index] & mask)
        {
            return false;
        }
        segment[word_index] |= mask;
        return true;
    }
};

using AddressMarkerImpl = AddressMarker<PTR_SIZE * 2, 4 * 1024>;

struct LivenessState
{
    metadata::RtClass* filter;
    il2cpp_register_object_callback callback;
    void* userdata;
    il2cpp_liveness_reallocate_callback reallocate;
    AddressMarkerImpl visited_objects;

    static constexpr size_t PENDING_CALLBACK_BATCH_SIZE = 256;
    vm::RtObject* pending_callback_batch[PENDING_CALLBACK_BATCH_SIZE];
    size_t pending_callback_batch_size;

    LivenessState(metadata::RtClass* filter, uint32_t max_object_count /* unused */, il2cpp_register_object_callback callback, void* userdata,
                  il2cpp_liveness_reallocate_callback reallocate)
        : filter(filter), callback(callback), userdata(userdata), reallocate(reallocate), pending_callback_batch_size(0)
    {
        (void)max_object_count;
        std::memset(pending_callback_batch, 0, sizeof(pending_callback_batch));
    }

    bool add_visited_object(vm::RtObject* obj)
    {
        return visited_objects.mark(obj);
    }

    void add_pending_callback(vm::RtObject* obj)
    {
        if (filter && !vm::Class::has_class_parent_fast(obj->klass, filter))
        {
            return;
        }
        pending_callback_batch[pending_callback_batch_size++] = obj;
        if (pending_callback_batch_size == PENDING_CALLBACK_BATCH_SIZE)
        {
            callback(pending_callback_batch, static_cast<int>(pending_callback_batch_size), userdata);
            pending_callback_batch_size = 0;
        }
    }

    void call_final_callbacks()
    {
        if (pending_callback_batch_size == 0)
        {
            return;
        }
        callback(pending_callback_batch, static_cast<int>(pending_callback_batch_size), userdata);
        pending_callback_batch_size = 0;
    }

    void finalize()
    {
    }
};

void* Liveness::allocate_struct(metadata::RtClass* filter, uint32_t max_object_count, il2cpp_register_object_callback callback, void* userdata,
                                il2cpp_liveness_reallocate_callback reallocate)
{
    if (filter)
    {
        auto ret = vm::Class::initialize_all(filter);
        if (ret.is_err())
        {
            assert(false && "Failed to initialize filter class");
            panic("Failed to initialize filter class");
            return nullptr;
        }
    }
    return new LivenessState(filter, max_object_count, callback, userdata, reallocate);
}

void Liveness::free_struct(void* state)
{
    delete reinterpret_cast<LivenessState*>(state);
}

static void process_object(vm::RtObject* obj, LivenessState* state);

static void visit_gc_bitmap(const metadata::RtClass* klass, uint8_t* slot_base, LivenessState* state)
{
    const size_t bit_count = klass->gc_bitmap_bit_count;
    const size_t start_bit = vm::Class::kFirstGCBitmapBitIndex;
    if (bit_count <= start_bit)
    {
        return;
    }

    const size_t* bitmap = klass->gc_bitmap;
    const size_t kBitsPerWord = vm::Class::kBitsPerWord;
    const size_t start_word = start_bit / kBitsPerWord;
    const size_t end_word = (bit_count + kBitsPerWord - 1) / kBitsPerWord;

    for (size_t w = start_word; w < end_word; ++w)
    {
        size_t word = bitmap[w];
        while (word != 0)
        {
            const size_t bit_in_word = utils::MemOp::count_trailing_zeros_nonzero(word);
            const size_t bit_index = w * kBitsPerWord + bit_in_word;
            vm::RtObject** slot = reinterpret_cast<vm::RtObject**>(slot_base + bit_index * sizeof(void*));
            process_object(*slot, state);
            word &= word - 1;
        }
    }
}

static void visit_normal_object(vm::RtObject* obj, LivenessState* state)
{
    visit_gc_bitmap(obj->klass, reinterpret_cast<uint8_t*>(obj), state);
}

static void visit_value_type(uint8_t* data, LivenessState* state, const metadata::RtClass* value_type_class)
{
    visit_gc_bitmap(value_type_class, data - vm::RT_OBJECT_HEADER_SIZE, state);
}

static void visit_array_object(vm::RtArray* obj, LivenessState* state)
{
    const metadata::RtClass* element_class = obj->klass->element_class;
    if (!vm::Class::get_has_references(element_class))
    {
        return;
    }
    if (vm::Class::is_reference_type(element_class))
    {
        vm::RtObject** elements = vm::Array::get_array_data_start_as<vm::RtObject*>(obj);
        for (int32_t i = 0; i < obj->length; ++i)
        {
            process_object(elements[i], state);
        }
    }
    else
    {
        size_t element_size = vm::Array::get_array_element_size(obj);
        void* elements_start_address = vm::Array::get_array_data_start_as_ptr_void(obj);
        for (size_t i = 0, n = static_cast<size_t>(obj->length); i < n; ++i)
        {
            uint8_t* element_address = reinterpret_cast<uint8_t*>(elements_start_address) + i * element_size;
            visit_value_type(element_address, state, element_class);
        }
    }
}

static void process_object(vm::RtObject* obj, LivenessState* state)
{
    if (!obj)
    {
        return;
    }
    if (!state->add_visited_object(obj))
    {
        return;
    }
    state->add_pending_callback(obj);
    if (vm::Class::is_array_or_szarray(obj->klass))
    {
        visit_array_object(reinterpret_cast<vm::RtArray*>(obj), state);
    }
    else
    {
        visit_normal_object(obj, state);
    }
}

void Liveness::calculation_from_root(vm::RtObject* root, void* state)
{
    auto liveness_state = reinterpret_cast<LivenessState*>(state);
    process_object(root, liveness_state);
    liveness_state->call_final_callbacks();
}

void Liveness::calculation_from_statics(void* state)
{
    auto liveness_state = reinterpret_cast<LivenessState*>(state);
    auto& all_classes_with_static_data = vm::Class::get_all_classes_with_static_data();
    for (const metadata::RtClass* klass : all_classes_with_static_data)
    {
        for (uint16_t i = 0; i < klass->field_count; ++i)
        {
            const metadata::RtFieldInfo* field = klass->fields + i;
            if (!vm::Field::is_static_excluded_literal_and_rva(field))
            {
                continue;
            }
            const metadata::RtTypeSig* type_sig = field->type_sig;
            if (type_sig->is_by_ref())
            {
                continue;
            }
            switch (type_sig->ele_type)
            {
            case metadata::RtElementType::Boolean:
            case metadata::RtElementType::Char:
            case metadata::RtElementType::I1:
            case metadata::RtElementType::U1:
            case metadata::RtElementType::I2:
            case metadata::RtElementType::U2:
            case metadata::RtElementType::I4:
            case metadata::RtElementType::U4:
            case metadata::RtElementType::I8:
            case metadata::RtElementType::U8:
            case metadata::RtElementType::R4:
            case metadata::RtElementType::R8:
            case metadata::RtElementType::I:
            case metadata::RtElementType::U:
            case metadata::RtElementType::Ptr:
            case metadata::RtElementType::FnPtr:
            case metadata::RtElementType::TypedByRef:
            case metadata::RtElementType::Var:
            case metadata::RtElementType::MVar:
            {
                break;
            }
            case metadata::RtElementType::Object:
            case metadata::RtElementType::String:
            case metadata::RtElementType::Class:
            case metadata::RtElementType::SZArray:
            case metadata::RtElementType::Array:
            handle_reference_type_field:
            {
                assert(field->offset % sizeof(void*) == 0);
                vm::RtObject* obj = *reinterpret_cast<vm::RtObject**>(reinterpret_cast<uint8_t*>(klass->static_fields_data) + field->offset);
                process_object(obj, liveness_state);
                break;
            }
            case metadata::RtElementType::GenericInst:
            {
                bool is_ref_type = vm::Type::is_reference_type(type_sig).unwrap();
                if (is_ref_type)
                {
                    goto handle_reference_type_field;
                }
                // fall through
            }
            case metadata::RtElementType::ValueType:
            {
                metadata::RtClass* field_class = vm::Class::get_class_from_typesig(type_sig).unwrap();
                assert(vm::Class::has_initialized_part(field_class, metadata::RtClassInitPart::Field));
                if (!vm::Class::get_has_references(field_class))
                {
                    break;
                }
                assert(field->offset % sizeof(void*) == 0);
                uint8_t* cur_field_address = reinterpret_cast<uint8_t*>(klass->static_fields_data) + field->offset;
                visit_value_type(cur_field_address, liveness_state, field_class);
                break;
            }
            default:
            {
                assert(false && "invalid element type");
            }
            }
        }
    }
    liveness_state->call_final_callbacks();
}

void Liveness::finalize(void* state)
{
    reinterpret_cast<LivenessState*>(state)->finalize();
}
} // namespace il2cpp
} // namespace leanclr