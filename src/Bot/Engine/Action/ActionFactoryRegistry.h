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

    static void RegisterByType(const std::type_index key, const Factory factory)
    {
        std::unordered_map<std::type_index, Factory>& map = ActionFactoryRegistry::GetTypeMap();

        map.insert(std::make_pair(key, factory));
    }

    static void RegisterByName(const std::string& name, const Factory factory)
    {
        std::lock_guard<std::mutex> lock(ActionFactoryRegistry::GetMutex());

        ActionFactoryRegistry::GetNameMap().insert(std::make_pair(name, factory));
    }

    static Factory GetFactoryByType(const std::type_index key)
    {
        std::lock_guard<std::mutex> lock(ActionFactoryRegistry::GetMutex());

        std::unordered_map<std::type_index, Factory>& map = ActionFactoryRegistry::GetTypeMap();
        const std::unordered_map<std::type_index, Factory>::const_iterator it = map.find(key);

        if (it == map.end())
        {
            return static_cast<Factory>(nullptr);
        }

        return it->second;
    }

    static Factory GetFactoryByName(const std::string_view name)
    {
        std::lock_guard<std::mutex> lock(ActionFactoryRegistry::GetMutex());

        std::unordered_map<std::string, Factory>& map = ActionFactoryRegistry::GetNameMap();
        const std::unordered_map<std::string, Factory>::const_iterator it = map.find(std::string(name));

        if (it == map.end())
        {
            return static_cast<Factory>(nullptr);
        }

        return it->second;
    }

private:
    static std::unordered_map<std::type_index, Factory>& GetTypeMap()
    {
        static std::unordered_map<std::type_index, Factory> map;

        return map;
    }

    static std::unordered_map<std::string, Factory>& GetNameMap()
    {
        static std::unordered_map<std::string, Factory> map;

        return map;
    }

    static std::mutex& GetMutex()
    {
        static std::mutex mtx;

        return mtx;
    }
};
