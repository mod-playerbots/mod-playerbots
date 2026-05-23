/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#include "PaladinGreaterBlessingAction.h"

#include "AiObjectContext.h"
#include "AiFactory.h"
#include "Event.h"
#include "GenericBuffUtils.h"
#include "PaladinHelper.h"
#include "Playerbots.h"
#include "SharedDefines.h"
#include "SpellAuraEffects.h"
#include "Value.h"

#include <algorithm>
#include <limits>

namespace ai::gbless
{
namespace
{
    constexpr uint32 GREATER_BLESSING_ASSIGNMENT_CACHE_MS = 4 * 1000;
    constexpr uint32 GREATER_BLESSING_PENDING_ASSIGNMENT_CACHE_MS = 100;
    constexpr uint8 MAX_BLESSING_SLOTS = 4;
    constexpr uint8 MAX_CLASS_ID = 12;

    bool UsesRoleBucket(uint8 classId)
    {
        switch (classId)
        {
            case CLASS_WARRIOR:
            case CLASS_DEATH_KNIGHT:
            case CLASS_SHAMAN:
            case CLASS_PALADIN:
            case CLASS_DRUID:
                return true;
            default:
                return false;
        }
    }

    bool IsSameBucket(
        CachedBlessingBucketAssignment const& left,
        CachedBlessingBucketAssignment const& right)
    {
        return left.classId == right.classId &&
               left.byRole == right.byRole &&
               (!left.byRole || left.role == right.role);
    }

    int TalentScore(Player* player)
    {
        if (!player)
            return 0;

        int score = 0;
        if (player->HasAura(SPELL_IMPROVED_MIGHT_R1) ||
            player->HasAura(SPELL_IMPROVED_MIGHT_R2))
        {
            score += 2;
        }
        if (player->HasAura(SPELL_IMPROVED_WISDOM_R1) ||
            player->HasAura(SPELL_IMPROVED_WISDOM_R2))
        {
            score += 1;
        }

        return score;
    }

    int TalentMatchScore(Player* player, BaseBlessingCategory category)
    {
        if (!player)
            return std::numeric_limits<int>::min() / 4;

        if (category == BASE_SANCTUARY)
        {
            if (!player->HasSpell(ai::paladin::SPELL_BLESSING_OF_SANCTUARY))
                return std::numeric_limits<int>::min() / 4;

            return 2;
        }

        if (category == BASE_MIGHT &&
            (player->HasAura(SPELL_IMPROVED_MIGHT_R1) ||
             player->HasAura(SPELL_IMPROVED_MIGHT_R2)))
        {
            return 1;
        }
        if (category == BASE_WISDOM &&
            (player->HasAura(SPELL_IMPROVED_WISDOM_R1) ||
             player->HasAura(SPELL_IMPROVED_WISDOM_R2)))
        {
            return 1;
        }

        return 0;
    }

    struct DesiredBlessingSet
    {
        std::array<BaseBlessingCategory, MAX_BLESSING_SLOTS> ordered = {};
        std::array<bool, BASE_SANCTUARY + 1> wants = {};
        uint8 count = 0;
    };

    struct PresentBucket
    {
        uint8 classId = 0;
        RoleProfile role = ROLE_CASTER;
        bool byRole = false;
        DesiredBlessingSet desired;
    };

    DesiredBlessingSet BuildDesiredBlessingSet(
        RoleProfile role, uint8 paladinCount, bool anySanctuaryAvailable)
    {
        DesiredBlessingSet desired;

        auto const& priority = BASE_BLESSING_PRIORITIES[role];
        uint8 requestedCount = std::min<uint8>(paladinCount, MAX_BLESSING_SLOTS);

        for (uint8 index = 0;
             index < MAX_BLESSING_SLOTS && desired.count < requestedCount;
             ++index)
        {
            BaseBlessingCategory category = priority.priorities[index];
            if (category == BASE_NONE)
                continue;

            if (category == BASE_SANCTUARY && !anySanctuaryAvailable)
                category = BASE_KINGS;

            if (category == BASE_NONE || desired.wants[category])
                continue;

            desired.ordered[desired.count++] = category;
            desired.wants[category] = true;
        }

        return desired;
    }

    std::vector<BaseBlessingCategory> OrderedCommonBases(
        std::vector<PresentBucket const*> const& classBuckets,
        std::array<bool, BASE_SANCTUARY + 1> const& commonBases)
    {
        std::vector<BaseBlessingCategory> ordered;

        for (PresentBucket const* bucket : classBuckets)
        {
            for (uint8 index = 0; index < bucket->desired.count; ++index)
            {
                BaseBlessingCategory category = bucket->desired.ordered[index];
                if (!commonBases[category])
                    continue;

                if (std::find(ordered.begin(), ordered.end(), category) == ordered.end())
                    ordered.push_back(category);
            }
        }

        return ordered;
    }

