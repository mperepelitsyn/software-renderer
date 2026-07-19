# font8x8_basic.h declares the glyph table as `char`, but rows with values
# above 0x7F make that a narrowing error in C++; retype it at fetch time.
file(READ "${FONT_DIR}/font8x8_basic.h" content)
if(NOT content MATCHES "unsigned char font8x8_basic")
  string(REPLACE "char font8x8_basic" "inline constexpr unsigned char font8x8_basic"
         content "${content}")
  file(WRITE "${FONT_DIR}/font8x8_basic.h" "${content}")
endif()
