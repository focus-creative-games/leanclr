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

  leanclr_pinvoke_safe_handle_add_ten: function (h) {
    return (h + 10) | 0;
  },
});
