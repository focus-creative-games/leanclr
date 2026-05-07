# LeanAOT AOT rule files (`aot.xml`) — user guide

This guide explains how to author **AOT rule XML** to control which managed methods are included in AOT code generation. These files are **independent of Unity’s `link.xml`** and are consumed only by LeanAOT.

**Implementation details and algorithms** (conflict handling, manifest integration, etc.): [aot-rule-file-design.md](aot-rule-file-design.md).  
**End-to-end AOT workflow:** [aot.md](aot.md).

---

## 1. What is a rule file?

- One or more **XML documents** whose root element is **`<aot>`**.
- A hierarchy of **assembly → type → method** declares whether methods should be AOT-compiled (`1` = include, `0` = exclude).
- You may set **defaults** at assembly or type scope, then use **per-method rules** as exceptions.

**Current limitation:** only ordinary **`MethodDef`** instances; generic instantiation and similar cases are out of scope. Do **not** use metadata **tokens** in configuration (they change when the DLL is rebuilt).

---

## 2. Specifying rule files on the command line

Pass each rule file with **`--leanaot-aot-rule-file`**; repeat the option for multiple files:

```text
LeanAOT ... --leanaot-aot-rule-file path\to\base.xml --leanaot-aot-rule-file path\to\game.xml
```

- Paths may be **absolute** or **relative to the current working directory**.
- **Multiple files:** later files refine or extend rules when there is **no conflict**; if **one file assigns `1` and another assigns `0`** to the **same method**, LeanAOT **fails** (see “Multiple rule files” below).

The exact option spelling may vary by LeanAOT build; use `--help` if in doubt.

---

## 3. Root structure and encoding

- **UTF-8** is recommended.
- The root element must be **`<aot>`**.

```xml
<?xml version="1.0" encoding="utf-8"?>
<aot>
  <!-- one or more <assembly> elements -->
</aot>
```

---

## 4. The `aot` attribute: only `1` and `0`

On **`<assembly>`**, **`<type>`**, and **`<method>`**, the **`aot`** attribute must be the literal string **`1`** or **`0`** (do not use `true`, `required`, etc., to avoid typos).

| Value | Meaning |
|-------|---------|
| **`1`** | **Include in AOT.** On an assembly or type, this is the **default include** for methods not matched more specifically below. |
| **`0`** | **Exclude from AOT.** On an assembly or type, this is the **default exclude** for that scope. |

Invalid values cause LeanAOT to **fail**; they are not ignored.

---

## 5. Three levels: assembly, type, method

### 5.1 `<assembly fullname="..." aot="...">`

- **`fullname`** (required): assembly identity; must match how you pass assemblies to LeanAOT (`-a` / `--assembly`, with or without `.dll`, per tool behavior).
- **`aot`** (optional): **default** for the whole assembly. If omitted, this `<assembly>` node contributes **no assembly-level default**; if children also omit defaults, a method may end up with **no XML verdict** and then **defaults to AOT** (see Sections 7 and 8.1).

### 5.2 `<type fullname="..." aot="...">`

- **`fullname`** (required): the type’s **full name** (including namespace). **Wildcards** are supported (Section 6). For nested types, use the same full-name format as LeanAOT/metadata (see project docs or error messages).
- **`aot`** (optional): **default** for methods under this type that are **not matched by any `<method>`** child.  
  - If **`aot`** is omitted on `<type>`: the type inherits the **parent `<assembly>`** `aot` value; if the assembly also omits `aot`, there may be **no XML default** for that type, and methods not matched by `<method>` fall through to **no XML verdict → AOT by default**.

### 5.3 `<method name="..." signature="..." aot="1|0">`

- **`name`** (required): short method name; **wildcards** allowed.
- **`signature`** (optional): disambiguates overloads or matches in bulk; must match LeanAOT’s internal **method signature string**; **wildcards** allowed. If omitted, the rule matches **every overload** with that short name.
- **`aot`** (required): `1` or `0`.

---

## 6. Wildcards

In **`type/@fullname`**, **`method/@name`**, and **`method/@signature`** (when present):

| Symbol | Meaning |
|--------|---------|
| `*` | Matches a substring of any length (including empty). |
| `?` | Matches **exactly one** character. |

Examples:

- `MyCompany.Game.Logic.*`: types under the `MyCompany.Game.Logic` namespace (whether child namespaces are included depends on the implementation’s glob rules; see design doc).
- `Update*`: every method whose short name starts with `Update`.

---

## 7. How defaults flow (intuitive summary)

