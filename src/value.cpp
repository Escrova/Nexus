#include "value.h"

Value::Value() : initialized(false), number(0) {}

Value::Value(bool initialized, int number) : initialized(initialized), number(number) {}
