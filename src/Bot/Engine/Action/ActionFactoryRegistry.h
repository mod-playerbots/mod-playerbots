#pragma once

#include <memory>
#include <typeindex>
#include <unordered_map>

class Action;
class PlayerbotAI;

class ActionFactoryRegistry
{
public:
    using Factory = std::unique_ptr<Action>(*)(PlayerbotAI* const botAI);

    static void Register(const std::type_index key, const Factory factory)
    {
        std::unordered_map<std::type_index, Factory>& map = ActionFactoryRegistry::GetMap();
        map.insert(std::make_pair(key, factory));
    }

    static Factory GetFactory(const std::type_index key)
    {
        std::unordered_map<std::type_index, Factory>& map = ActionFactoryRegistry::GetMap();
        const std::unordered_map<std::type_index, Factory>::const_iterator it = map.find(key);

        if (it == map.end())
        {
            return static_cast<Factory>(nullptr);
        }

        return it->second;
    }

    static std::unique_ptr<Action> Create(const std::type_index key, PlayerbotAI* const botAI)
    {
        const Factory factory = ActionFactoryRegistry::GetFactory(key);

        if (factory == static_cast<Factory>(nullptr))
        {
            return std::unique_ptr<Action>();
        }

        return factory(botAI);
    }

private:
    static std::unordered_map<std::type_index, Factory>& GetMap()
    {
        static std::unordered_map<std::type_index, Factory> map;
        return map;
    }
};
