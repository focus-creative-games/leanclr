# Custom P/Invoke Example for Win64

This project demonstrates how to add and register custom P/Invoke (Platform Invocation) functions in LeanCLR on the Windows 64-bit platform. It provides a practical example of integrating native C++ functions with managed C# code using LeanCLR's manual P/Invoke registration mechanism.

For a detailed explanation of the custom P/Invoke mechanism, see [Custom P/Invoke](https://doc.leanclr.com/docs/interop/custom-pinvoke).

## Project Structure

- **main.cpp**: Native C++ entry point, contains the registration and implementation of the custom P/Invoke function.
- **src/tests/managed/CoreTests/CustomPInvoke.cs**: C# code declaring the P/Invoke method signature.

## Build & Run

1. Open the solution or project in Visual Studio (or use CMake/Ninja for command-line builds).
2. Build the solution in Release or Debug mode for x64.
3. Run the resulting executable. The test will invoke the custom native function via P/Invoke and print the result.

## How This Project Adds the `my_add` P/Invoke Function

### 1. C# Declaration (P/Invoke Signature)

In `src/tests/managed/CoreTests/CustomPInvoke.cs`:

```csharp
using System.Runtime.InteropServices;

namespace test
{
    public class CustomPInvoke
    {
        [DllImport("CustomNativeLib.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern int Add(int a, int b);
    }
}
```

- The `Add` method is declared as a static extern method with the `DllImport` attribute.
- The DLL name and calling convention are placeholders; actual binding is handled manually in C++.

### 2. Native Implementation and Registration

In `main.cpp`:

- **Native Function Implementation:**

```cpp
int32_t my_add(int32_t a, int32_t b)
{
    return a + b;
}
```

- **Invoker Function:**

```cpp
RtResultVoid my_add_invoker(metadata::RtManagedMethodPointer, const metadata::RtMethodInfo*, const interp::RtStackObject* params, interp::RtStackObject* ret)
{
    size_t offset = 0;
    auto a = get_argument_from_eval_stack<int32_t>(params, offset);
    auto b = get_argument_from_eval_stack<int32_t>(params, offset);
    int32_t result = my_add(a, b);
    set_return_value_to_eval_stack(ret, result);
    RET_VOID_OK();
}
```

- **Registration:**

```cpp
void RegisterCustomPInvokeMethods()
{
    register_pinvoke_func(
        "[CoreTests]test.CustomPInvoke::Add(System.Int32,System.Int32)",
        (vm::PInvokeFunction)&my_add,
        my_add_invoker
    );
}

int main()
{
    // ...
    auto ret = vm::Runtime::Initialize();
    if (ret.is_err())
    {
        // ...
        return -1;
    }
    RegisterCustomPInvokeMethods();
    // ...
}
```

- The registration string must match the C# signature exactly.
- Registration occurs after the runtime is initialized.

## References

- [Custom P/Invoke](https://doc.leanclr.com/docs/interop/custom-pinvoke) — Full guide to custom P/Invoke in LeanCLR.
- [src/tests/managed/CoreTests/CustomPInvoke.cs](../../../../src/tests/managed/CoreTests/CustomPInvoke.cs) — C# P/Invoke declaration.
- [main.cpp](main.cpp) — Native implementation and registration.
