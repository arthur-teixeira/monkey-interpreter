#include "../code/code.h"
#include "../object/object.h"
#include <stdint.h>

typedef struct {
  Closure *closure;
  int64_t ip;
  size_t base_pointer;
} Frame;

Frame new_frame(Closure *, size_t);
const Instructions *frame_instructions(Frame *); 
