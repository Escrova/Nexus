#pragma once

struct Value {
    bool initialized;
    int number;

    Value();
    Value(bool initialized, int number);
};
