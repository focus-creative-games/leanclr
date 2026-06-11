#pragma once

#include <vector>
#include <string>
#include "alloc/general_allocation.h"
#include "vm/rt_exception.h"
#include "vm/settings.h"

using namespace leanclr;

namespace leanclr
{
namespace il2cpp
{
bool assembly_file_loader(const char* assembly_name, const char* extension, vm::FileData& file_data);
}

} // namespace leanclr