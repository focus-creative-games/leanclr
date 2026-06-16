# Custom P/Invoke Example for WebAssembly (WASM)

This project demonstrates how to add and register custom P/Invoke (Platform Invocation) functions in LeanCLR on the WebAssembly (WASM) platform. It provides a practical example of integrating native JavaScript or C++ functions with managed C# code using LeanCLR's manual P/Invoke registration mechanism.

For a detailed explanation of the custom P/Invoke mechanism, see [Custom P/Invoke](https://doc.leanclr.com/docs/interop/custom-pinvoke).

## Project Structure

- **main.cpp**: Native C++ entry point, contains the registration and implementation of the custom P/Invoke function for WASM.
- **mylib.js**: JavaScript library file (if using JS as the native backend for P/Invoke).
- **src/tests/managed/CoreTests/CustomPInvoke.cs**: C# code declaring the P/Invoke method signature.

## Build & Run

1. Ensure you have Emscripten installed and configured in your environment.
2. Build the project using CMake (or your preferred build system). The CMake configuration will automatically link the custom JS library and export the required functions.
3. Serve the output files (e.g., using a local HTTP server) and open the generated HTML/JS in a browser that supports WASM (such as Chrome or Firefox).
4. The test will invoke the custom native function via P/Invoke and print the result in the browser console or UI.

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
- The DLL name and calling convention are placeholders; actual binding is handled manually in C++/JS.

### 2. Native Implementation and Registration (C++/JS)

In `main.cpp`:

- **Native Function Implementation:**

```cpp
#ifdef EMSCRIPTEN
#include <emscripten.h>
// Declare the JS function (if implemented in JS)
extern "C" int32_t my_add(int32_t a, int32_t b);
#endif

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

// In your initialization code, after runtime is initialized:
// RegisterCustomPInvokeMethods();
```

- The registration string must match the C# signature exactly.
- Registration occurs after the runtime is initialized.

- **JavaScript Implementation (mylib.js):**

```js
mergeInto(LibraryManager.library, {
  my_add: function(a, b) {
    return a + b;
  }
});
```

- The JS function is linked at build time using the `--js-library` option in CMakeLists.txt.

## References

- [Custom P/Invoke](https://doc.leanclr.com/docs/interop/custom-pinvoke) — Full guide to custom P/Invoke in LeanCLR.
- [src/tests/managed/CoreTests/CustomPInvoke.cs](../../../../src/tests/managed/CoreTests/CustomPInvoke.cs) — C# P/Invoke declaration.
- [main.cpp](main.cpp) — Native implementation and registration.
- [mylib.js](mylib.js) — JavaScript implementation of the native function (if used).
