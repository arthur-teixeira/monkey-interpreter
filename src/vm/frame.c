#include "frame.h"

Frame new_frame(CompiledFunction *fn, size_t base_pointer) {
  return (Frame){
      .fn = fn,
      .ip = -1,
      .base_pointer = base_pointer,
  };
}

const Instructions *frame_instructions(Frame *f) {
  return &f->fn->instructions;
}