    bool ComputeBestOwners(
        std::vector<BaseBlessingCategory> const& categories,
        std::vector<Player*> const& botPaladins,
        std::vector<int> const& candidatePaladins,
        std::vector<int>& outOwners,
        int* outScore = nullptr)
    {
        outOwners.clear();
        if (categories.empty())
        {
            if (outScore)
                *outScore = 0;
            return true;
        }

        std::vector<int> currentOwners(categories.size(), -1);
        std::vector<int> bestOwners(categories.size(), -1);
        std::vector<bool> used(candidatePaladins.size(), false);
        int bestScore = std::numeric_limits<int>::min();
        int bestTalentCost = std::numeric_limits<int>::max();
        bool found = false;

        auto search = [&](auto&& self, size_t position, int score, int talentCost) -> void
        {
            if (position >= categories.size())
            {
                if (!found || score > bestScore ||
                    (score == bestScore && talentCost < bestTalentCost) ||
                    (score == bestScore &&
                     talentCost == bestTalentCost &&
                     std::lexicographical_compare(currentOwners.begin(), currentOwners.end(),
                                                  bestOwners.begin(), bestOwners.end())))
                {
                    found = true;
                    bestScore = score;
                    bestTalentCost = talentCost;
                    bestOwners = currentOwners;
                }
                return;
            }

            for (size_t candidateIndex = 0;
                 candidateIndex < candidatePaladins.size(); ++candidateIndex)
            {
                if (used[candidateIndex])
                    continue;

                int paladinIndex = candidatePaladins[candidateIndex];
                int matchScore = TalentMatchScore(botPaladins[paladinIndex], categories[position]);
                if (matchScore <= std::numeric_limits<int>::min() / 8)
                    continue;

                used[candidateIndex] = true;
                currentOwners[position] = paladinIndex;
                self(self, position + 1, score + matchScore,
                     talentCost + TalentScore(botPaladins[paladinIndex]));
                currentOwners[position] = -1;
                used[candidateIndex] = false;
            }
        };

        search(search, 0, 0, 0);

        if (!found)
            return false;

        outOwners = std::move(bestOwners);
        if (outScore)
            *outScore = bestScore;
        return true;
    }

    uint8 CategoryMask(BaseBlessingCategory category)
    {
        return static_cast<uint8>(1u << static_cast<uint8>(category));
    }

    uint8 CountSetBits(uint8 mask)
    {
        uint8 count = 0;
        while (mask)
        {
            count += mask & 1u;
            mask >>= 1;
        }

        return count;
    }

    int PriorityWeight(uint8 orderedIndex)
    {
        return 1 << (MAX_BLESSING_SLOTS - orderedIndex - 1);
    }

