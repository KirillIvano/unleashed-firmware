#include "brain_dead.h"

#define LOG_TAG "brain_dead"

static BrainDeadApp* init_app() {
    BrainDeadApp* app = malloc(sizeof(BrainDeadApp));

    app->mutex = furi_mutex_alloc(FuriMutexTypeNormal);
    app->queue = furi_message_queue_alloc(8, sizeof(BrainDeadEvent));

    app->pos_x = 0;
    app->pos_y = 0;

    return app;
}

static void cleanup_app(BrainDeadApp* app) {
    furi_assert(app);

    furi_mutex_free(app->mutex);
    furi_message_queue_free(app->queue);

    free(app);
}

static void _brain_dead_draw_shit(Canvas* canvas, BrainDeadApp* app) {
    canvas_draw_circle(canvas, app->pos_x, app->pos_y, 10);
}

static void update_frame_data(BrainDeadApp* app) {
    UNUSED(app);
}

static void brain_dead_draw_callback(Canvas* canvas, void* context) {
    FURI_LOG_D(LOG_TAG, "draw callback");
    BrainDeadApp* app = context;

    FURI_LOG_D(LOG_TAG, "mutex acquire");
    furi_mutex_acquire(app->mutex, FuriWaitForever);
    FURI_LOG_D(LOG_TAG, "working with graphics");

    _brain_dead_draw_shit(canvas, app);

    FURI_LOG_D(LOG_TAG, "mutex release");
    furi_mutex_release(app->mutex);

    update_frame_data(app);
}

static void brain_dead_input_callback(InputEvent* event, void* context) {
    FURI_LOG_D(LOG_TAG, "input callback");
    BrainDeadApp* app = context;

    furi_assert(app->queue);

    BrainDeadEvent ev = {.input_type = event->type, .input_key = event->key};
    furi_message_queue_put(app->queue, &ev, 0);
}

int32_t brain_dead_app(void* p) {
    UNUSED(p);

    FURI_LOG_I(LOG_TAG, "app totally started");

    BrainDeadApp* app = init_app();

    ViewPort* view_port = view_port_alloc();

    view_port_draw_callback_set(view_port, brain_dead_draw_callback, app);
    view_port_input_callback_set(view_port, brain_dead_input_callback, app);

    Gui* gui = furi_record_open(RECORD_GUI);
    gui_add_view_port(gui, view_port, GuiLayerFullscreen);

    while(!app->app_finished) {
        BrainDeadEvent inputEvent;

        FuriStatus status = furi_message_queue_get(app->queue, &inputEvent, 0);

        if(status == FuriStatusOk) {
            if(inputEvent.input_key == InputKeyBack && inputEvent.input_type != InputTypePress) {
                app->app_finished = true;
                break;
            } else if(inputEvent.input_key == InputKeyLeft) {
                app->pos_x -= 5;
            } else if(inputEvent.input_key == InputKeyRight) {
                app->pos_x += 5;
            } else if(inputEvent.input_key == InputKeyDown) {
                app->pos_y += 5;
            } else if(inputEvent.input_key == InputKeyUp) {
                app->pos_y -= 5;
            }
        }

        furi_delay_ms(100);
    }

    gui_remove_view_port(gui, view_port);
    view_port_free(view_port);
    cleanup_app(app);

    furi_record_close(RECORD_GUI);

    return 0;
}
