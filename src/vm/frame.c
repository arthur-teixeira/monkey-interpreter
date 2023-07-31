#include "frame.h"

Frame new_frame(Closure *fn, size_t base_pointer) {
  return (Frame){
      .closure = fn,
      .ip = -1,
      .base_pointer = base_pointer,
  };
}

const Instructions *frame_instructions(Frame *f) {
  return &f->closure->fn->instructions;
}
