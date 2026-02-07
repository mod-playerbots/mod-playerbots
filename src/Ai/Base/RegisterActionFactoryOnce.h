#include <cassert>
#include <mutex>
#include <type_traits>
#include <memory>
#include <string>

#include "Action.h"
#include "ActionFactoryRegistry.h"

class Action;
class PlayerbotAI;

template <typename TAction>
std::unique_ptr<Action> CreateAction(PlayerbotAI* const botAI);

template <typename TAction>
void RegisterActionFactoryOnce(PlayerbotAI* const botAI)
{
    static_assert(std::is_base_of<Action, TAction>::value == true);

    static std::once_flag flag;

    std::call_once(
        flag,
        [botAI]()
        {
            assert(botAI != nullptr);

            ActionFactoryRegistry::RegisterByType(std::type_index(typeid(TAction)), &CreateAction<TAction>);

            std::unique_ptr<Action> probe = std::unique_ptr<Action>(new TAction(botAI));

            const std::string& name = probe->getName();

            ActionFactoryRegistry::RegisterByName(name, &CreateAction<TAction>);
        }
    );
}
