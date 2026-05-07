# LeanAOT AOT rule files (`aot.xml`) — implementation specification

This document is intended for **implementers and code review**. It specifies parsing, merging, and integration with manifest generation. The end-user configuration guide is [aot-rule-file.md](aot-rule-file.md).

Background: [AOT workflow overview](aot.md).

---

## 1. Goals and scope

### 1.1 Goals

- Control which **`MethodDef`** instances are included in AOT via **standalone XML files** (collectively referred to as `aot.xml` in code and docs; the physical filename is arbitrary).
- Support **assembly-level and type-level defaults** plus **method-level rules**; support **glob wildcards**; support **multiple files** with explicit **conflict semantics**.

### 1.2 Scope and non-goals

| Item | Description |
|------|-------------|
| **`MethodDef` only** | Does not cover generic instantiations, `MethodSpec`, etc. |
| **Independent of `link.xml`** | Does not read or depend on Unity `link.xml`. |
| **No tokens** | `MethodDef` metadata tokens must not appear in configuration. |

---

## 2. File format and root element

- Encoding: UTF-8 is recommended.
- Root element name: **`aot`** (fixed).
- Optional: a `version` attribute for future incompatible revisions; v1 may omit strict validation.

---

## 3. Attributes and allowed values

### 3.1 `aot` values

On **`assembly`**, **`type`**, and **`method`**:

- Only **`"1"`** or **`"0"`** are allowed.
- Any other value: parse **fails** (hard failure; report file path and element).

Semantics:

- **`1`**: include in AOT (manifest); when used as a default, it is the default for methods in the subtree that are not matched more specifically.
- **`0`**: exclude from AOT; when used as a default, it is the default exclusion for that scope.

### 3.2 Element attribute reference

**`<assembly>`**

| Attribute | Required | Description |
|-----------|----------|-------------|
| `fullname` | Yes | Assembly match key; must align with runtime/load logic. The implementation must **pick one matching rule** (simple name vs. full display name, `.dll` suffix, etc.) and document it. |
| `aot` | No | Assembly-wide default. |

**`<type>`**

| Attribute | Required | Description |
|-----------|----------|-------------|
| `fullname` | Yes | Type full name; glob; **one fixed format** for nested types (consistent with metadata). |
| `aot` | No | Type-level default; if omitted, inherits `assembly/@aot` (if the assembly also omits `aot`, there is no XML default at that level). |

**`<method>`**

| Attribute | Required | Description |
|-----------|----------|-------------|
| `name` | Yes | Short method name; glob. |
| `signature` | No | Must match the implementation’s **`MethodDef` display signature** string; glob; if omitted, the rule matches **all overloads** with that short name. |
| `aot` | Yes | `1` or `0`. |

---

## 4. Glob subset

- `*`: any-length substring (including empty).
- `?`: exactly one character.
- Applies to: `type/@fullname`, `method/@name`, `method/@signature`.
- v1 may omit escaping of literal `*` and `?`; evolve via `version` if needed.

---

## 5. Default inheritance (assembly → type)

1. If `assembly/@aot` is present: any **`<type>`** that **does not specify its own `aot`** uses the assembly default as its type-level default.
2. If `type/@aot` is present: any **`MethodDef`** under that type that **is not matched by any `<method>`** rule uses the type default when evaluating the XML branch only.
3. If there is only `<assembly ... aot="...">` with **no `<type>` children**: methods in that assembly that are **not matched by `<method>`** use the **assembly default** when a default is needed.
4. No XML conclusion: after **8.1** and **8.2**, **8.4** applies (**AOT by default**).

---

## 6. Single-file merge: later overrides earlier

- For a given **`MethodDef`**, within one XML document, every rule that yields an explicit **`1`/`0`** (including `<method>` matches and defaults implied by `assembly`/`type`) is applied in **document order**.
- The **last** verdict written for that method is the **single-file XML verdict**.
- Alternating `1` and `0` in one file is **not** an error.

Implementation note: model defaults as ordered updates in the same stream as `<method>` rules, or as one ordered rule list; behavior must match this specification.

---

## 7. Multi-file merge and conflicts