    bool ComputeOwnersForClassPlan(
        std::vector<BaseBlessingCategory> const& classWideBases,
        std::vector<std::vector<BaseBlessingCategory>> const& exclusiveBasesByBucket,
        std::vector<Player*> const& botPaladins,
        std::vector<int> const& allPaladins,
        std::vector<int>& outClassWideOwners,
        std::vector<std::vector<int>>& outExclusiveOwnersByBucket,
        int& outOwnerScore,
        int& outCommonTalentCost)
    {
        outClassWideOwners.clear();
        outExclusiveOwnersByBucket.clear();
        outOwnerScore = std::numeric_limits<int>::min();
        outCommonTalentCost = std::numeric_limits<int>::max();

        std::vector<int> currentClassWideOwners(classWideBases.size(), -1);
        std::vector<int> bestClassWideOwners(classWideBases.size(), -1);
        std::vector<bool> used(allPaladins.size(), false);
        std::vector<std::vector<int>> bestExclusiveOwnersByBucket(exclusiveBasesByBucket.size());
        bool found = false;

        auto search = [&](auto&& self, size_t position, int commonScore, int commonTalentCost) -> void
        {
            if (position >= classWideBases.size())
            {
                std::vector<int> availablePaladins;
                availablePaladins.reserve(allPaladins.size());
                for (size_t candidateIndex = 0; candidateIndex < allPaladins.size(); ++candidateIndex)
                {
                    if (!used[candidateIndex])
                        availablePaladins.push_back(allPaladins[candidateIndex]);
                }

                int totalOwnerScore = commonScore;
                std::vector<std::vector<int>> exclusiveOwnersByBucket(exclusiveBasesByBucket.size());
                for (size_t bucketIndex = 0; bucketIndex < exclusiveBasesByBucket.size(); ++bucketIndex)
                {
                    int exclusiveScore = 0;
                    if (!ComputeBestOwners(
                            exclusiveBasesByBucket[bucketIndex], botPaladins, availablePaladins,
                            exclusiveOwnersByBucket[bucketIndex], &exclusiveScore))
                    {
                        return;
                    }

                    totalOwnerScore += exclusiveScore;
                }

                if (!found || totalOwnerScore > outOwnerScore ||
                    (totalOwnerScore == outOwnerScore && commonTalentCost < outCommonTalentCost) ||
                    (totalOwnerScore == outOwnerScore &&
                     commonTalentCost == outCommonTalentCost &&
                     std::lexicographical_compare(currentClassWideOwners.begin(), currentClassWideOwners.end(),
                                                  bestClassWideOwners.begin(), bestClassWideOwners.end())))
                {
                    found = true;
                    outOwnerScore = totalOwnerScore;
                    outCommonTalentCost = commonTalentCost;
                    bestClassWideOwners = currentClassWideOwners;
                    bestExclusiveOwnersByBucket = std::move(exclusiveOwnersByBucket);
                }

                return;
            }

            for (size_t candidateIndex = 0; candidateIndex < allPaladins.size(); ++candidateIndex)
            {
                if (used[candidateIndex])
                    continue;

                int const paladinIndex = allPaladins[candidateIndex];
                int const matchScore = TalentMatchScore(botPaladins[paladinIndex], classWideBases[position]);
                if (matchScore <= std::numeric_limits<int>::min() / 8)
                    continue;

                used[candidateIndex] = true;
                currentClassWideOwners[position] = paladinIndex;
                self(self, position + 1, commonScore + matchScore,
                     commonTalentCost + TalentScore(botPaladins[paladinIndex]));
                currentClassWideOwners[position] = -1;
                used[candidateIndex] = false;
            }
        };

        search(search, 0, 0, 0);

        if (!found)
            return false;

        outClassWideOwners = std::move(bestClassWideOwners);
        outExclusiveOwnersByBucket = std::move(bestExclusiveOwnersByBucket);
        return true;
    }

