#ifndef BINDING_ASSERT_H
#define BINDING_ASSERT_H

#include <assert.h>
#include <stddef.h>

#define ASSERT_SIZE(type, size) static_assert(sizeof(type) == size, #type " is malformed.")
#define ASSERT_OFFSET(type, field, offset)                                                                             \
  static_assert(offsetof(type, field) == offset, #field " of type " #type " is not at the required offset " #offset)

#endif
