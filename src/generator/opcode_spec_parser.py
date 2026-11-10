import xml.etree.ElementTree as ET

class AddressMode:
    NORMAL = 2
    LARGE = 3

class CliOpCode:
    def __init__(self, name, name2, args, o1, o2, flow, type, constant):
        self.name = name
        self.name2 = name2
        self.args = args
        self.o1 = o1
        self.o2 = o2
        self.flow = flow
        self.type = type
        self.constant = constant

def parse_cli_opcodes(xml_root):
    opcodes = []
    for opcode_elem in xml_root.findall('opcode'):
        name = opcode_elem.get('name')
        name2 = opcode_elem.get('name2')
        args = opcode_elem.get('args')
        o1 = int(opcode_elem.get('o1'), 16)
        o2 = int(opcode_elem.get('o2'), 16)
        flow = opcode_elem.get('flow')
        type = opcode_elem.get('type')
        constant_str = opcode_elem.get('constant', None)
        constant = int(constant_str, 16) if constant_str is not None else None
        opcode = CliOpCode(name, name2, args, o1, o2, flow, type, constant)
        opcodes.append(opcode)

    return opcodes

def parse_cli_opcode_file(file_path):
    tree = ET.parse(file_path)
    root = tree.getroot()
    return parse_cli_opcodes(root)

class OpcodeParam:
    def __init__(self, name, type, arg, arg_kind, addr_mode=None):
        self.name = name
        self.type = type
        self.arg = arg
        self.arg_kind = arg_kind
        self.addr_mode = addr_mode

    def override_addr_mode(self, addr_mode):
        if self.addr_mode is None:
            self.addr_mode = addr_mode
    def address_type(self, normal_type, large_type=None):
        return large_type if self.addr_mode == AddressMode.LARGE and large_type else normal_type
    def compile(self):
        if self.type is None:
            match self.arg_kind:
                case "size":
                    self.type = "u16"
                case "stack" | "stack_const" | "resolved_data" | "invoker_idx":
                    self.type = "u16"
                case "aot_invoker_idx":
                    self.type = "u32"
                case "target":
                    self.type = "i32"
                case "const":
                    self.type = "u32"
                case "field_offset":
                    self.type = self.address_type("u16", "u32")
                case "exception_clause_index":
                    self.type = "u8"
                case _:
                    raise NotImplementedError(f"arg_kind {self.arg_kind!r} is not supported")

class LowLevelOpcodeTemplate:
    def __init__(self, name, params):
        self.name = name
        self.params = params

    def clone_params(self):
        return [OpcodeParam(p.name, p.type, p.arg, p.arg_kind, p.addr_mode) for p in self.params]

def get_type_size(type_str):
    match type_str:
        case "u8" | "i8":
            return 1
        case "u16" | "i16":
            return 2
        case "u32" | "i32" | "f32":
            return 4
        case "u64" | "i64" | "f64":
            return 8
        case _:
            raise NotImplementedError(f"type {type_str!r} is not supported")

class LowLevelOpcode:
    def __init__(self, name, params, variant, addr_mode):
        self.name = name
        self.params = params
        self.variant = variant
        self.addr_mode = addr_mode

    def set_code(self, code):
        self.code = code
    def compact_arrange_params(self):
        # Arrange params: first stack params, then const params
        self.params.sort(key=lambda p: get_type_size(p.type))
        total_size = 2  # uint16_t opcode
        arranged_params = []
        for param in self.params:
            param_size = get_type_size(param.type)
            if total_size % param_size != 0:
                # Add padding param
                for k in range(param_size - (total_size % param_size)):
                    padding_param = OpcodeParam(f"__padding_{total_size}", "u8", None, None, None)
                    arranged_params.append(padding_param)
                    total_size += 1
                assert total_size % param_size == 0
            arranged_params.append(param)
            total_size += param_size
        if total_size % 4 != 0:
            # Add padding to align to 4 bytes
            for k in range(4 - (total_size % 4)):
                padding_param = OpcodeParam(f"__padding_{total_size}", "u8", None, None, None)
                arranged_params.append(padding_param)
                total_size += 1
            assert total_size % 4 == 0
        self.params = arranged_params
    def compile(self):
        if self.addr_mode is not None:
            for param in self.params:
                param.override_addr_mode(self.addr_mode)
        for param in self.params:
            param.compile()
        self.compact_arrange_params()

def parse_low_level_opcode_params(opcode_elem, params):
    for param_elem in opcode_elem.findall('param'):
        name = param_elem.get('name')
        type = param_elem.get('type')
        arg = param_elem.get('arg')
        arg_kind = param_elem.get('arg_kind')
        addr_mode_str = param_elem.get('addr_mode')
        addr_mode = int(addr_mode_str) if addr_mode_str is not None else None
        param = OpcodeParam(name, type, arg, arg_kind, addr_mode)
        params.append(param)

def compile_low_level_opcodes(low_level_opcodes):
    if len(low_level_opcodes) > 65535:
        raise ValueError("Opcode count exceeds the uint16_t instruction space")
    if len({op.name for op in low_level_opcodes}) != len(low_level_opcodes):
        raise ValueError("Duplicate instruction name")
    for code, opcode in enumerate(low_level_opcodes):
        opcode.set_code(code)
        opcode.compile()
    return low_level_opcodes

def parse_low_level_opcodes(xml_root):
    low_level_templates = {}
    for opcode_elem in xml_root.findall('tplopcode'):
        name = opcode_elem.get('name')
        params = []
        parse_low_level_opcode_params(opcode_elem, params)
        variant = opcode_elem.get('variant')
        low_level_templates[name] = LowLevelOpcodeTemplate(name, params)

    low_level_opcodes = []
    for opcode_elem in xml_root.findall('opcode'):
        name = opcode_elem.get('name')
        if name.endswith('_S'):
            raise ValueError("Short instruction variants are no longer supported")
        if name.endswith('_L'):
            name = name[:-2] + "Large"
            addr_mode = AddressMode.LARGE
        else:
            addr_mode = AddressMode.NORMAL
        if opcode_elem.get('base') is not None:
            tpl_name = opcode_elem.get('base')
            tpl = low_level_templates.get(tpl_name)
            if tpl is None:
                raise ValueError(f"Template '{tpl_name}' not found for opcode '{name}'")
            params = tpl.clone_params()
        else:
            params = []
        parse_low_level_opcode_params(opcode_elem, params)
        variant = opcode_elem.get('variant')
        param_size_type_str = opcode_elem.get('size_param')
        if param_size_type_str is not None and param_size_type_str == '1':
            params.append(OpcodeParam("size", None, "size", "size"))
        opcode = LowLevelOpcode(name, params, variant, addr_mode)
        low_level_opcodes.append(opcode)
    return compile_low_level_opcodes(low_level_opcodes)

def parse_low_level_opcode_file(file_path):
    tree = ET.parse(file_path)
    root = tree.getroot()
    return parse_low_level_opcodes(root)
