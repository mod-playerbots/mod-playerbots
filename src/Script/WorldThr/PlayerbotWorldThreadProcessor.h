/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_WORLD_THREAD_PROCESSOR_H
#define _PLAYERBOT_WORLD_THREAD_PROCESSOR_H

#include <memory>
#include <mutex>
#include <queue>

#include "Log.h"
#include "PlayerbotOperation.h"

/**
 * @brief Processes thread-unsafe bot operations in the world thread
 *
 * The PlayerbotWorldThreadProcessor manages a queue of operations that must be executed
 * in the world thread rather than map threads. This ensures thread safety for operations
 * like group modifications, LFG, guilds, battlegrounds, etc.
 *
 * Architecture:
 * - Map threads queue operations via QueueOperation()
 * - World thread processes operations via Update() (called from WorldScript::OnUpdate)
 * - Operations are processed in priority order
 * - Thread-safe queue protected by mutex
 *
 * Usage:
 *   auto op = std::make_unique<MyOperation>(botGuid, params);
 *   PlayerbotWorldThreadProcessor::instance().QueueOperation(std::move(op));
 */
class PlayerbotWorldThreadProcessor
{
public:
    static PlayerbotWorldThreadProcessor& instance()
    {
        static PlayerbotWorldThreadProcessor instance;

        return instance;
    }

    /**
     * @brief Update and process queued operations (called from world thread)
     *
     * This method should be called from WorldScript::OnUpdate hook, which runs in the world thread.
     * It processes a batch of queued operations.
     *
     * @param diff Time since last update in milliseconds
     */
    void Update(uint32 diff);

    /**
     * @brief Queue an operation for execution in the world thread
     *
     * Thread-safe method that can be called from any thread (typically map threads).
     * The operation will be executed later during Update().
     *
     * @param operation Unique pointer to the operation (ownership is transferred)
     * @return true if operation was queued, false if queue is full
     */
    bool QueueOperation(std::unique_ptr<PlayerbotOperation> operation);

    /**
     * @brief Get current queue size
     *
     * Thread-safe method for monitoring queue size.
     *
     * @return Number of operations waiting to be processed
     */
    uint32 GetQueueSize() const;

    /**
     * @brief Clear all queued operations
     *
     * Used during shutdown or emergency situations.
     */
    void ClearQueue();

    /**
     * @brief Get statistics about operation processing
     */
    struct Statistics
    {
        uint64 totalOperationsProcessed = 0;
        uint64 totalOperationsFailed = 0;
        uint64 totalOperationsSkipped = 0;
        uint32 currentQueueSize = 0;
        uint32 maxQueueSize = 0;
        uint32 averageExecutionTimeMs = 0;
    };

    Statistics GetStatistics() const;

    /**
     * @brief Enable/disable operation processing
     *
     * When disabled, operations are still queued but not processed.
     * Useful for testing or temporary suspension.
     *
     * @param enabled true to enable processing, false to disable
     */
    void SetEnabled(bool enabled) { m_enabled = enabled; }

    bool IsEnabled() const { return m_enabled; }

private:
    PlayerbotWorldThreadProcessor()
    : m_enabled(true),
    m_maxQueueSize(10000),
    m_batchSize(100),
    m_queueWarningThreshold(80),
    m_timeSinceLastUpdate(0),
    m_updateInterval(50)  // Process at least every 50ms
    {
        LOG_INFO("playerbots", "PlayerbotWorldThreadProcessor initialized");
    }
    ~PlayerbotWorldThreadProcessor()
    {
        this->ClearQueue();
    }

    /**
     * @brief Process a single batch of operations
     *
     * Extracts operations from queue and executes them.
     * Called internally by Update().
     */
    void ProcessBatch();

    /**
     * @brief Check if queue is approaching capacity
     *
     * Logs warning if queue is getting full.
     */
    void CheckQueueHealth();

    // Thread-safe queue
    mutable std::mutex m_queueMutex;
    std::queue<std::unique_ptr<PlayerbotOperation>> m_operationQueue;

    // Configuration
    bool m_enabled;
    uint32 m_maxQueueSize;           // Maximum operations in queue
    uint32 m_batchSize;              // Operations to process per Update()
    uint32 m_queueWarningThreshold;  // Warn when queue reaches this percentage

    // Statistics
    mutable std::mutex m_statsMutex;
    Statistics m_stats;

    // Timing
    uint32 m_timeSinceLastUpdate;
    uint32 m_updateInterval;  // Minimum ms between updates
};

#endif