    bool ComputeBestClassAssignments(
        std::vector<PresentBucket const*> const& classBuckets,
        uint8 activePaladinCount,
        std::vector<Player*> const& botPaladins,
        std::vector<int> const& allPaladins,
        std::vector<int>& outClassWideOwners,
        std::vector<std::vector<int>>& outExclusiveOwnersByBucket,
        std::vector<BaseBlessingCategory>& outClassWideBases,
        std::vector<std::vector<BaseBlessingCategory>>& outExclusiveBasesByBucket)
    {
        outClassWideOwners.clear();
        outExclusiveOwnersByBucket.clear();
        outClassWideBases.clear();
        outExclusiveBasesByBucket.clear();

        uint8 unionMask = 0;
        for (PresentBucket const* bucket : classBuckets)
        {
            for (uint8 index = 0; index < bucket->desired.count; ++index)
                unionMask |= CategoryMask(bucket->desired.ordered[index]);
        }

        uint8 const selectedCount = std::min<uint8>(activePaladinCount, CountSetBits(unionMask));
        if (!selectedCount)
            return false;

        int bestCoverageScore = std::numeric_limits<int>::min();
        int bestOwnerScore = std::numeric_limits<int>::min();
        int bestCommonTalentCost = std::numeric_limits<int>::max();
        std::vector<int> bestClassWideOwners;
        std::vector<std::vector<int>> bestExclusiveOwnersByBucket;
        std::vector<BaseBlessingCategory> bestClassWideBases;
        std::vector<std::vector<BaseBlessingCategory>> bestExclusiveBasesByBucket;
        uint8 const categoryMaskLimit =
            static_cast<uint8>(1u << (static_cast<uint8>(BASE_SANCTUARY) + 1u));
        bool found = false;

        for (uint8 selectedMask = 0; selectedMask < categoryMaskLimit; ++selectedMask)
        {
            if ((selectedMask & unionMask) != selectedMask || CountSetBits(selectedMask) != selectedCount)
                continue;

            int coverageScore = 0;
            std::array<bool, BASE_SANCTUARY + 1> commonBases = {};
            std::vector<std::vector<BaseBlessingCategory>> exclusiveBasesByBucket(classBuckets.size());
            bool valid = true;

            for (size_t bucketIndex = 0; bucketIndex < classBuckets.size(); ++bucketIndex)
            {
                PresentBucket const* bucket = classBuckets[bucketIndex];
                bool bucketCovered = false;
                for (uint8 index = 0; index < bucket->desired.count; ++index)
                {
                    BaseBlessingCategory category = bucket->desired.ordered[index];
                    if (!(selectedMask & CategoryMask(category)))
                        continue;

                    bucketCovered = true;
                    coverageScore += PriorityWeight(index);
                    exclusiveBasesByBucket[bucketIndex].push_back(category);
                }

                if (!bucketCovered)
                {
                    valid = false;
                    break;
                }
            }

            if (!valid)
                continue;

            for (uint8 baseValue = BASE_MIGHT; baseValue <= BASE_SANCTUARY; ++baseValue)
            {
                BaseBlessingCategory category = static_cast<BaseBlessingCategory>(baseValue);
                if (!(selectedMask & CategoryMask(category)))
                    continue;

                commonBases[category] = std::all_of(classBuckets.begin(), classBuckets.end(),
                                                    [&](PresentBucket const* bucket)
                                                    {
                                                        return bucket->desired.wants[category];
                                                    });
            }

            std::vector<BaseBlessingCategory> classWideBases =
                OrderedCommonBases(classBuckets, commonBases);

            for (size_t bucketIndex = 0; bucketIndex < classBuckets.size(); ++bucketIndex)
            {
                auto& exclusiveBases = exclusiveBasesByBucket[bucketIndex];
                exclusiveBases.erase(
                    std::remove_if(exclusiveBases.begin(), exclusiveBases.end(),
                                   [&](BaseBlessingCategory category)
                                   {
                                       return commonBases[category];
                                   }),
                    exclusiveBases.end());
            }

            std::vector<int> classWideOwners;
            std::vector<std::vector<int>> exclusiveOwnersByBucket;
            int ownerScore = 0;
            int commonTalentCost = 0;
            if (!ComputeOwnersForClassPlan(
                    classWideBases, exclusiveBasesByBucket, botPaladins, allPaladins,
                    classWideOwners, exclusiveOwnersByBucket, ownerScore, commonTalentCost))
            {
                continue;
            }

            if (!found || coverageScore > bestCoverageScore ||
                (coverageScore == bestCoverageScore && ownerScore > bestOwnerScore) ||
                (coverageScore == bestCoverageScore && ownerScore == bestOwnerScore &&
                 commonTalentCost < bestCommonTalentCost) ||
                (coverageScore == bestCoverageScore && ownerScore == bestOwnerScore &&
                 commonTalentCost == bestCommonTalentCost &&
                 std::lexicographical_compare(classWideOwners.begin(), classWideOwners.end(),
                                              bestClassWideOwners.begin(), bestClassWideOwners.end())))
            {
                found = true;
                bestCoverageScore = coverageScore;
                bestOwnerScore = ownerScore;
                bestCommonTalentCost = commonTalentCost;
                bestClassWideOwners = std::move(classWideOwners);
                bestExclusiveOwnersByBucket = std::move(exclusiveOwnersByBucket);
                bestClassWideBases = std::move(classWideBases);
                bestExclusiveBasesByBucket = std::move(exclusiveBasesByBucket);
            }
        }

        if (!found)
            return false;

        outClassWideOwners = std::move(bestClassWideOwners);
        outExclusiveOwnersByBucket = std::move(bestExclusiveOwnersByBucket);
        outClassWideBases = std::move(bestClassWideBases);
        outExclusiveBasesByBucket = std::move(bestExclusiveBasesByBucket);
        return true;
    }

    void AddUniqueAssignment(
        std::vector<CachedBlessingBucketAssignment>& assignments,
        CachedBlessingBucketAssignment const& assignment)
    {
        auto existing = std::find_if(assignments.begin(), assignments.end(),
                                     [&](CachedBlessingBucketAssignment const& cachedAssignment)
                                     {
                                         return cachedAssignment.blessing == assignment.blessing &&
                                                IsSameBucket(cachedAssignment, assignment);
                                     });

        if (existing == assignments.end())
            assignments.push_back(assignment);
    }