- Paths come from **`AotMethodRuleFiles`**, in **CLI order** (each repeated `--leanaot-aot-rule-file` occurrence appends one path).
- For each **`MethodDef`**, record whether **each file** produced an **explicit XML verdict** (`1` or `0`) after applying that file’s internal “later overrides earlier” merge.
- If **at least two files** each produce a verdict and the values **differ** (`1` vs `0`): **hard fail**; the error must identify assembly, type, method, conflicting paths, and values.
- If all files that produce a verdict **agree**: use that value.
- If **only one file** produces a verdict: use that value.

**Methods excluded from cross-file conflict checks** (Section 8): methods for which the rule files **never produce an applicable XML verdict** (e.g. already decided by an attribute, or in the “must AOT” category). The implementation should apply **8.1** and **8.2** first, then run XML merge and **§7** conflict detection only on the remaining methods (or skip conflict rows for methods with no XML participation).

---

## 8. Interaction with attributes, forced AOT, and defaults

Whether a **`MethodDef`** is included in the AOT manifest is decided **top-down** (first match wins; later stages must not override):

| Step | Condition | Outcome |
|------|-----------|---------|
| **8.1** | **`AotMethodAttribute`** is present and the implementation derives include/exclude from it | **Attribute wins**; **rule file must not override**. |
| **8.2** | **`IsPinvokeImpl`** or **`IsInternalCall`** (and any other implementation-defined **must-AOT** category) | **Always AOT**; rule file **`aot="0"` has no effect**; optional warning or silent ignore of XML. |
| **8.3** | Otherwise | Evaluate **`aot.xml`**: single-file **§6**, multi-file **§7**, inheritance **§5**; if a clear **`1`/`0`** is produced, use it. |
| **8.4** | After **8.3**, there is **still no XML verdict** (no matching `assembly`/`type`/`method` chain plus inherited defaults that assign `1`/`0`) | **Include in AOT** (treat the XML layer’s final verdict as **`1`**). |

**Note:** No rule files loaded, or all loaded files yield **no verdict** for a method, counts as “no XML conclusion”; **8.4** applies — **AOT**.

---

## 9. CLI and `CliOptions`

| Item | Specification |
|------|----------------|
| CLI | Same naming style as other `leanaot-*` options; e.g. **`--leanaot-aot-rule-file <path>`**, **repeatable**; each occurrence appends one path. |
| Type | **`AotMethodRuleFiles`**: `IEnumerable<string>` (replaces the former `AotMethodRuleFile`). |
| Order | Collection order equals the order of occurrences on the command line. |
| Errors | Missing path, invalid XML, or cross-file conflict: fail with path or detailed message. |

---

## 10. Integration with `Manifest`

- On the code path that **builds the per-method AOT manifest** (`Manifest` or an equivalent step), apply Section **8** in order for each candidate **`MethodDef`**.
- **Signature string:** the implementation must define a single, stable **`MethodDef` → string`** mapping for `method/@signature` glob matching; document it in code comments or in an appendix to this spec.

---

## 11. Implementation checklist

1. XML parse and validation: root is `aot`; `aot` attributes are only `1`/`0`.  
2. Glob: assembly, type, method, and optional `signature`.  
3. Single-file ordered application: **later overrides earlier**.  
4. Per-method per-file verdicts and **§7** conflict detection.  
5. CLI wiring for **`AotMethodRuleFiles`**.  
6. `Manifest` integration: **8.1 → 8.2 → 8.3 → 8.4** (**8.4 = AOT when there is no XML verdict**).  
7. Pin down **`fullname`** and **signature** formats; unit tests for edge cases and conflicts.

---

## 12. Diagnostics (recommended)

- Debug/Verbose: optionally log whether a method’s decision came from XML, `1`/`0`, source file, and matched pattern.

---

## 13. Revision history

| Date | Notes |
|------|--------|
| — | Split from combined draft; single-file later-overrides; multi-file conflict error; `[AotMethod]` precedence; P/Invoke and InternalCall always AOT and ignore rule file exclusions. |
| — | **8.4:** no rule match (no XML verdict) ⇒ **AOT by default**. |
