from pathlib import Path

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

STACK_FIELDS = {"arg1": "first", "src": "first", "arg2": "second", "arg3": "third", "dst": "destination"}
VALUE_FIELDS = {'element_size': 'element_size', 'finally_clauses_count': 'finally_count', 'i2': 'static_cast<int16_t>(immediate)', 'i4': 'static_cast<int32_t>(immediate)', 'i8_high': 'static_cast<uint32_t>(immediate >> 32)', 'i8_low': 'static_cast<uint32_t>(immediate)', 'num_targets': 'switch_targets.count', 'size': 'size', 'total_params_stack_object_size': 'parameter_size', 'first_finally_clause_index': 'first_finally', 'field_offset': 'field_offset', 'field_size': 'field_size', 'invoker_idx': 'invoker_index', 'resolved_data_index': 'resolved_index', 'frame_base': 'frame_base', 'locals_offset': 'locals_offset', 'branch_target_offset': '0'}

def operand_expression(param):
    if param.arg_kind == "stack":
        return f"operands.{STACK_FIELDS[param.arg]}->eval_stack_offset"
    expr = VALUE_FIELDS[param.arg]
    if expr == "0":
        return expr
    if expr.startswith("static_cast"):
        return expr.replace("(immediate", "(operands.immediate")
    return "operands." + expr

def gen_low_level_opcode_write_to_data(opcodes):
    lines = []
    for op in opcodes:
        lines += [f"    case OpCodeEnum::{op.name}:", "    {", f"        auto* ir = reinterpret_cast<{op.name}*>(codes);",
                  "        ir->__code = static_cast<uint16_t>(opcode);"]
        for param in op.params:
            if param.name.startswith("__padding_"):
                continue
            expr = operand_expression(param)
            lines.append(f"        ir->{param.name} = static_cast<{get_type_cpp_type_name(param.type)}>({expr});")
        lines += [f"        return codes + sizeof({op.name});", "    }"]
    return "\n".join(lines)

def gen_branch_patch_offsets(opcodes):
    lines = []
    for op in opcodes:
        for param in op.params:
            if param.arg_kind == "target":
                lines.append(f"    case OpCodeEnum::{op.name}: return offsetof({op.name}, {param.name});")
    return "\n".join(lines)

def gen_operand_validation(opcodes):
    lines = []
    for op in opcodes:
        conditions = []
        for param in op.params:
            if param.name.startswith("__padding_") or param.arg_kind == "target":
                continue
            expr = operand_expression(param)
            if param.arg_kind == "stack":
                conditions.append(f"operands.{STACK_FIELDS[param.arg]} != nullptr")
            # Immediate bit patterns are selected by the opcode-specific emitter.
            # Frame slots, indices and sizes must never be silently truncated.
            if param.type.startswith("u") and param.arg not in ("i2", "i4", "i8_low", "i8_high"):
                conditions.append(f"{expr} <= UINT{param.type[1:]}_MAX")
        condition = " && ".join(dict.fromkeys(conditions)) or "true"
        lines += [f"    case OpCodeEnum::{op.name}:", f"        return {condition};"]
    return "\n".join(lines)

def gen_computed_goto_labels_region(opcodes):
    return "\n".join(["    static void* const instruction_labels[] = {"] +
                     [f"        &&LABEL_{op.name}," for op in opcodes] + ["    };"])


def generate_includes(opcodes, output_dir):
    definitions = (
        "enum class OpCodeEnum : uint16_t\n{\n"
        + gen_low_level_opcode_enum(opcodes)
        + "\n    __Count,\n};\n\n"
        + gen_low_level_opcode_structs(opcodes)
    )
    implementation = (
        "size_t OpCodes::s_opsizes[static_cast<size_t>(OpCodeEnum::__Count)] = {\n"
        + gen_low_level_opcode_size(opcodes)
        + "\n};\n\n"
        + "uint8_t* OpCodes::write_instruction_to_data(uint8_t* codes, OpCodeEnum opcode, const InstructionOperands& operands)\n"
        + "{\n    switch (opcode)\n    {\n"
        + gen_low_level_opcode_write_to_data(opcodes)
        + '\n    default:\n        assert(false && "Unhandled opcode in write_instruction_to_data");\n        return nullptr;\n    }\n}\n\n'
        + "size_t OpCodes::get_branch_patch_offset(OpCodeEnum opcode)\n{\n    switch (opcode)\n    {\n"
        + gen_branch_patch_offsets(opcodes)
        + "\n    default: return static_cast<size_t>(-1);\n    }\n}\n\n"
        + "bool OpCodes::validate_operands(OpCodeEnum opcode, const InstructionOperands& operands)\n{\n    switch (opcode)\n    {\n"
        + gen_operand_validation(opcodes)
        + "\n    default: return false;\n    }\n}\n"
    )
    outputs = {
        "ll_opcodes_defs.inc": definitions,
        "ll_opcodes_impl.inc": implementation,
        "interpreter_dispatch.inc": gen_computed_goto_labels_region(opcodes),
    }
    output_dir = Path(output_dir)
    output_dir.mkdir(parents=True, exist_ok=True)
    notice = "// Generated by src/generator/gen_low_level_opcodes.py from ll-opcodes.xml. Do not edit.\n\n"
    for name, body in outputs.items():
        path = output_dir / name
        content = notice + body.rstrip() + "\n"
        # Unchanged output should not force recompilation of the interpreter.
        if not path.exists() or path.read_text(encoding="utf-8") != content:
            path.write_text(content, encoding="utf-8", newline="\n")


if __name__ == "__main__":
    import sys
    if len(sys.argv) != 3:
        sys.exit("Usage: gen_low_level_opcodes.py <opcodes.xml> <output_directory>")
    xml, output_dir = sys.argv[1:]
    opcodes = opcode_spec_parser.parse_low_level_opcode_file(xml)
    generate_includes(opcodes, output_dir)
