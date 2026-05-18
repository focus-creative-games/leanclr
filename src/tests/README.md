# LeanCLR Test Framework

This directory contains the unit test framework and test cases for the LeanCLR runtime.

## Directory Structure

```
tests/
├── basic_test_runner/     # C++ test runner (loads and executes managed test assemblies)
├── managed/               # C# managed test projects
│   ├── CoreTests/         # Core functionality tests
│   ├── CorlibTests/       # Base class library tests
│   └── ILTests/           # IL instruction tests
├── scripts\test\build-all.bat          # Forwards to scripts/test/build-all.bat
└── scripts\test\run.bat          # Forwards to scripts/test/run.bat
```

Official scripts live under [`scripts/test/`](../../scripts/test/). See [`scripts/README.md`](../../scripts/README.md).

---

## Adding Test Cases

### 1. Create a Test Class

Add test methods to any existing class in the `managed/CoreTests/Tests/` directory, or create a new test class file.

**Test Class Requirements:**

- Must inherit from the `GeneralTestCaseBase` base class
- Must have a **parameterless constructor** (implicit default constructor is acceptable)

```csharp
namespace Tests.CSharp
{
    class TC_MyFeature : GeneralTestCaseBase
    {
        // Test methods go here
    }
}
```

### 2. Write Test Methods

**Test Method Requirements:**

| Requirement | Description |
|-------------|-------------|
| Access Modifier | `public` |
| Return Type | `void` |
| Parameters | None |
| Instance Method | Non-static (cannot be `static`) |
| Attribute | Must have the `[UnitTest]` attribute |

**Example:**

```csharp
using System;

namespace Tests.CSharp
{
    class TC_MyFeature : GeneralTestCaseBase
    {
        [UnitTest]
        public void TestAddition()
        {
            int result = 1 + 2;
            Assert.Equal(3, result);
        }

        [UnitTest]
        public void TestStringNotNull()
        {
            string s = "hello";
            Assert.NotNull(s);
            Assert.Equal(5, s.Length);
        }
    }
}
```

### 3. Assert Class

The `Assert` class provides a rich set of assertion methods for validating test results:

| Method | Description |
|--------|-------------|
| `Fail()` | Immediately fail the test |
| `Fail(string message)` | Fail the test with a message |
| `IsTrue(bool condition)` | Assert that condition is `true` |
| `IsFalse(bool condition)` | Assert that condition is `false` |
| `True(bool condition)` | Alias for `IsTrue` |
| `False(bool condition)` | Alias for `IsFalse` |
| `Null(object obj)` | Assert that object is `null` |
| `NotNull(object obj)` | Assert that object is not `null` |
| `Equal(T a, T b)` | Assert that two values are equal |
| `NotEqual(int a, int b)` | Assert that two integers are not equal |
| `EqualAny(object a, object b)` | Assert that two objects are equal (uses `Object.Equals`) |

**Type overloads supported by the `Equal` method:**

- `bool`, `int`, `long`, `float`, `double`, `char`, `string`, `Type`
- Pointer type `int*`
- Generic `Equal<T>(T a, T b)`

> **Note:** When an assertion fails, an `Exception` is thrown. The test runner catches the exception and reports the test as failed.

---

## Building Test Projects

### Option 1: One-Click Build (Recommended)

Use `scripts\test\build-all.bat` (or legacy `build_all.bat`) to build both the test runner and managed test projects:

```batch
# Build with default Debug configuration, x64 architecture
scripts\test\build-all.bat

# Specify configuration and architecture
scripts\test\build-all.bat <Config> [Arch]

# Examples
scripts\test\build-all.bat Debug x64
scripts\test\build-all.bat Release x64
scripts\test\build-all.bat Debug x86
```

**Parameters:**

| Parameter | Values | Default | Description |
|-----------|--------|---------|-------------|
| `Config` | `Debug`, `Release` | `Debug` | Build configuration |
| `Arch` | `x64`, `x86` | `x64` | Target architecture |

### Option 2: Build Separately

#### Build C++ Test Runner

```batch
cd basic_test_runner
build.bat [Config] [Arch]

# Examples
build.bat Debug x64
build.bat Release x64
```

**Parameters:**

| Parameter | Values | Default | Description |
|-----------|--------|---------|-------------|
| `Config` | `Debug`, `Release` | `Debug` | Build configuration |
| `Arch` | `x64`, `x86` | `x64` | Target architecture |

After building, the executable is located at:
```
out/cmake/tests/basic_test_runner/<Config>-x64/bin/<Config>/test.exe
```

#### Build C# Managed Test Projects

```batch
cd managed
dotnet build -c <Config>

# Examples
dotnet build -c Debug
dotnet build -c Release
```

---

## Running Tests

Use `scripts\test\run.bat` to run all tests:

```batch
# Run tests with Debug configuration
scripts\test\run.bat

# Run tests with specified configuration
scripts\test\run.bat <Config>

# Examples
scripts\test\run.bat Debug
scripts\test\run.bat Release
```

**Parameters:**

| Parameter | Values | Default | Description |
|-----------|--------|---------|-------------|
| `Config` | `Debug`, `Release` | `Debug` | Build configuration to run |

**Example Output:**

```
Running tests...
[PASS] Tests.CSharp.TC_String.EmptyString
[PASS] Tests.CSharp.TC_Enum.GetRed_0
[PASS] Tests.CSharp.TC_Enum.GetGreen_1
...
All tests passed.
```

> **Note:** Make sure to run `scripts\test\build-all.bat` before running tests. The script will display an error if the test runner is not found.

---

## Quick Start

```batch
# 1. Build all test projects
scripts\test\build-all.bat

# 2. Run tests
scripts\test\run.bat

# Or complete in one step (Release configuration)
scripts\test\build-all.bat Release && scripts\test\run.bat Release
```

---

## Test Project Descriptions

| Project | Description |
|---------|-------------|
| `CoreTests` | LeanCLR core functionality tests, including C# language features, reflection, delegates, generics, etc. |
| `CorlibTests` | Base class library (mscorlib/System) tests, including internal calls, P/Invoke, etc. |
| `ILTests` | IL instruction-level tests, written using ILAsm |

---

## FAQ

### Q: Test run fails with "Test runner not found"

**A:** Run `scripts\test\build-all.bat` first to build the test projects.

### Q: How do I run only specific tests?

**A:** The current version does not support filtering specific tests. All methods with the `[UnitTest]` attribute will be executed.

### Q: How do I see detailed information about test failures?

**A:** When a test fails, an exception is thrown and the exception message is displayed in the console output.
