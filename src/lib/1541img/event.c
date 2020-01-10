#include <stdlib.h>
#include <string.h>

#include "util.h"
#include "log.h"

#include <1541img/event.h>

#define EVCHUNKSIZE 4

typedef struct EvHandler
{
    void *receiver;
    EventHandler handler;
} EvHandler;

struct Event
{
    const void *sender;
    EvHandler *handlers;
    size_t size;
    size_t capa;
    int id;
};

SOEXPORT Event *Event_create(int id, const void *sender)
{
    Event *self = xmalloc(sizeof *self);
    self->sender = sender;
    self->handlers = 0;
    self->size = 0;
    self->capa = 0;
    self->id = id;
    return self;
}

SOEXPORT void Event_register(Event *self, void *receiver, EventHandler handler)
{
    if (self->size == self->capa)
    {
        self->capa += EVCHUNKSIZE;
        self->handlers = xrealloc(self->handlers,
                self->capa * sizeof *self->handlers);
    }
    self->handlers[self->size].receiver = receiver;
    self->handlers[self->size].handler = handler;
    ++self->size;
}

SOEXPORT void Event_unregister(
	Event *self, void *receiver, EventHandler handler)
{
    size_t pos;
    for (pos = 0; pos < self->size; ++pos)
    {
        if (self->handlers[pos].receiver == receiver
                && self->handlers[pos].handler == handler)
        {
            break;
        }
    }
    if (pos < self->size)
    {
        --self->size;
        if (pos < self->size)
        {
            memmove(self->handlers + pos, self->handlers + pos + 1,
                    (self->size - pos) * sizeof *self->handlers);
        }
    }
}

SOEXPORT void Event_raise(Event *self, const void *args)
{
    for (size_t i = 0; i < self->size; ++i)
    {
        self->handlers[i].handler(self->handlers[i].receiver, self->id,
                self->sender, args);
    }
}

SOEXPORT void Event_destroy(Event *self)
{
    if (!self) return;
    free(self->handlers);
    free(self);
}

