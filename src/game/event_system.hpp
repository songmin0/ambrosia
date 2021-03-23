#pragma once

#include <unordered_map>
#include <functional>
#include <vector>

// Manages the unique identifier for an event listener
class EventListenerInfo
{
public:
	EventListenerInfo() : id(INVALID_ID) {};

	void initialize() {
		static int nextID = 0;
		id = nextID++;
	}

	void uninitialize() {id = INVALID_ID;}

	int getID() const {return id;}
	bool isValid() const {return id != INVALID_ID;}

private:
	int id;
	static const int INVALID_ID = -1;
};

// SINGLETON EVENT SYSTEM
// Can register listeners for different kinds of events. Listeners are callback
// functions that will execute when their associated event occurs.
template <typename T>
class EventSystem
{
public:
	typedef std::function<void(const T&)> EventListener;

	// Returns the singleton instance of this system
	static EventSystem& instance()
	{
		static EventSystem eventSystem;
		return eventSystem;
	}

	// Send an event to all who are listening for that event
	void sendEvent(T event)
	{
   //Make sure that if we are looping through all the listeners that if someone decideds to unlisten it doesn't break everything because if we allow deletes while we are iterating over the map it breaks.
   sendingEvents = true;
		for (const auto& listener : eventListeners)
		{
			listener.second(event);
		}
  sendingEvents = false;
  for (auto info : listenersToDelete) {
    unregisterListener(info);
  }
  listenersToDelete.clear();
	}

	// Register a callback for events
	EventListenerInfo registerListener(const EventListener& callback)
	{
		EventListenerInfo listenerInfo;
		listenerInfo.initialize();

		eventListeners.insert({listenerInfo.getID(), callback});

		return listenerInfo;
	}

	// Remove an event listener using the unique identifier that was received
	// when registering it
	void unregisterListener(EventListenerInfo& info)
	{
   if (!sendingEvents) {
     eventListeners.erase(info.getID());
     info.uninitialize();
   }
   else {
     listenersToDelete.push_back(info);
   }
	}

private:
	// Default constructor
	EventSystem() = default;

	// Copy constructor
	EventSystem(const EventSystem&) = default;

	// Destructor
	~EventSystem() = default;

	// Assignment operator
	EventSystem& operator=(const EventSystem&) {return *this;}

	// Address-of operator
	EventSystem* operator&(const EventSystem&) {return nullptr;}

	// Event listeners
	std::unordered_map<int, EventListener> eventListeners;

 //listeners to be deleted
 std::vector<EventListenerInfo> listenersToDelete;

 bool sendingEvents = false;
};

/**
 * ---------------------------------------------------------------------------
 * ACCESSING THE EVENT SYSTEM:
 *
 * The event system is a singleton. I included it in common.hpp, so it should
 * be accessible from anywhere. To access it, we would use the `Instance()`
 * function and include the event type in angle brackets, like this:
 *
 * 		EventSystem<MouseClickEvent>.Instance()......
 * 		EventSystem<DeathEvent>.Instance()......
 * 		EventSystem<HitEvent>.Instance()......
 * 		etc.
 *
 *
 * ---------------------------------------------------------------------------
 * SENDING EVENTS:
 *
 * If you need a new kind of event, create it in Events.hpp and then send it
 * like this:
 *
 * 		MouseClickEvent event;
 * 		event.mousePos = {mousePosX, mousePosY};
 * 		EventSystem<MouseClickEvent>::Instance().SendEvent(event);
 *
 *
 * ---------------------------------------------------------------------------
 * EVENT LISTENER OVERVIEW:
 *
 * We can register and unregister "event listeners" using
 * EventSystem::RegisterListener and EventSystem::UnregisterListener. The
 * listeners are just callback functions.
 *
 * When you register a listener, the RegisterListener function gives you back
 * an EventListenerInfo object which is a unique identifier for the listener.
 * When it's time to unregister your listener, you need to pass that object to
 * UnregisterListener.
 *
 *
 * ---------------------------------------------------------------------------
 * USING MEMBER FUNCTIONS AS LISTENERS (RECOMMENDED):
 *
 * Create a member function to handle the event:
 *
 * 		void TurnSystem::OnMouseClick(const MouseClickEvent& event)
 * 		{
 * 			vec2 mousePosition = event.mousePos;
 *
 * 			...do something with the mouse position...
 * 		}
 *
 * Then pass that function to the event system:
 *
 * 		EventListenerInfo mouseClickListener =
 * 			EventSystem<MouseClickEvent>::Instance().RegisterListener(
 * 				std::bind(&TurnSystem::OnMouseClick, this, std::placeholders::_1));
 *
 * At some later time, unregister the listener:
 *
 * 		EventSystem<MouseClickEvent>::Instance().UnregisterListener(mouseClickListener);
 *
 *
 * ---------------------------------------------------------------------------
 * LISTENING FOR EVENTS USING LAMBDAS:
 *
 * Equivalent to using member functions, you could also create a lambda like
 * this:
 *
 * 		EventSystem<MouseClickEvent>::EventListener callback = [&](const MouseClickEvent& event) {
 * 			vec2 mousePosition = event.mousePos;
 *
 * 			...do something with the mouse position...
 * 		}
 *
 * Then pass it to the event system:
 *
 * 		EventListenerInfo mouseClickListener =
 * 			EventSystem<MouseClickEvent>::Instance().RegisterListener(callback);
 *
 * At some later time, unregister the listener:
 *
 * 		EventSystem<MouseClickEvent>::Instance().UnregisterListener(mouseClickListener);
 */
