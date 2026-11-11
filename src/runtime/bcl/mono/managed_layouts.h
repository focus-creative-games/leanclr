#pragma once

#include "vm/rt_managed_types.h"

// Private Mono 4.5 managed ABI. Runtime core code must not include this file.
// The vm namespace preserves source compatibility for adapter/codegen clients.
namespace leanclr
{
namespace vm
{

struct RtMarshalByRefObject : public RtObject
{
    RtObject* identity;
};

struct RtReflectionType
{
    RtObject header;
    const metadata::RtTypeSig* type_handle;
};

struct RtReflectionRuntimeType
{
    RtReflectionType reflection_type;
    RtObject* type_info;
    RtObject* generic_cache;
    RtObject* serialization_ctor;
};

struct RtReflectionField
{
    RtObject header;
    const metadata::RtClass* klass;
    const metadata::RtFieldInfo* field;
    RtString* name;
    RtReflectionType* type_;
    uint32_t attrs;
};

struct RtReflectionMethod
{
    RtObject header;
    const metadata::RtMethodInfo* method;
    RtString* name;
    RtReflectionType* ref_type;
};

struct RtMonoMethodInfo
{
    RtReflectionType* parent;
    RtReflectionType* return_type;
    uint32_t attrs;
    uint32_t impl_attrs;
    uint32_t call_conv;
};

struct RtMonoPropertyInfo
{
    RtReflectionType* parent;
    RtReflectionType* declaring_type;
    RtString* name;
    RtReflectionMethod* get_method;
    RtReflectionMethod* set_method;
    uint32_t attrs;
};

struct RtReflectionProperty : public RtObject
{
    const metadata::RtClass* klass;
    const metadata::RtPropertyInfo* property;
    RtMonoPropertyInfo info;
    uint32_t cached;
    RtObject* cached_get;
};

struct RtReflectionEvent : public RtObject
{
    RtObject* cached_add_event;
};

struct RtReflectionEventInfo
{
    RtReflectionEvent reflection_event;
    RtReflectionType* ref_type;
    metadata::RtEventInfo* event;
};

struct RtReflectionParameter : public RtObject
{
    uint32_t attrs;
    RtReflectionType* parent_type;
    RtObject* default_value;
    RtObject* member;
    RtString* name;
    int32_t index;
    RtObject* marshaling_info;
};

struct RtReflectionModule : public RtObject
{
    const void* image; // EEImage*
    RtReflectionAssembly* assembly;
    RtString* fqname;
    RtString* name;
    RtString* scope_name;
    bool is_resource;
    uint32_t token;
};

struct RtReflectionAssembly : public RtObject
{
    metadata::RtAssembly* assembly;
    RtObject* evidence;
    RtObject* resolve_event_holder;
    RtObject* minimum;
    RtObject* optional;
    RtObject* refused;
    RtObject* granted;
    RtObject* denied;
    bool from_byte_array;
    RtString* name;
};

struct RtReflectionAssemblyName : public RtObject
{
    RtString* name;
    RtString* code_base;
    int32_t major;
    int32_t minor;
    int32_t build;
    int32_t revision;
    RtString* culture_info;
    uint32_t flags;
    uint32_t hash_alg;
    RtObject* key_pair;
    RtArray* public_key;
    RtArray* public_key_token;
    uint32_t version_compat;
    RtObject* version;
    uint32_t processor_architecture;
    uint32_t content_type;
};

struct RtCustomAttribute : public RtObject
{
};

struct RtDelegateData : public RtObject
{
    RtReflectionType* target_type;
    RtString* method_name;
    bool curried_first_arg;
};

struct RtDelegate : public RtObject
{
    uintptr_t _method_ptr;
    uintptr_t invoke_impl;
    RtObject* target;
    const metadata::RtMethodInfo* method;
    uintptr_t _delegate_trampoline;
    intptr_t extra_arg;
    uintptr_t method_code;
    uintptr_t _interp_method;
    metadata::RtManagedMethodPointer interp_invoke_impl;
    const metadata::RtMethodInfo* method_info;
    const metadata::RtMethodInfo* original_method_info;
    RtDelegateData* data;
    bool method_is_virtual;
};

struct RtMulticastDelegate
{
    RtDelegate dele;
    RtArray* deles;
};

struct RtAppDomain : public RtMarshalByRefObject
{
    RtMonoAppDomain* mono_app_domain;
    RtObject* evidence;
    RtObject* granted;
    int32_t principal_policy;
    RtObject* assembly_load;
    RtObject* assembly_resolve;
    RtObject* domain_unload;
    RtObject* process_exit;
    RtObject* resource_resolve;
    RtObject* type_resolve;
    RtObject* unhandled_exception;
    RtObject* first_chance_exception;
    RtObject* domain_manager;
    RtObject* reflection_only_assembly_resolve;
    RtObject* activation;
    RtObject* application_identity;
    RtObject* compatibility_switch;
};

struct RtAppContext
{
    RtObject object_header;
    int32_t domain_id;
    int32_t context_id;
    uint8_t* static_data;
    uintptr_t data;
    RtObject* server_context_sink_chain;
    RtObject* client_context_sink_chain;
    RtObject* context_properties;
    RtObject* local_data_store;
    RtObject* context_dynamic_properties;
    RtObject* callback_object;
};

struct RtMonoAppDomain
{
    RtAppDomain* appdomain;
    RtObject* setup;
    RtAppContext* context;
    RtObject* ephemeron_tombstone;
    const char* friendly_name;
    int32_t domain_id;
    int32_t threadpool_job_counter;
    void* agent_info;
};

struct RtException : public RtObject
{
    RtString* class_name;
    RtString* message;
    RtObject* data;
    RtException* inner_exception;
    RtString* help_url;
    RtArray* trace_ips;
    RtString* stack_trace;
    RtString* remote_stack_trace;
    int32_t remote_stack_index;
    RtObject* dynamic_methods;
    int32_t hresult;
    RtString* source;
    RtObject* safe_serialization_manager;
    RtArray* captured_traces;
    RtArray* native_trace_ips;
    int32_t caught_in_unmanaged;
};

struct RtCultureData : public RtObject
{
    RtString* sm1159;
    RtString* pm2359;
    RtString* time_separator;
    RtArray* long_times;
    RtArray* short_times;
    int32_t first_day_of_week;
    int32_t first_week_of_year;
    RtArray* calendars;
    RtArray* calendar_datas;
    RtString* iso639_language;
    RtString* real_name;
    bool use_overrides;
    int32_t calendar_id;
    int32_t number_index;
    int32_t default_ansi_code_page;
    int32_t default_oem_code_page;
    int32_t default_mac_code_page;
    int32_t default_ebcdic_code_page;
    bool is_right_to_left;
    RtString* list_sep;
};

struct RtCalendarData : public RtObject
{
    RtString* native_calendar_name;
    RtArray* short_date_patterns;
    RtArray* year_month_patterns;
    RtArray* long_date_patterns;
    RtString* month_day;
    RtArray* era_names;
    RtArray* abbreviated_era_names;
    RtArray* abbreviated_english_era_names;
    RtArray* day_names;
    RtArray* abbreviated_day_names;
    RtArray* super_short_day_names;
    RtArray* month_names;
    RtArray* abbreviated_month_names;
    RtArray* month_genitive_names;
    RtArray* abbreviated_month_genitive_names;
    RtArray* leap_year_month_names;
    int32_t two_digit_year_max;
    int32_t current_era;
    bool use_user_overrides;
};

struct RtDateTimeFormatInfo : public RtObject
{
    RtObject* culture_data;
    RtString* name;
    RtString* lang_name;
    RtObject* compare_info;
    RtObject* culture_info;
    RtString* am_designator;
    RtString* pm_designator;
    RtString* date_separator;
    RtString* general_short_time_pattern;
    RtString* general_long_time_pattern;
    RtString* time_separator;
    RtString* month_day_pattern;
    RtString* date_time_offset_pattern;
    RtObject* calendar;
    uint32_t first_day_of_week;
    uint32_t calendar_week_rule;
    RtString* full_date_time_pattern;
    RtArray* abbreviated_day_names;
    RtArray* short_day_names;
    RtArray* day_names;
    RtArray* abbreviated_month_names;
    RtArray* month_names;
    RtArray* genitive_month_names;
    RtArray* genitive_abbreviated_month_names;
    RtArray* leap_year_month_names;
    RtString* long_date_pattern;
    RtString* short_date_pattern;
    RtString* year_month_pattern;
    RtString* long_time_pattern;
    RtString* short_time_pattern;
    RtArray* year_month_patterns;
    RtArray* short_date_patterns;
    RtArray* long_date_patterns;
    RtArray* short_time_patterns;
    RtArray* long_time_patterns;
    RtArray* era_names;
    RtArray* abbrev_era_names;
    RtArray* abbrev_english_era_names;
    RtArray* optional_calendars;
    bool read_only;
    int32_t format_flags;
    RtString* full_time_span_positive_pattern;
    RtString* full_time_span_negative_pattern;
    RtArray* dtfi_token_hash;
};

struct RtNumberFormatInfo : public RtObject
{
    RtArray* number_group_sizes;
    RtArray* currency_group_sizes;
    RtArray* percent_group_sizes;
    RtString* positive_sign;
    RtString* negative_sign;
    RtString* number_decimal_separator;
    RtString* number_group_separator;
    RtString* currency_group_separator;
    RtString* currency_decimal_separator;
    RtString* currency_symbol;
    RtString* ansi_currency_symbol;
    RtString* nan_symbol;
    RtString* positive_infinity_symbol;
    RtString* negative_infinity_symbol;
    RtString* percent_decimal_separator;
    RtString* percent_group_separator;
    RtString* percent_symbol;
    RtString* per_mille_symbol;
    RtArray* native_digits;
    int32_t data_item;
    int32_t number_decimal_digits;
    int32_t currency_decimal_digits;
    int32_t currency_positive_pattern;
    int32_t currency_negative_pattern;
    int32_t number_negative_pattern;
    int32_t percent_positive_pattern;
    int32_t percent_negative_pattern;
    int32_t percent_decimal_digits;
    int32_t digit_substitution;
    bool read_only;
    bool use_user_override;
    bool is_invariant;
    bool valid_for_parse_as_number;
    bool valid_for_parse_as_currency;
};

struct RtCultureInfo : public RtObject
{
    bool is_read_only;
    int32_t lcid;
    int32_t parent_lcid;
    int32_t datetime_index;
    int32_t number_index;
    int32_t default_calendar_type;
    bool use_user_override;
    RtNumberFormatInfo* number_format;
    RtDateTimeFormatInfo* datetime_format;
    RtObject* textinfo;
    RtString* name;
    RtString* englishname;
    RtString* nativename;
    RtString* iso3lang;
    RtString* iso2lang;
    RtString* win3lang;
    RtString* territory;
    RtArray* native_calendar_names;
    RtString* compareinfo;
    const char* text_info_data;
    int32_t data_item;
    RtObject* calendar;
    RtObject* parent_culture;
    bool constructed;
    RtArray* cached_serialized_form;
    RtObject* culture_data;
    bool is_inherited;
};

struct RtRegionInfo : public RtObject
{
    int32_t geo_id;
    RtString* iso2name;
    RtString* iso3name;
    RtString* win3name;
    RtString* english_name;
    RtString* native_name;
    RtString* currency_symbol;
    RtString* iso_currency_symbol;
    RtString* currency_english_name;
    RtString* currency_native_name;
};

struct RtStackFrame : public RtObject
{
    int32_t il_offset;
    int32_t native_offset;
    uint64_t method_address;
    uint32_t method_index;
    RtReflectionMethod* method;
    RtString* filename;
    int32_t line;
    int32_t column;
    RtString* internal_method_name;
};

struct RtReflectionExceptionHandlingClause : public RtObject
{
    RtReflectionType* catch_type;
    int32_t filter_offset;
    int32_t flags;
    int32_t try_offset;
    int32_t try_length;
    int32_t handler_offset;
    int32_t handler_length;
};

struct RtReflectionLocalVariableInfo : public RtObject
{
    RtReflectionType* type_;
    bool is_pinned;
    uint16_t position;
};

struct RtReflectionMethodBody : public RtObject
{
    RtArray* clauses;
    RtArray* locals;
    RtArray* codes;
    bool init_locals;
    int32_t sig_token;
    int32_t max_stack;
};

struct RtReflectionMonoEventInfo
{
    RtReflectionType* declaring_type;
    RtReflectionType* ref_type;
    RtString* name;
    RtReflectionMethod* add_method;
    RtReflectionMethod* remove_method;
    RtReflectionMethod* raise_method;
    uint16_t attrs;
    RtArray* other_methods;
};

struct RtNativeThread
{
};

struct RtInternalThread : public RtObject
{
    int32_t lock_thread_id;
    RtNativeThread* handle;
    void* native_handle;
    const Utf16Char* name_chars;
    int32_t name_free;
    int32_t name_length;
    RtThreadState state;
    RtException* abort_exc;
    int32_t abort_state_handle;
    int64_t thread_id;
    void* debugger_thread;
    void* static_data;
    void* runtime_thread_info;
    RtObject* current_appcontext;
    RtObject* root_domain_thread;
    RtArray* serialized_principal; // byte[]
    int32_t serialized_principal_version;
    void* appdomain_refs;
    int32_t interruption_requested;
    void* long_lived;
    bool threadpool_thread;
    bool thread_interrupt_requested;
    int32_t stack_size;
    uint8_t apartment_state;
    int32_t critical_region_level;
    int32_t managed_id;
    int32_t small_id;
    void* manage_callback;
    void* flags;
    void* thread_pinning_ref;
    void* abort_protected_block_count;
    int32_t priority;
    void* owned_mutexes;
    void* suspended_event;
    int32_t self_suspended;
    void* thread_state;
    void* netcore0;
    void* netcore1;
    void* netcore2;
    void* last;
};

struct RtThread : public RtObject
{
    RtInternalThread* internal_thread;
    RtObject* thread_start_arg;
    RtException* pending_exception;
    RtMulticastDelegate* delegate;
    RtObject* execution_context;
    bool execution_context_belongs_to_current_scope;
    RtObject* principal;
    int32_t principal_version;
};

static_assert(alignof(RtMarshalByRefObject) >= core::pointer_alignment<RtMarshalByRefObject>::value, "Opaque result pointer alignment must match ABI");
static_assert(alignof(RtReflectionType) >= core::pointer_alignment<RtReflectionType>::value, "Opaque result pointer alignment must match ABI");
static_assert(alignof(RtReflectionRuntimeType) >= core::pointer_alignment<RtReflectionRuntimeType>::value, "Opaque result pointer alignment must match ABI");
static_assert(alignof(RtReflectionField) >= core::pointer_alignment<RtReflectionField>::value, "Opaque result pointer alignment must match ABI");
static_assert(alignof(RtReflectionMethod) >= core::pointer_alignment<RtReflectionMethod>::value, "Opaque result pointer alignment must match ABI");
static_assert(alignof(RtMonoMethodInfo) >= core::pointer_alignment<RtMonoMethodInfo>::value, "Opaque result pointer alignment must match ABI");
static_assert(alignof(RtMonoPropertyInfo) >= core::pointer_alignment<RtMonoPropertyInfo>::value, "Opaque result pointer alignment must match ABI");
static_assert(alignof(RtReflectionProperty) >= core::pointer_alignment<RtReflectionProperty>::value, "Opaque result pointer alignment must match ABI");
static_assert(alignof(RtReflectionEvent) >= core::pointer_alignment<RtReflectionEvent>::value, "Opaque result pointer alignment must match ABI");
static_assert(alignof(RtReflectionEventInfo) >= core::pointer_alignment<RtReflectionEventInfo>::value, "Opaque result pointer alignment must match ABI");
static_assert(alignof(RtReflectionParameter) >= core::pointer_alignment<RtReflectionParameter>::value, "Opaque result pointer alignment must match ABI");
static_assert(alignof(RtReflectionModule) >= core::pointer_alignment<RtReflectionModule>::value, "Opaque result pointer alignment must match ABI");
static_assert(alignof(RtReflectionAssembly) >= core::pointer_alignment<RtReflectionAssembly>::value, "Opaque result pointer alignment must match ABI");
static_assert(alignof(RtReflectionAssemblyName) >= core::pointer_alignment<RtReflectionAssemblyName>::value, "Opaque result pointer alignment must match ABI");
static_assert(alignof(RtCustomAttribute) >= core::pointer_alignment<RtCustomAttribute>::value, "Opaque result pointer alignment must match ABI");
static_assert(alignof(RtDelegateData) >= core::pointer_alignment<RtDelegateData>::value, "Opaque result pointer alignment must match ABI");
static_assert(alignof(RtDelegate) >= core::pointer_alignment<RtDelegate>::value, "Opaque result pointer alignment must match ABI");
static_assert(alignof(RtMulticastDelegate) >= core::pointer_alignment<RtMulticastDelegate>::value, "Opaque result pointer alignment must match ABI");
static_assert(alignof(RtAppDomain) >= core::pointer_alignment<RtAppDomain>::value, "Opaque result pointer alignment must match ABI");
static_assert(alignof(RtAppContext) >= core::pointer_alignment<RtAppContext>::value, "Opaque result pointer alignment must match ABI");
static_assert(alignof(RtMonoAppDomain) >= core::pointer_alignment<RtMonoAppDomain>::value, "Opaque result pointer alignment must match ABI");
static_assert(alignof(RtException) >= core::pointer_alignment<RtException>::value, "Opaque result pointer alignment must match ABI");
static_assert(alignof(RtCultureData) >= core::pointer_alignment<RtCultureData>::value, "Opaque result pointer alignment must match ABI");
static_assert(alignof(RtCalendarData) >= core::pointer_alignment<RtCalendarData>::value, "Opaque result pointer alignment must match ABI");
static_assert(alignof(RtDateTimeFormatInfo) >= core::pointer_alignment<RtDateTimeFormatInfo>::value, "Opaque result pointer alignment must match ABI");
static_assert(alignof(RtNumberFormatInfo) >= core::pointer_alignment<RtNumberFormatInfo>::value, "Opaque result pointer alignment must match ABI");
static_assert(alignof(RtCultureInfo) >= core::pointer_alignment<RtCultureInfo>::value, "Opaque result pointer alignment must match ABI");
static_assert(alignof(RtRegionInfo) >= core::pointer_alignment<RtRegionInfo>::value, "Opaque result pointer alignment must match ABI");
static_assert(alignof(RtStackFrame) >= core::pointer_alignment<RtStackFrame>::value, "Opaque result pointer alignment must match ABI");
static_assert(alignof(RtReflectionExceptionHandlingClause) >= core::pointer_alignment<RtReflectionExceptionHandlingClause>::value,
              "Opaque result pointer alignment must match ABI");
static_assert(alignof(RtReflectionLocalVariableInfo) >= core::pointer_alignment<RtReflectionLocalVariableInfo>::value,
              "Opaque result pointer alignment must match ABI");
static_assert(alignof(RtReflectionMethodBody) >= core::pointer_alignment<RtReflectionMethodBody>::value, "Opaque result pointer alignment must match ABI");
static_assert(alignof(RtReflectionMonoEventInfo) >= core::pointer_alignment<RtReflectionMonoEventInfo>::value,
              "Opaque result pointer alignment must match ABI");
static_assert(alignof(RtNativeThread) >= core::pointer_alignment<RtNativeThread>::value, "Opaque result pointer alignment must match ABI");
static_assert(alignof(RtInternalThread) >= core::pointer_alignment<RtInternalThread>::value, "Opaque result pointer alignment must match ABI");
static_assert(alignof(RtThread) >= core::pointer_alignment<RtThread>::value, "Opaque result pointer alignment must match ABI");
} // namespace vm
} // namespace leanclr
