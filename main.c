#include "interpreter/interpreter.h"

int main() {
    s_interpreter* Interpreter = init_interpreter();

    run(Interpreter);

    return 0;
}
