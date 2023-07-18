#include <furi.h>
#include <gui/gui.h>

typedef struct {
    InputKey input_key;
    InputType input_type;
} BrainDeadEvent;

typedef struct {
    FuriMutex* mutex;
    FuriMessageQueue* queue;

    int8_t pos_x;
    int8_t pos_y;

    bool app_finished;
} BrainDeadApp;
