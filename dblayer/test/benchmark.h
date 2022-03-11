/*
 * toolkit.h
 *
 *  Created on: Oct 16, 2014
 *      Author: fuyf
 */

#ifndef TOOLKIT_H_
#define TOOLKIT_H_

#include <time.h>
#include <ctsp/debug.h>

// Timing toolkit
#define BEGIN_BENCHMARK() \
    struct timespec TS_START; \
    clock_gettime(CLOCK_MONOTONIC, &TS_START);

#define END_BENCHMARK() \
    struct timespec TS_END; \
    clock_gettime(CLOCK_MONOTONIC, &TS_END);

#define TIME_INTERVAL_NS() \
    ((TS_END.tv_sec*1000000000+TS_END.tv_nsec)-(TS_START.tv_sec*1000000000+TS_START.tv_nsec))

#define TIME_INTERVAL_US() \
    (((TS_END.tv_sec*1000000000+TS_END.tv_nsec)-(TS_START.tv_sec*1000000000+TS_START.tv_nsec)) / 1000)

#define TIME_INTERVAL_MS() \
    (((TS_END.tv_sec*1000000000+TS_END.tv_nsec)-(TS_START.tv_sec*1000000000+TS_START.tv_nsec)) / 1000000)

#define TIME_INTERVAL_SEC() \
    (((TS_END.tv_sec*1000000000+TS_END.tv_nsec)-(TS_START.tv_sec*1000000000+TS_START.tv_nsec)) / 1000000000)


// Some utils on benchmark

// show benchmark result, 'max' and 'min' is the max and min of intervals, it's better to use 'static' variables for max and min
#define DEFINE_BENCHMARK_MAX_MIN(max, min) \
    static unsigned long long max = 0, min = 1 << 30;

#define SHOW_BENCHMARK_MAX_MIN(max, min) \
{ \
    unsigned long long interval = TIME_INTERVAL_NS(); \
    if(interval > max) \
    { \
        max = interval; \
        INFO << " [BENCHMARK " << #max << "]: '" << __FUNCTION__ << "' MAX_INTERVAL " << max << "ns\n\n"; \
    } \
    if(interval < min) \
    { \
        min = interval; \
        INFO << " [BENCHMARK " << #min << "]: '" << __FUNCTION__ << "' MIN_INTERVAL " << min << "ns\n\n"; \
    } \
    if(interval % 5000 == 0) \
    { \
        INFO << " [BENCHMARK " << #max << "]: '" << __FUNCTION__ << "' RANDOM_INTERVAL " << interval << "ns\n\n"; \
    } \
}

// show loop count, total intervals and average benchmark intervals
#define DEFINE_BENCHMARK_TOTAL_COUNT(total, count) \
    static double total = 0; \
    static unsigned long long count = 0;

#define INCR_BENCHMARK_TOTAL_COUNT(total, count) \
    total += TIME_INTERVAL_NS(); \
    ++count;

#define SHOW_BENCHMARK_TOTAL_COUNT_AVG(total, count) \
{ \
    INFO << " [BENCHMARK " << #total << "]: '" << __FUNCTION__ << "' TOTAL " << total << "ns, COUNT " << count << ", AVG_INTERVAL " << (total/count) << "ns\n\n"; \
}

#endif /* TOOLKIT_H_ */
