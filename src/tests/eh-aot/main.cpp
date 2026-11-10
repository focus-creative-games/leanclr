#include "alloc/general_allocation.h"
#include "metadata/metadata_cache.h"
#include "metadata/module_def.h"
#include "vm/assembly.h"
#include "vm/class.h"
#include "vm/method.h"
#include "vm/runtime.h"
#include "vm/settings.h"
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <string>
#include <chrono>
#include <vector>
#include "interp/interpreter.h"
#include "interp/machine_state.h"

using namespace leanclr;
extern metadata::RtAotModulesData g_aot_modules_data;
static std::string dlls;

static void check(bool ok, const char* message)
{
    if (!ok)
    {
        std::fprintf(stderr, "%s\n", message);
        std::exit(1);
    }
}

static bool load(const char* name, const char* ext, vm::FileData& data)
{
    std::ifstream file(dlls + "/" + name + "." + ext, std::ios::binary | std::ios::ate);
    if (!file)
        return false;
    const auto length = file.tellg();
    if (length < 0)
        return false;
    auto* bytes = static_cast<uint8_t*>(alloc::GeneralAllocation::malloc(static_cast<size_t>(length)));
    if (!bytes)
        return false;
    file.seekg(0);
    if (!file.read(reinterpret_cast<char*>(bytes), length))
    {
        alloc::GeneralAllocation::free(bytes);
        return false;
    }
    data = {bytes, static_cast<size_t>(length), false};
    return true;
}

static uint32_t invoke(const metadata::RtMethodInfo* method, const void** args)
{
    const auto old_top = interp::MachineState::get_global_machine_state().get_frame_stack_top();
    auto result = vm::Runtime::invoke_with_run_cctor(method, nullptr, args);
    check(result.is_ok() && result.unwrap(), "EH fixture invocation");
    check(interp::MachineState::get_global_machine_state().get_frame_stack_top() == old_top, "handoff leaked interpreter frames");
    return *reinterpret_cast<const uint32_t*>(result.unwrap() + 1);
}

int main(int argc, char** argv)
{
    check(argc >= 2, "DLL directory required");
    dlls = argv[1];
    vm::Settings::set_file_loader(load);
    vm::Settings::set_aot_modules_data(&g_aot_modules_data);
    check(vm::Runtime::initialize().is_ok(), "initialize runtime");
    auto assembly = vm::Assembly::load_by_name("EhFixtures");
    check(assembly.is_ok(), "load EH fixtures");
    auto type = assembly.unwrap()->mod->get_class_by_name("EhFixtures", false, false);
    check(type.is_ok() && type.unwrap(), "EH fixture class");
    auto* klass = type.unwrap();
    check(vm::Class::initialize_all(klass).is_ok(), "initialize EH fixtures");
    const char* compiled[] = {"Catch", "MutatedArguments", "Nested", "FinallyThrows", "ReturnFinally", "CalleeFailure", "Rethrow",
                              "Loop",  "ReferenceLocal",   "Void",   "Floating",      "Wide",          "NullThrow"};
    for (const char* name : compiled)
    {
        const auto* method = vm::Method::find_matched_method_in_class_by_name(klass, name);
        check(method && method->invoker_type == metadata::RtInvokerType::Aot, name);
    }
    const char* fallback[] = {"AliasFallback", "ComplexFinallyFallback"};
    for (const char* name : fallback)
    {
        const auto* method = vm::Method::find_matched_method_in_class_by_name(klass, name);
        check(method && method->invoker_type == metadata::RtInvokerType::Interpreter, "unsafe handoff method must remain interpreted");
    }
    const auto* run = vm::Method::find_matched_method_in_class_by_name(klass, "Run");
    for (int i = 0; i < 4; ++i)
    {
        const uint32_t code = invoke(run, nullptr);
        if (code)
            std::fprintf(stderr, "EH behavior failure: %u\n", code);
        check(code == 0, "EH side effects, snapshots, finally and rethrow");
    }
    const auto* normal = vm::Method::find_matched_method_in_class_by_name(klass, "NormalLoop");
    check(normal && normal->interp_data == nullptr, "no eager interpreter translation for normal path");
    int n = argc > 2 ? std::atoi(argv[2]) : 100000;
    int divisor = 7;
    const void* args[] = {&n, &divisor};
    invoke(normal, args);
    const auto start = std::chrono::steady_clock::now();
    const uint32_t checksum = invoke(normal, args);
    const auto end = std::chrono::steady_clock::now();
    if (normal->invoker_type == metadata::RtInvokerType::Aot)
        check(normal->interp_data == nullptr, "non-throwing path must stay AOT");
    const double ns = std::chrono::duration<double, std::nano>(end - start).count() / n;
    std::printf("{\"workload\":\"eh-normal\",\"execution\":\"%s\",\"checksum\":\"%u\",\"ns_per_op\":%.6f}\n",
                normal->invoker_type == metadata::RtInvokerType::Aot ? "aot" : "interpreter", checksum, ns);
    auto initialized =
        normal->interp_data ? RtResult<const interp::RtInterpMethodInfo*>(normal->interp_data) : interp::Interpreter::init_interpreter_method(normal);
    check(initialized.is_ok(), "interpreter metadata for invalid handoff probe");
    const auto* info = initialized.unwrap();
    std::vector<interp::RtStackObject> invalid(info->total_arg_and_local_stack_object_size);
    auto bad = interp::Interpreter::resume_exception(normal, invalid.data(), invalid.size(), UINT32_MAX, RtErr::DivideByZero);
    check(bad.is_err() && bad.unwrap_err() == RtErr::InvalidOperation, "invalid IL handoff must fail");
    auto filter_assembly = vm::Assembly::load_by_name("FilterFixture");
    check(filter_assembly.is_ok(), "load filter fixture");
    auto filter_type = filter_assembly.unwrap()->mod->get_class_by_name("FilterFixture", false, false);
    check(filter_type.is_ok() && vm::Class::initialize_all(filter_type.unwrap()).is_ok(), "initialize filter fixture");
    const auto* filter = vm::Method::find_matched_method_in_class_by_name(filter_type.unwrap(), "Test");
    check(filter && filter->invoker_type == metadata::RtInvokerType::Interpreter, "filter must stay interpreted");
    int32_t filter_value = 0;
    const void* filter_args[] = {&filter_value};
    check(invoke(filter, filter_args) == 2, "interpreted exception filter");
    filter_value = 1;
    check(invoke(filter, filter_args) == 1, "interpreted filter success path");
    std::puts("EH normal-path handoff checks passed");
}
