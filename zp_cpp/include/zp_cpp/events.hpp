#pragma once

#include <forward_list>
#include <functional>

// ====================================================================================================================
// ====================================================================================================================
// ====================================================================================================================
// ====================================================================================================================
namespace zp
{
    template <typename T> class Event
    {
        public:
        void subscribe(const std::function<void(T)>& callback)
        {
            auto it = listeners.before_begin();
            for (auto next_it = listeners.begin(); next_it != listeners.end(); ++it, ++next_it)
            {
            }
            listeners.insert_after(it, callback);
        }

        void unsubscribe(const std::function<void(T)>& callback)
        {
            listeners.remove_if([&](const std::function<void(T)>& item) { return is_same_callback(callback, item); });
        }

        void trigger(const T& data)
        {
            auto listeners_copy = listeners;
            for (auto& listener : listeners_copy)
            {
                listener(data);
            }
        }

        private:
        static bool is_same_callback(const std::function<void(T)>& lhs, const std::function<void(T)>& rhs)
        {
            return lhs.target_type() == rhs.target_type() && lhs.template target<void(T)>() == rhs.template target<void(T)>();
        }

        std::forward_list<std::function<void(T)>> listeners;
    };

    struct Void
    {
    };

    using VoidEvent = Event<Void>;
}
