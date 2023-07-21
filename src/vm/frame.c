#include "frame.h"

Frame new_frame(CompiledFunction *fn) {
  return (Frame){
      .fn = fn,
      .ip = -1,
  };
}

const Instructions *frame_instructions(Frame *f) {
  return &f->fn->instructions;
}
