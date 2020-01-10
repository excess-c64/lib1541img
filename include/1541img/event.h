#ifndef I1541_EVENT_H
#define I1541_EVENT_H

/** declarations for the Event class
 * @file
 */

#include <1541img/decl.h>

/** Delegate for an event handler
 * @param receiver the receiving instance of the event
 * @param id the id of the event
 * @param sender the sending instance of the event
 * @param args a pointer to some additional event arguments
 */
typedef void (*EventHandler)(void *receiver, int id,
        const void *sender, const void *args);

/** An event.
 * This class models an Event. It is meant as a member of a class publishing
 * this event. It contains a numeric ID, so when it is raised, event
 * handlers have an easy way to determine what kind of event was raised if
 * they are designed to handle multiple events.
 * @class Event event.h <1541img/event.h>
 */
C_CLASS_DECL(Event);

/** Event default constructor
 * @memberof Event
 * @param id a numeric ID
 * @param sender a pointer to the instance publishing this event
 * @returns a newly created Event
 */
DECLEXPORT Event *Event_create(int id, const void *sender);

/** Register a handler to an event
 * This must be called to subscribe to an event.
 * @memberof Event
 * @param self the Event
 * @param receiver a pointer to the instance receiving this event (NULL for
 *     "static" handlers)
 * @param handler a pointer to a function handling the raised event
 */
DECLEXPORT void Event_register(
	Event *self, void *receiver, EventHandler handler);

/** Unregister a handler from an event
 * Call this to no longer receive raised events by a handler on a given
 * receiver instance.
 * @memberof Event
 * @param self the Event
 * @param receiver a pointer to the instance receiving this event (NULL for
 *     "static" handlers)
 * @param handler a pointer to a function handling the raised event
 */
DECLEXPORT void Event_unregister(
	Event *self, void *receiver, EventHandler handler);

/** Raise an event
 * Call this to notify all registered handlers about an event.
 * @memberof Event
 * @param self the Event
 * @param args a pointer to some additional event arguments
 */
DECLEXPORT void Event_raise(Event *self, const void *args);

/** Event destructor
 * @memberof Event
 * @param self the Event
 */
DECLEXPORT void Event_destroy(Event *self);

#endif
