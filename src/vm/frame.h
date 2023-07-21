#include "../code/code.h"
#include "../object/object.h"
#include <stdint.h>

typedef struct {
  CompiledFunction *fn;
  int64_t ip;
} Frame;

Frame new_frame(CompiledFunction *);
const Instructions *frame_instructions(Frame *); 
