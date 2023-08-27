#include "frame.h"
#include <assert.h>

Frame new_frame(Closure *fn, size_t base_pointer) {
  return (Frame){
      .closure = fn,
      .ip = -1,
      .base_pointer = base_pointer,
  };
}

const Instructions *frame_instructions(Frame *f) {
  switch(f->closure->enclosed->type) {
    case COMPILED_FUNCTION_OBJ:
      return &((CompiledFunction *)f->closure->enclosed)->instructions;
    case COMPILED_LOOP_OBJ:
      return &((CompiledLoop *)f->closure->enclosed)->instructions;
    default:
      assert(0 && "unreachable");
  }
}
