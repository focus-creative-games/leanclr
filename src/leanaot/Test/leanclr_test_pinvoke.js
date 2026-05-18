mergeInto(LibraryManager.library, {
  leanclr_pinvoke_add_i32: function (a, b) {
    return (a + b) | 0;
  },

  leanclr_pinvoke_mul_i32: function (a, b) {
    return Math.imul(a, b) | 0;
  },

  leanclr_pinvoke_neg_i32: function (x) {
    return -x | 0;
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

  leanclr_pinvoke_return_null_utf8: function (sPtr) {
    return 0;
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

  leanclr_pinvoke_struct_pair_mul_add: function (a, b) {
    return ((a | 0) * (b | 0) + (b | 0)) | 0;
  },

  leanclr_pinvoke_invoke_binary_op: function (cbPtr, a, b) {
    if (!cbPtr) {
      return 0;
    }
    var tbl = typeof wasmTable !== 'undefined' ? wasmTable : (typeof Module !== 'undefined' ? Module['wasmTable'] : null);
    if (!tbl || !tbl.get) {
      console.warn('leanclr_pinvoke_invoke_binary_op: wasmTable not available');
      return 0;
    }
    return tbl.get(cbPtr)(a | 0, b | 0) | 0;
  },

  leanclr_pinvoke_invoke_string_utf8_len_op: function (cbPtr, sPtr) {
    if (!cbPtr) {
      return 0;
    }
    var tbl = typeof wasmTable !== 'undefined' ? wasmTable : (typeof Module !== 'undefined' ? Module['wasmTable'] : null);
    if (!tbl || !tbl.get) {
      console.warn('leanclr_pinvoke_invoke_string_utf8_len_op: wasmTable not available');
      return 0;
    }
    return tbl.get(cbPtr)(sPtr) | 0;
  },

  leanclr_pinvoke_invoke_array_sum_op: function (cbPtr, arrPtr, count) {
    if (!cbPtr || !arrPtr || count <= 0) {
      return 0;
    }
    var tbl = typeof wasmTable !== 'undefined' ? wasmTable : (typeof Module !== 'undefined' ? Module['wasmTable'] : null);
    if (!tbl || !tbl.get) {
      console.warn('leanclr_pinvoke_invoke_array_sum_op: wasmTable not available');
      return 0;
    }
    return tbl.get(cbPtr)(arrPtr, count | 0) | 0;
  },

  leanclr_pinvoke_invoke_struct_op: function (cbPtr, a, b) {
    if (!cbPtr) {
      return 0;
    }
    var tbl = typeof wasmTable !== 'undefined' ? wasmTable : (typeof Module !== 'undefined' ? Module['wasmTable'] : null);
    if (!tbl || !tbl.get) {
      console.warn('leanclr_pinvoke_invoke_struct_op: wasmTable not available');
      return 0;
    }
    return tbl.get(cbPtr)(a | 0, b | 0) | 0;
  },

  leanclr_pinvoke_invoke_safe_handle_op: function (cbPtr, h) {
    if (!cbPtr) {
      return 0;
    }
    var tbl = typeof wasmTable !== 'undefined' ? wasmTable : (typeof Module !== 'undefined' ? Module['wasmTable'] : null);
    if (!tbl || !tbl.get) {
      console.warn('leanclr_pinvoke_invoke_safe_handle_op: wasmTable not available');
      return 0;
    }
    return tbl.get(cbPtr)(h | 0) | 0;
  },

  leanclr_pinvoke_invoke_nested_binary_op: function (outerPtr, innerPtr, a, b) {
    if (!outerPtr) {
      return 0;
    }
    var tbl = typeof wasmTable !== 'undefined' ? wasmTable : (typeof Module !== 'undefined' ? Module['wasmTable'] : null);
    if (!tbl || !tbl.get) {
      console.warn('leanclr_pinvoke_invoke_nested_binary_op: wasmTable not available');
      return 0;
    }
    return tbl.get(outerPtr)(innerPtr, a | 0, b | 0) | 0;
  },

  leanclr_pinvoke_safe_handle_add_ten: function (h) {
    return (h + 10) | 0;
  },

  leanclr_pinvoke_ansi_string_builder_byte_len: function (sbPtr) {
    if (!sbPtr) {
      return -1;
    }
    var h = HEAPU8;
    var n = 0;
    while (h[sbPtr + n]) {
      n++;
    }
    return n | 0;
  },

  leanclr_pinvoke_ansi_string_builder_set_native_text: function (sbPtr) {
    if (!sbPtr) {
      return;
    }
    var text = "native";
    for (var i = 0; i < text.length; i++) {
      HEAPU8[sbPtr + i] = text.charCodeAt(i);
    }
    HEAPU8[sbPtr + text.length] = 0;
  },

  leanclr_pinvoke_utf8_string_builder_byte_len: function (sbPtr) {
    if (!sbPtr) {
      return -1;
    }
    var h = HEAPU8;
    var n = 0;
    while (h[sbPtr + n]) {
      n++;
    }
    return n | 0;
  },

  leanclr_pinvoke_utf8_string_builder_set_native_text: function (sbPtr) {
    if (!sbPtr) {
      return;
    }
    // UTF-8 for 好
    HEAPU8[sbPtr] = 0xe5;
    HEAPU8[sbPtr + 1] = 0xa5;
    HEAPU8[sbPtr + 2] = 0xbd;
    HEAPU8[sbPtr + 3] = 0;
  },
});
