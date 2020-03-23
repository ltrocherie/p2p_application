
#include <sys/queue.h>

typedef struct work {
    STAILQ_ENTRY(thread) next;
} work;