    bool ComputeGreaterBlessingAssignments(
        PlayerbotAI* botAI, std::vector<CachedBlessingBucketAssignment>& outAssignments)
    {
        Player* bot = botAI->GetBot();
        Group* group = bot->GetGroup();
        if (!IsEligibleGroupForAutoBlessings(group))
            return false;

        std::vector<Player*> botPaladins;
        struct RaidMember
        {
            Player* player;
            RoleProfile role;
        };
        std::vector<RaidMember> raidMembers;

        for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
        {
            Player* player = ref->GetSource();
            if (!player || !player->IsInWorld())
                continue;

            if (player->getClass() == CLASS_PALADIN && GET_PLAYERBOT_AI(player))
                botPaladins.push_back(player);

            // Important: keep dead members in the assignment model so revive does
            // not temporarily delete an entire blessing bucket from the cache.
            raidMembers.push_back({player, ResolveRoleProfile(player)});
        }

        if (botPaladins.empty())
            return false;

        bool anySanctuaryAvailable = false;
        for (Player* paladin : botPaladins)
        {
            if (paladin && paladin->HasSpell(ai::paladin::SPELL_BLESSING_OF_SANCTUARY))
            {
                anySanctuaryAvailable = true;
                break;
            }
        }

        std::sort(botPaladins.begin(), botPaladins.end(),
                  [](Player* a, Player* b)
                  {
                      int sa = TalentScore(a);
                      int sb = TalentScore(b);
                      if (sa != sb)
                          return sa > sb;
                      return a->GetGUID() < b->GetGUID();
                  });

        uint8 activePaladinCount =
            std::min<uint8>(static_cast<uint8>(botPaladins.size()), MAX_BLESSING_SLOTS);

        int mySlot = -1;
        for (size_t i = 0; i < botPaladins.size(); ++i)
        {
            if (botPaladins[i]->GetGUID() == bot->GetGUID())
            {
                mySlot = static_cast<int>(i);
                break;
            }
        }
        if (mySlot < 0 || mySlot >= activePaladinCount)
            return false;

        std::vector<PresentBucket> buckets;
        buckets.reserve(raidMembers.size());

        for (auto const& member : raidMembers)
        {
            uint8 classId = member.player->getClass();
            if (classId >= MAX_CLASS_ID)
                continue;

            PresentBucket bucket;
            bucket.classId = classId;
            bucket.role = member.role;
            bucket.byRole = UsesRoleBucket(classId);
            bucket.desired = BuildDesiredBlessingSet(
                member.role, activePaladinCount, anySanctuaryAvailable);

            if (!bucket.byRole)
                bucket.role = ROLE_CASTER;

            if (!bucket.desired.count)
                continue;

            auto existing = std::find_if(buckets.begin(), buckets.end(),
                                         [&](PresentBucket const& other)
                                         {
                                             return other.classId == bucket.classId &&
                                                    other.byRole == bucket.byRole &&
                                                    (!bucket.byRole || other.role == bucket.role);
                                         });

            if (existing == buckets.end())
                buckets.push_back(bucket);
        }

        if (buckets.empty())
            return false;

        std::vector<int> allPaladins;
        allPaladins.reserve(activePaladinCount);
        for (uint8 paladinIndex = 0; paladinIndex < activePaladinCount; ++paladinIndex)
            allPaladins.push_back(paladinIndex);

        outAssignments.clear();

        for (uint8 classId = 0; classId < MAX_CLASS_ID; ++classId)
        {
            std::vector<PresentBucket const*> classBuckets;
            for (PresentBucket const& bucket : buckets)
            {
                if (bucket.classId == classId)
                    classBuckets.push_back(&bucket);
            }

            if (classBuckets.empty())
                continue;

            std::vector<int> classWideOwners;
            std::vector<std::vector<int>> exclusiveOwnersByBucket;
            std::vector<BaseBlessingCategory> classWideBases;
                std::vector<std::vector<BaseBlessingCategory>> exclusiveBasesByBucket;
            if (!ComputeBestClassAssignments(
                    classBuckets, activePaladinCount, botPaladins, allPaladins,
                    classWideOwners, exclusiveOwnersByBucket, classWideBases,
                    exclusiveBasesByBucket))
                return false;

            for (size_t index = 0; index < classWideBases.size(); ++index)
            {
                if (classWideOwners[index] != mySlot)
                    continue;

                CachedBlessingBucketAssignment assignment;
                assignment.classId = classId;
                assignment.role = classBuckets.front()->role;
                assignment.byRole = false;
                assignment.blessing = ToGreaterVariant(classWideBases[index]);
                AddUniqueAssignment(outAssignments, assignment);
            }

            for (size_t bucketIndex = 0; bucketIndex < classBuckets.size(); ++bucketIndex)
            {
                PresentBucket const* bucket = classBuckets[bucketIndex];
                std::vector<BaseBlessingCategory> const& exclusiveBases = exclusiveBasesByBucket[bucketIndex];
                std::vector<int> const& exclusiveOwners = exclusiveOwnersByBucket[bucketIndex];

                for (size_t index = 0; index < exclusiveBases.size(); ++index)
                {
                    if (exclusiveOwners[index] != mySlot)
                        continue;

                    CachedBlessingBucketAssignment assignment;
                    assignment.classId = bucket->classId;
                    assignment.role = bucket->role;
                    assignment.byRole = true;
                    assignment.blessing = ToSingleVariant(exclusiveBases[index]);
                    AddUniqueAssignment(outAssignments, assignment);
                }
            }
        }

        return !outAssignments.empty();
    }

