import file_region_replacer
import opcode_spec_parser


def gen_low_level_opcode_enum(opcodes):
    return "\n".join(f"    {op.name} = {op.code}," for op in opcodes)

def get_type_cpp_type_name(type_name):
    match type_name:
        case "u8":
            return "uint8_t"
        case "i8":
            return "int8_t"
        case "u16":
            return "uint16_t"
        case "i16":
            return "int16_t"
        case "u32":
            return "uint32_t"
        case "i32":
            return "int32_t"
        case "u64":
            return "uint64_t"
        case "i64":
            return "int64_t"
        case "f32":
            return "float"
        case "f64":
            return "double"
        case _:
            raise NotImplementedError(f"Type {type_name!r} is not supported")

def gen_low_level_opcode_structs(low_level_opcodes):
    lines = []
    for opcode in low_level_opcodes:
        lines.append(f"struct {opcode.name}")
        lines.append("{")
        lines.append("    uint16_t __code;")
        for param in opcode.params:
            lines.append(f"    {get_type_cpp_type_name(param.type)} {param.name};")
        lines.append("};")
        lines.append("")
    return "\n".join(lines)


def gen_low_level_opcode_size(low_level_opcodes):
    lines = []
    padding = "    "
    for opcode in low_level_opcodes:
        lines.append(f"{padding}sizeof({opcode.name}),")
    return "\n".join(lines)

def gen_low_level_opcode_write_to_data(low_level_opcodes):
    lines = []
    padding = "    "
    for opcode in low_level_opcodes:
        lines.append(f"{padding}case OpCodeEnum::{opcode.name}:")
        lines.append(f"{padding}{{")
        lines.append(f"{padding}    auto ir = ({opcode.name}*)codes;")
        lines.append(f"{padding}    ir->__code = static_cast<uint16_t>(OpCodeEnum::{opcode.name});")
        for param in opcode.params:
            if param.name.startswith("__padding_"):
                continue
            if param.arg_kind == "stack":
                lines.append(f"{padding}    ir->{param.name} = ({get_type_cpp_type_name(param.type)})inst.get_var_{param.arg}_eval_stack_idx();")
            else:
                lines.append(f"{padding}    ir->{param.name} = ({get_type_cpp_type_name(param.type)})inst.get_{param.arg}();")
        match opcode.name:
            case "Switch":
                lines.append(f"{padding}    auto targetsInfo = inst.get_switch_targets();")
                lines.append(f"{padding}    int32_t self_ir_offset = inst.get_ir_offset();")
                lines.append(f"{padding}    int32_t* target_offsets = (int32_t*)(ir + 1);")
                lines.append(f"{padding}    for (int i = 0; i < targetsInfo.count; i++)")
                lines.append(f"{padding}    {{")
                lines.append(f"{padding}        target_offsets[i] = (int32_t)targetsInfo.targets[i]->ir_offset - self_ir_offset;")
                lines.append(f"{padding}    }}")
                lines.append(f"{padding}    return codes + sizeof({opcode.name}) + (targetsInfo.count * sizeof(int32_t));")
            case _:
                lines.append(f"{padding}    return codes + sizeof({opcode.name});")
        lines.append(f"{padding}}}")
    return "\n".join(lines)


def gen_computed_goto_labels_region(opcodes):
    return "\n".join(["    static void* const instruction_labels[] = {"] +
                     [f"        &&LABEL_{op.name}," for op in opcodes] + ["    };"])


if __name__ == "__main__":
    import sys
    if len(sys.argv) != 5:
        sys.exit("Usage: gen_low_level_opcodes.py <opcodes.xml> <ll_opcodes.h> <ll_opcodes.cpp> <interpreter.cpp>")
    xml, header, source, interpreter = sys.argv[1:]
    opcodes = opcode_spec_parser.parse_low_level_opcode_file(xml)
    h = file_region_replacer.FileRegionReplacer(header)
    h.replace_region("LOW_LEVEL_OPCODE_ENUM", gen_low_level_opcode_enum(opcodes))
    h.replace_region("LOW_LEVEL_INSTRUCTION_STRUCTS", gen_low_level_opcode_structs(opcodes))
    h.save()
    cpp = file_region_replacer.FileRegionReplacer(source)
    cpp.replace_region("LOW_LEVEL_INSTRUCTION_SIZES", gen_low_level_opcode_size(opcodes))
    cpp.replace_region("LOW_LEVEL_INSTRUCTION_WRITE_TO_DATA", gen_low_level_opcode_write_to_data(opcodes))
    cpp.save()
    interp = file_region_replacer.FileRegionReplacer(interpreter)
    interp.replace_region("COMPUTED_GOTO_LABELS", gen_computed_goto_labels_region(opcodes))
    interp.save()
