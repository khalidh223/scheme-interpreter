#ifndef _INTERPRETER
#define _INTERPRETER

void interpret(Value *tree);
void bind(char *name, Value *(*function)(struct Value *), Frame *frame);
Value *primitiveAdd(Value *args);
Value *primitiveMinus(Value *args);
Value *primitiveEqual(Value *args);
Value *primitiveMultiply(Value *args);
Value *primitiveDivide(Value *args);
Value *primitiveModulo(Value *args);
Value *primitiveCons(Value *args);
Value *primitiveNull(Value *args);
Value *primitiveCar(Value *args);
Value *primitiveCdr(Value *args);
Value *primitiveGreaterThan(Value *args);
Value *primitiveLessThan(Value *args);
Value *eval(Value *expr, Frame *frame);

#endif

