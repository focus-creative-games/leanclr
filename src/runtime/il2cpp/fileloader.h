#pragma once

#include <vector>
#include <string>
#include <fstream>
#include "alloc/general_allocation.h"
#include "vm/rt_exception.h"
#include "vm/settings.h"

using namespace leanclr;

namespace leanclr
{
namespace il2cpp
{
RtResult<vm::FileData> assembly_file_loader(const char* assembly_name, const char* extension);
}

} // namespace leanclr