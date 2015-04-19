#pragma once
#include "global.h"

enum DynamicValueType {
  DYN_VALUE_NULL,
  DYN_VALUE_INTEGER,
  DYN_VALUE_REAL,
  DYN_VALUE_TEXT,
  DYN_VALUE_BLOB
};

struct DynamicValue {
  DynamicValueType type;

  union data {
    int integer;
    float real;
    std::string text;
  };
};

