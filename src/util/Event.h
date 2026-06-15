#pragma once
#include <unordered_map>
#include <string>
#include <functional>
#include <memory>
struct EventData 
{};

template<typename T = EventData>
requires std::is_base_of_v<EventData, T>
struct EventHandler
{
	std::pair<void*, std::string> handlerKey;
	std::function<void(const T& data)> func;
};

template<typename T = EventData>
requires std::is_base_of_v<EventData, T>
struct EventSubscriber;

template<typename T = EventData>
requires std::is_base_of_v<EventData, T>
class Event
{
public:
	using Handler = EventHandler<T>;
	std::unique_ptr<EventSubscriber<T>> subscribe(void* owner, const std::string& key, const std::function<void(const T& data)>& func)
	{
		return std::make_unique<EventSubscriber<T>>(this, observers.emplace_back(std::make_pair(owner, key), func));
	}
	std::unique_ptr<EventSubscriber<T>> subscribe(void* owner, const std::string& key, std::function<void(const T& data)>&& func)
	{
		return std::make_unique<EventSubscriber<T>>(this, observers.emplace_back(std::make_pair(owner, key), std::move(func)));
	}
	void unsubscribe(std::pair<void*, std::string> handlerKey)
	{
		std::erase_if(observers, [&handlerKey](auto& handler) { 
			return handler.handlerKey == handlerKey; 
			});
	}
	void dispatch(const T& data)
	{
		for (Handler& handler : observers)
			handler.func(data);
	}
private:
	std::vector<Handler> observers;
};

template<typename T>
requires std::is_base_of_v<EventData, T>
struct EventSubscriber
{
	EventSubscriber(Event<T>* event, EventHandler<T>& handler) : event(event), handlerKey(handler.handlerKey) {}
	EventSubscriber(const EventSubscriber<T>& other) = delete;
	EventSubscriber& operator=(const EventSubscriber<T>& other) = delete;
	~EventSubscriber() { 
		if(event)
			event->unsubscribe(handlerKey);
	}
	Event<T>* event;
	std::pair<void*, std::string> handlerKey;
};