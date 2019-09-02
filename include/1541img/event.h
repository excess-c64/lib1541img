#ifndef I1541_EVENT_H
#define I1541_EVENT_H

typedef void (*EventHandler)(void *receiver, int id,
        const void *sender, const void *args);

typedef struct Event Event;

Event *Event_create(int id, const void *sender);
void Event_register(Event *self, void *receiver, EventHandler handler);
void Event_unregister(Event *self, void *receiver, EventHandler handler);
void Event_raise(Event *self, const void *args);
void Event_destroy(Event *self);

#endif