    class GreaterBlessingAssignmentsValue : public CalculatedValue<CachedBlessingAssignments>
    {
    public:
        GreaterBlessingAssignmentsValue(PlayerbotAI* botAI)
            : CalculatedValue<CachedBlessingAssignments>(
                  botAI, "greater blessing assignments", GREATER_BLESSING_ASSIGNMENT_CACHE_MS) {}

    protected:
        CachedBlessingAssignments Calculate() override
        {
            CachedBlessingAssignments cached;

            Player* bot = botAI->GetBot();
            Group* group = bot ? bot->GetGroup() : nullptr;
            cached.groupKey = group ? group->GetLeaderGUID().GetCounter() : 0;

            std::vector<CachedBlessingBucketAssignment> assignments;
            if (!ComputeGreaterBlessingAssignments(botAI, assignments))
                return cached;

            cached.valid = true;
            cached.assignments = std::move(assignments);

            return cached;
        }
    };

    bool GetCachedAssignments(
        AiObjectContext* context,
        Player* bot,
        std::vector<CachedBlessingBucketAssignment>& outAssignments)
    {
        Group* group = bot ? bot->GetGroup() : nullptr;
        uint32 const groupKey = group ? group->GetLeaderGUID().GetCounter() : 0;

        Value<CachedBlessingAssignments>* cacheValue =
            context->GetValue<CachedBlessingAssignments>("greater blessing assignments");
        if (!cacheValue)
            return false;

        CachedBlessingAssignments cachedAssignments = cacheValue->Get();
        if (cachedAssignments.groupKey != groupKey)
        {
            cacheValue->Reset();
            cachedAssignments = cacheValue->Get();
        }

        if (!cachedAssignments.valid || cachedAssignments.groupKey != groupKey)
            return false;

        outAssignments = cachedAssignments.assignments;
        return true;
    }

    static bool HasMyExactBlessing(PlayerbotAI* botAI, Unit* target, BlessingType type)
    {
        std::string name = BlessingSpellName(type);
        if (name.empty())
            return false;

        return botAI->GetAura(name, target, true) != nullptr;
    }

    static bool UsesBlessingStrengthGate(BaseBlessingCategory category)
    {
        return category == BASE_MIGHT || category == BASE_WISDOM;
    }

    static AuraType BlessingStrengthAuraType(BaseBlessingCategory category)
    {
        return category == BASE_MIGHT ? SPELL_AURA_MOD_ATTACK_POWER : SPELL_AURA_MOD_POWER_REGEN;
    }

    static int32 GetAuraStrength(Aura const* aura, AuraType auraType)
    {
        if (!aura)
            return 0;

        int32 amount = 0;
        for (uint8 effect = 0; effect < MAX_SPELL_EFFECTS; ++effect)
        {
            AuraEffect* auraEffect = aura->GetEffect(effect);
            if (!auraEffect || auraEffect->GetAuraType() != auraType)
                continue;

            amount = std::max(amount, auraEffect->GetAmount());
        }

        return amount;
    }

    static int32 GetExistingBlessingStrength(
        PlayerbotAI* botAI, Unit* target, BaseBlessingCategory category)
    {
        if (!UsesBlessingStrengthGate(category))
            return 0;

        AuraType auraType = BlessingStrengthAuraType(category);
        int32 strongestAmount = 0;

        for (BlessingType type : {ToSingleVariant(category), ToGreaterVariant(category)})
        {
            Aura* aura = botAI->GetAura(BlessingSpellName(type), target);
            strongestAmount = std::max(strongestAmount, GetAuraStrength(aura, auraType));
        }

        return strongestAmount;
    }

    static bool HasSameFamilyBlessing(
        PlayerbotAI* botAI, Unit* target, BaseBlessingCategory category)
    {
        for (BlessingType type : {ToSingleVariant(category), ToGreaterVariant(category)})
        {
            if (botAI->GetAura(BlessingSpellName(type), target) != nullptr)
                return true;
        }

        return false;
    }