1. If the **assembly** has **`aot`**: any **`<type>`** that **does not set its own `aot`** uses that assembly default.  
2. If a **type** has **`aot`**: methods under that type that **match no `<method>` rule** use the **type default**.  
3. **Assembly only, no `<type>` children:** if the assembly sets **`aot`**, methods in that assembly that **match no `<method>` rule** still get the **assembly default**.

If, at the **rule layer**, a method **matches no rule** that assigns `1` or `0` (no applicable `assembly` / `type` / `method` combination), then after Section **8** steps 1–2, the method is still **included in AOT** (“not mentioned in rules ⇒ AOT”).  
To exclude methods, you must set **`aot="0"`** explicitly (assembly/type default or a specific `<method>`).

---

## 8. What overrides the rule file (required reading)

These two cases **do not require XML** and **cannot be turned off** by the rule file:

1. **`[AotMethod]` attribute**  
   If the method carries your **`AotMethod`** attribute (sometimes written `[AotMethod]` in docs), the **attribute always wins**. Rule file `1`/`0` **does not override** it.

2. **P/Invoke and internal calls**  
   Such methods **must be AOT-compiled**. Rule file `aot="0"` is **ignored** (optionally with a warning). **Do not rely on XML** to strip them from AOT.

Only ordinary methods that **neither** are decided by the attribute path **nor** belong to the forced-AOT category are evaluated by the **rule file**:
- If **rules assign** `1` or `0` (including inherited defaults), that value is used.  
- If **no rule matches**, the method is **included in AOT by default** (you do not need to list every method with `1`).

---

## 8.1 Rule-layer outcome summary

| Situation | AOT? (when Section 8 steps 1–2 do not apply) |
|-----------|-----------------------------------------------|
| Rules yield **`1`** for the method | Yes |
| Rules yield **`0`** | No |
| **No rule matches the method** | **Yes** (default AOT) |

---

## 9. Multiple rules in one file: which wins?

Within a **single** XML file, if several rules **apply to the same method** (mixing assembly/type defaults and `<method>` rules), **document order** applies: **a later rule overrides an earlier one**.

You can therefore write a broad rule first, then narrower rules below as exceptions.

---

## 10. Using multiple rule files

- Files are loaded in **command-line order** (left to right / first to last).
- **Same method:** if one file’s **final** verdict is **`1`** and another’s is **`0`**, LeanAOT **errors and stops** (no silent tie-break).
- If every file that assigns a verdict **agrees** (`1` everywhere or `0` everywhere), there is no conflict.

For a “base + patch” workflow, put **only deltas** in the patch file and avoid assigning **opposite** `1`/`0` to the same method across files.

---

## 11. Full example

```xml
<?xml version="1.0" encoding="utf-8"?>
<aot>
  <assembly fullname="Game" aot="0">
    <type fullname="Game.Core.*" aot="1">
      <method name="Tick" aot="0" />
      <method name="Init*" signature="void Init*(System.Int32)" aot="1" />
    </type>
  </assembly>
</aot>
```

Assuming Section **8** does not short-circuit:

- Under assembly **`Game`**, the default is **exclude** (`0`).  
- Types matching **`Game.Core.*`** default to **include** (`1`).  
- Method **`Tick`** is **forced off** AOT (`0`).  
- Methods named like **`Init*`** whose signature matches the pattern are **forced on** (`1`).  
- If another rule below targets the same method again, the **lower rule in the file wins**.

---

## 12. FAQ

**Can I address a method by metadata token?**  
No. Use assembly + type full name + method name, and add `signature` when needed.

**Must `signature` match my C# source exactly?**  
It must match LeanAOT’s **internal signature string**. If matching fails, enable debug logging or read the mapping in [aot-rule-file-design.md](aot-rule-file-design.md).

**Do I maintain `link.xml` together with this?**  
No. They are unrelated: `link.xml` is for Unity’s linker pipeline; `aot.xml` is for LeanAOT only.

**How do I “AOT only a handful of methods”?**  
Because **unmatched methods default to AOT**, you need a **broad default `aot="0"`** (assembly or type), then **`aot="1"`** on the few types or `<method>` entries you want included.

**What if I pass no rule files at all?**  
After Section **8** steps 1–2, there is **no rule-layer verdict** ⇒ **AOT by default**, consistent with design **8.4**.

---

## 13. Related documents

| Document | Contents |
|----------|----------|
| [aot-rule-file-design.md](aot-rule-file-design.md) | Implementation spec, evaluation order, conflict algorithm |
| [aot.md](aot.md) | LeanAOT workflow, inputs/outputs, runtime registration |
