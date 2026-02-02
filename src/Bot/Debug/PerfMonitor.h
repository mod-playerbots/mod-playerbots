/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_PERFORMANCEMONITOR_H
#define _PLAYERBOT_PERFORMANCEMONITOR_H

#include <chrono>
#include <ctime>
#include <map>
#include <mutex>
#include <vector>
#include <cstdint>

typedef std::vector<std::string> PerformanceStack;

struct PerformanceData
{
    uint64_t minTime;
    uint64_t maxTime;
    uint64_t totalTime;
    uint32_t count;
    std::mutex lock;
};

enum PerformanceMetric
{
    PERF_MON_TRIGGER,
    PERF_MON_VALUE,
    PERF_MON_ACTION,
    PERF_MON_RNDBOT,
    PERF_MON_TOTAL
};

class PerfMonitorOperation
{
public:
    PerfMonitorOperation(PerformanceData* data, std::string const name, PerformanceStack* stack);
    void finish();

private:
    PerformanceData* data;
    std::string const name;
    PerformanceStack* stack;
    std::chrono::microseconds started;
};

class PerfMonitor
{
public:
    static PerfMonitor& instance()
    {
        static PerfMonitor instance;

        return instance;
    }

    PerfMonitorOperation* start(PerformanceMetric metric, std::string const name,
                                       PerformanceStack* stack = nullptr);
    void PrintStats(bool perTick = false, bool fullStack = false);
    void Reset();

private:
    PerfMonitor() = default;
    virtual ~PerfMonitor() = default;

    PerfMonitor(const PerfMonitor&) = delete;
    PerfMonitor& operator=(const PerfMonitor&) = delete;

    PerfMonitor(PerfMonitor&&) = delete;
    PerfMonitor& operator=(PerfMonitor&&) = delete;

    std::map<PerformanceMetric, std::map<std::string, PerformanceData*> > data;
    std::mutex lock;
};

#define sPerfMonitor PerfMonitor::instance()

#endif