    static int32 GetBlessingCastStrength(Player* caster, BlessingType type, uint32 spellId)
    {
        if (!caster || !spellId)
            return 0;

        BaseBlessingCategory category = BaseBlessingOf(type);
        if (!UsesBlessingStrengthGate(category))
            return 0;

        SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(spellId);
        if (!spellInfo)
            return 0;

        AuraType auraType = BlessingStrengthAuraType(category);
        int32 amount = 0;
        for (uint8 effect = 0; effect < MAX_SPELL_EFFECTS; ++effect)
        {
            if (spellInfo->Effects[effect].ApplyAuraName != auraType)
                continue;

            amount = std::max(amount, spellInfo->Effects[effect].BasePoints + 1);
        }

        if (amount <= 0)
            return 0;

        switch (category)
        {
            case BASE_MIGHT:
                if (caster->HasAura(SPELL_IMPROVED_MIGHT_R2))
                    return amount * 125 / 100;
                if (caster->HasAura(SPELL_IMPROVED_MIGHT_R1))
                    return amount * 112 / 100;
                break;
            case BASE_WISDOM:
                if (caster->HasAura(SPELL_IMPROVED_WISDOM_R2))
                    return amount * 120 / 100;
                if (caster->HasAura(SPELL_IMPROVED_WISDOM_R1))
                    return amount * 110 / 100;
                break;
            default:
                break;
        }

        return amount;
    }

    static bool HasEquivalentOrStrongerSameFamilyBlessing(
        PlayerbotAI* botAI, Unit* target, BlessingType castType, uint32 spellId)
    {
        BaseBlessingCategory category = BaseBlessingOf(castType);
        if (!UsesBlessingStrengthGate(category))
            return HasSameFamilyBlessing(botAI, target, category);

        int32 castStrength = GetBlessingCastStrength(botAI->GetBot(), castType, spellId);
        if (castStrength <= 0)
            return false;

        return GetExistingBlessingStrength(botAI, target, category) >= castStrength;
    }

    static bool MatchesBucket(Player* player, CachedBlessingBucketAssignment const& assignment)
    {
        if (!player || player->getClass() != assignment.classId)
            return false;

        return !assignment.byRole || ResolveRoleProfile(player) == assignment.role;
    }

    static Player* FindMissingBlessingTarget(
        PlayerbotAI* botAI, CachedBlessingBucketAssignment const& assignment,
        BlessingType castType, uint32 spellId, std::string const& spellName)
    {
        Player* bot = botAI->GetBot();
        Group* group = bot->GetGroup();
        if (!group)
            return nullptr;

        for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
        {
            Player* player = ref->GetSource();
            if (!player || !player->IsInWorld() || !player->IsAlive())
                continue;

            if (!(player->IsInSameGroupWith(bot) || player->IsInSameRaidWith(bot)))
                continue;

            if (!MatchesBucket(player, assignment) ||
                HasMyExactBlessing(botAI, player, castType) ||
                HasEquivalentOrStrongerSameFamilyBlessing(botAI, player, castType, spellId) ||
                !botAI->CanCastSpell(spellName, player))
            {
                continue;
            }

            return player;
        }

        return nullptr;
    }

    bool FindPendingAssignmentFromAssignments(
        PlayerbotAI* botAI,
        std::vector<CachedBlessingBucketAssignment> const& assignments,
        GreaterBlessingPlayerAssignment& outAssignment,
        std::string& outSpellName)
    {
        Player* bot = botAI->GetBot();

        for (auto const& assigned : assignments)
        {
            if (assigned.blessing == BLESSING_NONE)
                continue;

            BlessingType castType = assigned.blessing;
            std::string spellName = BlessingSpellName(castType);
            if (spellName.empty())
                continue;

            if (IsGreaterVariant(castType))
            {
                uint32 spellId =
                    botAI->GetAiObjectContext()->GetValue<uint32>("spell id", spellName)->Get();
                if (!spellId || !ai::buff::HasRequiredReagents(bot, spellId))
                {
                    castType = ToSingleVariant(castType);
                    spellName = BlessingSpellName(castType);
                    if (spellName.empty())
                        continue;
                }
            }

            uint32 spellId =
                botAI->GetAiObjectContext()->GetValue<uint32>("spell id", spellName)->Get();
            if (!spellId)
                continue;

            Player* target = FindMissingBlessingTarget(
                botAI, assigned, castType, spellId, spellName);
            if (!target)
                continue;

            outAssignment = {target, assigned.blessing};
            outSpellName = spellName;
            return true;
        }

        return false;
    }

