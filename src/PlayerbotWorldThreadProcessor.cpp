/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#include "PlayerbotWorldThreadProcessor.h"

#include "Log.h"
#include "PlayerbotAIConfig.h"

#include <algorithm>

PlayerbotWorldThreadProcessor::PlayerbotWorldThreadProcessor()
    : m_enabled(true), m_maxQueueSize(10000), m_batchSize(100), m_queueWarningThreshold(80),
      m_timeSinceLastUpdate(0), m_updateInterval(50)  // Process at least every 50ms
{
    LOG_INFO("playerbots", "PlayerbotWorldThreadProcessor initialized");
}

PlayerbotWorldThreadProcessor::~PlayerbotWorldThreadProcessor() { ClearQueue(); }

PlayerbotWorldThreadProcessor* PlayerbotWorldThreadProcessor::instance()
{
    static PlayerbotWorldThreadProcessor instance;
    return &instance;
}

void PlayerbotWorldThreadProcessor::Update(uint32 diff)
{
    if (!m_enabled)
        return;

    // Accumulate time
    m_timeSinceLastUpdate += diff;

    // Don't process too frequently to reduce overhead
    if (m_timeSinceLastUpdate < m_updateInterval)
        return;

    m_timeSinceLastUpdate = 0;

    // Check queue health (warn if getting full)
    CheckQueueHealth();

    // Process a batch of operations
    ProcessBatch();
}

bool PlayerbotWorldThreadProcessor::QueueOperation(std::unique_ptr<PlayerbotOperation> operation)
{
    if (!operation)
    {
        LOG_ERROR("playerbots", "Attempted to queue null operation");
        return false;
    }

    std::lock_guard<std::mutex> lock(m_queueMutex);

    // Check if queue is full
    if (m_operationQueue.size() >= m_maxQueueSize)
    {
        LOG_ERROR("playerbots",
                  "PlayerbotWorldThreadProcessor queue is full ({} operations). Dropping operation: {}",
                  m_maxQueueSize, operation->GetName());

        std::lock_guard<std::mutex> statsLock(m_statsMutex);
        m_stats.totalOperationsSkipped++;
        return false;
    }

    // Queue the operation
    m_operationQueue.push(std::move(operation));

    // Update statistics
    {
        std::lock_guard<std::mutex> statsLock(m_statsMutex);
        m_stats.currentQueueSize = static_cast<uint32>(m_operationQueue.size());
        m_stats.maxQueueSize = std::max(m_stats.maxQueueSize, m_stats.currentQueueSize);
    }

    return true;
}

void PlayerbotWorldThreadProcessor::ProcessBatch()
{
    // Extract a batch of operations from the queue
    std::vector<std::unique_ptr<PlayerbotOperation>> batch;
    batch.reserve(m_batchSize);

    {
        std::lock_guard<std::mutex> lock(m_queueMutex);

        // Extract up to batchSize operations
        while (!m_operationQueue.empty() && batch.size() < m_batchSize)
        {
            batch.push_back(std::move(m_operationQueue.front()));
            m_operationQueue.pop();
        }

        // Update current queue size stat
        std::lock_guard<std::mutex> statsLock(m_statsMutex);
        m_stats.currentQueueSize = static_cast<uint32>(m_operationQueue.size());
    }

    // Execute operations outside of lock to avoid blocking queue
    uint32 totalExecutionTime = 0;
    for (auto& operation : batch)
    {
        if (!operation)
            continue;

        try
        {
            // Check if operation is still valid
            if (!operation->IsValid())
            {
                LOG_DEBUG("playerbots", "Skipping invalid operation: {}", operation->GetName());

                std::lock_guard<std::mutex> statsLock(m_statsMutex);
                m_stats.totalOperationsSkipped++;
                continue;
            }

            // Time the execution
            uint32 startTime = getMSTime();

            // Execute the operation
            bool success = operation->Execute();

            uint32 executionTime = GetMSTimeDiffToNow(startTime);
            totalExecutionTime += executionTime;

            // Log slow operations
            if (executionTime > 100)
                LOG_WARN("playerbots", "Slow operation: {} took {}ms", operation->GetName(), executionTime);

            // Update statistics
            std::lock_guard<std::mutex> statsLock(m_statsMutex);
            if (success)
                m_stats.totalOperationsProcessed++;
            else
            {
                m_stats.totalOperationsFailed++;
                LOG_DEBUG("playerbots", "Operation failed: {}", operation->GetName());
            }
        }
        catch (std::exception const& e)
        {
            LOG_ERROR("playerbots", "Exception in operation {}: {}", operation->GetName(), e.what());

            std::lock_guard<std::mutex> statsLock(m_statsMutex);
            m_stats.totalOperationsFailed++;
        }
        catch (...)
        {
            LOG_ERROR("playerbots", "Unknown exception in operation {}", operation->GetName());

            std::lock_guard<std::mutex> statsLock(m_statsMutex);
            m_stats.totalOperationsFailed++;
        }
    }

    // Update average execution time
    if (!batch.empty())
    {
        std::lock_guard<std::mutex> statsLock(m_statsMutex);
        uint32 avgTime = totalExecutionTime / static_cast<uint32>(batch.size());
        // Exponential moving average
        m_stats.averageExecutionTimeMs =
            (m_stats.averageExecutionTimeMs * 9 + avgTime) / 10;  // 90% old, 10% new
    }
}

void PlayerbotWorldThreadProcessor::CheckQueueHealth()
{
    uint32 queueSize = GetQueueSize();
    uint32 threshold = (m_maxQueueSize * m_queueWarningThreshold) / 100;

    if (queueSize >= threshold)
    {
        LOG_WARN("playerbots",
                 "PlayerbotWorldThreadProcessor queue is {}% full ({}/{}). "
                 "Consider increasing update frequency or batch size.",
                 (queueSize * 100) / m_maxQueueSize, queueSize, m_maxQueueSize);
    }
}

uint32 PlayerbotWorldThreadProcessor::GetQueueSize() const
{
    std::lock_guard<std::mutex> lock(m_queueMutex);
    return static_cast<uint32>(m_operationQueue.size());
}

void PlayerbotWorldThreadProcessor::ClearQueue()
{
    std::lock_guard<std::mutex> lock(m_queueMutex);

    uint32 cleared = static_cast<uint32>(m_operationQueue.size());
    if (cleared > 0)
        LOG_INFO("playerbots", "Clearing {} queued operations", cleared);

    // Clear the queue
    while (!m_operationQueue.empty())
    {
        m_operationQueue.pop();
    }

    // Reset queue size stat
    std::lock_guard<std::mutex> statsLock(m_statsMutex);
    m_stats.currentQueueSize = 0;
}

PlayerbotWorldThreadProcessor::Statistics PlayerbotWorldThreadProcessor::GetStatistics() const
{
    std::lock_guard<std::mutex> statsLock(m_statsMutex);
    return m_stats;  // Return a copy
}
