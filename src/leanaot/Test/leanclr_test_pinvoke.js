/**
 * Emscripten --js-library：为 Test 程序集中 P/Invoke 提供 wasm 导入实现。
 * 键名必须与 C# DllImport 的 EntryPoint / 元数据 ImplMap.Name 一致。
 *
 * string 参数/返回值在 wasm 侧为 const char*（UTF-8，0 结尾）；由 LeanCLR 生成体负责 UTF-16 互转。
 * int[] 参数为 int32_t*（元素区首指针），与 C# 中 SumIntRange 一致。
 */
mergeInto(LibraryManager.library, {
  leanclr_pinvoke_add_i32: function (a, b) {
    return (a + b) | 0;
  },

  leanclr_pinvoke_mul_i32: function (a, b) {
    return Math.imul(a, b) | 0;
  },

  leanclr_pinvoke_neg_i32: function (x) {
    return (-x) | 0;
  },

  leanclr_pinvoke_is_nonzero_i32: function (x) {
    return x ? 1 : 0;
  },

  leanclr_pinvoke_utf8_byte_len: function (sPtr) {
    if (!sPtr) {
      return 0;
    }
    var h = HEAPU8;
    var n = 0;
    while (h[sPtr + n]) {
      n++;
    }
    return n;
  },

  // 使用 Emscripten 内置 helper，避免在 --js-library 里直接 __deps _malloc（易被 LTO/裁剪导致 undefined symbol）
  leanclr_pinvoke_dup_utf8__deps: ['$allocateUTF8', '$UTF8ToString'],
  leanclr_pinvoke_dup_utf8: function (sPtr) {
    if (!sPtr) {
      return 0;
    }
    return allocateUTF8(UTF8ToString(sPtr));
  },

  leanclr_pinvoke_sum_int_range: function (arrPtr, count) {
    if (!arrPtr || count <= 0) {
      return 0;
    }
    var base = arrPtr >> 2;
    var sum = 0;
    for (var i = 0; i < count; i++) {
      sum += HEAP32[base + i] | 0;
    }
    return sum | 0;
  },
});