    class GreaterBlessingPendingAssignmentValue : public CalculatedValue<CachedPendingBlessingAssignment>
    {
    public:
        GreaterBlessingPendingAssignmentValue(PlayerbotAI* botAI)
            : CalculatedValue<CachedPendingBlessingAssignment>(
                  botAI, "greater blessing pending assignment",
                  GREATER_BLESSING_PENDING_ASSIGNMENT_CACHE_MS) {}

    protected:
        CachedPendingBlessingAssignment Calculate() override
        {
            CachedPendingBlessingAssignment cached;
            AiObjectContext* aiObjectContext = botAI->GetAiObjectContext();

            Player* bot = botAI->GetBot();
            Group* group = bot ? bot->GetGroup() : nullptr;
            cached.groupKey = group ? group->GetLeaderGUID().GetCounter() : 0;

            std::vector<CachedBlessingBucketAssignment> assignments;
            if (!GetCachedAssignments(aiObjectContext, bot, assignments))
                return cached;

            if (!FindPendingAssignmentFromAssignments(
                    botAI, assignments, cached.assignment, cached.spellName))
            {
                return cached;
            }

            cached.valid = true;
            return cached;
        }
    };
}

UntypedValue* greater_blessing_assignments_value(PlayerbotAI* botAI)
{
    return new GreaterBlessingAssignmentsValue(botAI);
}

UntypedValue* greater_blessing_pending_assignment_value(PlayerbotAI* botAI)
{
    return new GreaterBlessingPendingAssignmentValue(botAI);
}

bool IsEligibleGroupForAutoBlessings(Group const* group)
{
    if (!group)
        return false;

    switch (sPlayerbotAIConfig.autoGreaterBlessings)
    {
        case AutoPartyBuffMode::RAID_ONLY:
            return group->isRaidGroup();
        case AutoPartyBuffMode::GROUP_OR_RAID:
            return true;
        case AutoPartyBuffMode::DISABLED:
        default:
            return false;
    }
}

bool IsAutoGreaterBlessingActive(Player const* bot)
{
    return bot && IsEligibleGroupForAutoBlessings(bot->GetGroup());
}

}

CastGreaterBlessingAssignmentAction::CastGreaterBlessingAssignmentAction(
    PlayerbotAI* botAI) : Action(botAI, "cast greater blessing assignment") {}

bool CastGreaterBlessingAssignmentAction::isUseful()
{
    return ai::gbless::IsAutoGreaterBlessingActive(bot);
}

bool CastGreaterBlessingAssignmentAction::HasPendingAssignment()
{
    ai::gbless::GreaterBlessingPlayerAssignment assignment;
    std::string spellName;
    return FindPendingAssignment(assignment, spellName);
}

bool CastGreaterBlessingAssignmentAction::Execute(Event /*event*/)
{
    ai::gbless::GreaterBlessingPlayerAssignment assignment;
    std::string spellName;
    if (!FindPendingAssignment(assignment, spellName))
        return false;

    uint32 finalId = AI_VALUE2(uint32, "spell id", spellName);
    if (!finalId)
        return false;

    return botAI->CastSpell(spellName, assignment.player);
}

bool CastGreaterBlessingAssignmentAction::FindPendingAssignment(
    ai::gbless::GreaterBlessingPlayerAssignment& outAssignment,
    std::string& outSpellName)
{
    Group* group = bot->GetGroup();
    uint32 const groupKey = group ? group->GetLeaderGUID().GetCounter() : 0;

    Value<ai::gbless::CachedPendingBlessingAssignment>* pendingValue =
        context->GetValue<ai::gbless::CachedPendingBlessingAssignment>("greater blessing pending assignment");
    if (!pendingValue)
        return false;

    ai::gbless::CachedPendingBlessingAssignment pendingAssignment = pendingValue->Get();
    if (pendingAssignment.groupKey != groupKey)
    {
        pendingValue->Reset();
        pendingAssignment = pendingValue->Get();
    }

    if (!pendingAssignment.valid || pendingAssignment.groupKey != groupKey)
        return false;

    outAssignment = pendingAssignment.assignment;
    outSpellName = pendingAssignment.spellName;
    return true;
}

bool CastGreaterBlessingAssignmentAction::GetAssignments(
    std::vector<ai::gbless::CachedBlessingBucketAssignment>& outAssignments)
{
    return ai::gbless::GetCachedAssignments(context, bot, outAssignments);
}
