"""IR format invariants and reproducibility: python -m unittest discover -s src/generator."""
from collections import Counter
from pathlib import Path
import re
import subprocess
import sys
import tempfile
import unittest
import xml.etree.ElementTree as ET

import opcode_spec_parser as parser

HERE = Path(__file__).resolve().parent
INTERP = HERE.parent / "runtime/interp"


class OpcodeTests(unittest.TestCase):
    def test_single_opcode_space_and_dispatch_coverage(self):
        ops = parser.parse_low_level_opcode_file(HERE / "ll-opcodes.xml")
        self.assertGreater(len(ops), 256)
        self.assertEqual([op.code for op in ops], list(range(len(ops))))
        self.assertFalse(any(op.name.endswith("Short") for op in ops))
        cases = re.findall(r"^\s+LEANCLR_CASE_BEGIN(?:_LITE)?\((\w+)\)",
                           (INTERP / "interpreter.cpp").read_text(), re.M)
        self.assertEqual(Counter(cases), Counter(op.name for op in ops))
        # Parsing twice must not consume a process-global opcode allocator.
        again = parser.parse_low_level_opcode_file(HERE / "ll-opcodes.xml")
        self.assertEqual([op.code for op in ops], [op.code for op in again])

    def test_operand_widths_and_direct_parameters(self):
        ops = {op.name: op for op in parser.parse_low_level_opcode_file(HERE / "ll-opcodes.xml")}
        self.assertEqual({p.name: p.type for p in ops["Switch"].params
                          if not p.name.startswith("__")}, {"index": "u16", "num_targets": "u32"})
        self.assertEqual(next(p.type for p in ops["Br"].params if p.name == "target_offset"), "i32")
        self.assertEqual(next(p.type for p in ops["InitLocals"].params if p.name == "offset"), "u16")
        for op in ops.values():
            offset = 2
            for param in op.params:
                size = parser.get_type_size(param.type)
                self.assertEqual(offset % size, 0, (op.name, param.name))
                offset += size
            self.assertEqual(offset % 4, 0, op.name)

    def test_reject_short_variants_and_duplicate_names(self):
        for spec in ['<llopcodes><opcode name="Nop_S"/></llopcodes>',
                     '<llopcodes><opcode name="Nop"/><opcode name="Nop"/></llopcodes>']:
            with self.assertRaises(ValueError):
                parser.parse_low_level_opcodes(ET.fromstring(spec))

    def test_checked_in_generation_is_reproducible(self):
        with tempfile.TemporaryDirectory() as temp:
            outputs = [Path(temp) / name for name in ("ll_opcodes.h", "ll_opcodes.cpp", "interpreter.cpp")]
            for output in outputs:
                output.write_bytes((INTERP / output.name).read_bytes())
            command = [sys.executable, str(HERE / "gen_low_level_opcodes.py"),
                       str(HERE / "ll-opcodes.xml"), *map(str, outputs)]
            subprocess.run(command, check=True)
            for output in outputs:
                self.assertEqual(output.read_text(), (INTERP / output.name).read_text(), output.name)
            first = [output.read_bytes() for output in outputs]
            subprocess.run(command, check=True)
            self.assertEqual(first, [output.read_bytes() for output in outputs])


if __name__ == "__main__":
    unittest.main()
