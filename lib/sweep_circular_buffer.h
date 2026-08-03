/* -*- c++ -*- */
/*
 * Copyright 2026 Mohammad Haghpanah.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

/**
 * @file sweep_circular_buffer.h
 * @brief Thread-safe circular buffer of BB60C sweep vectors.
 */

#ifndef INCLUDED_BB60C_SWEEP_CIRCULAR_BUFFER_H
#define INCLUDED_BB60C_SWEEP_CIRCULAR_BUFFER_H

#include <chrono>
#include <condition_variable>
#include <cstddef>
#include <mutex>
#include <utility>
#include <vector>

namespace gr {
namespace bb60c {

/**
 * @brief Fixed-capacity circular buffer storing complete sweep vectors.
 *
 * Producer (acquisition thread) pushes sweeps; consumer (work()) pops them.
 * When full, the oldest sweep is overwritten.
 */
class sweep_circular_buffer
{
public:
    /**
     * @brief Constructs a buffer with @p capacity sweep slots.
     *
     * @param capacity Maximum number of stored sweeps (must be >= 1).
     */
    explicit sweep_circular_buffer(std::size_t capacity)
        : d_buffer(capacity)
        , d_capacity(capacity)
        , d_head(0)
        , d_tail(0)
        , d_count(0)
        , d_active(true)
    {
    }

    /**
     * @brief Enables or disables waiting consumers (used on shutdown).
     *
     * @param active false unblocks waiting pop() calls.
     */
    void set_active(bool active)
    {
        {
            std::lock_guard<std::mutex> lock(d_mutex);
            d_active = active;
        }
        d_cv.notify_all();
    }

    /**
     * @brief Pushes one sweep; overwrites the oldest when full.
     *
     * @param sweep Sweep vector (moved when possible).
     */
    void push(std::vector<float> sweep)
    {
        {
            std::lock_guard<std::mutex> lock(d_mutex);
            if (!d_active) {
                return;
            }

            d_buffer[d_head] = std::move(sweep);
            d_head = (d_head + 1) % d_capacity;

            if (d_count < d_capacity) {
                ++d_count;
            } else {
                d_tail = (d_tail + 1) % d_capacity;
            }
        }
        d_cv.notify_one();
    }

    /**
     * @brief Pops the oldest sweep, waiting up to @p timeout.
     *
     * @param[out] sweep   Destination for the popped sweep.
     * @param      timeout Maximum wait duration.
     * @return true if a sweep was returned, false on timeout/inactive.
     */
    bool pop(std::vector<float>& sweep, std::chrono::milliseconds timeout)
    {
        std::unique_lock<std::mutex> lock(d_mutex);
        if (!d_cv.wait_for(lock, timeout, [this] {
                return d_count > 0 || !d_active;
            })) {
            return false;
        }

        if (d_count == 0) {
            return false;
        }

        sweep = std::move(d_buffer[d_tail]);
        d_tail = (d_tail + 1) % d_capacity;
        --d_count;
        return true;
    }

    /**
     * @brief Returns the number of sweeps currently stored.
     */
    std::size_t size() const
    {
        std::lock_guard<std::mutex> lock(d_mutex);
        return d_count;
    }

    /**
     * @brief Discards all buffered sweeps (used after device reconfigure).
     *
     * Why: after RBW/span changes, sweep vector length may change; stale
     * buffers would desynchronize work() from the new sweep_size.
     */
    void clear()
    {
        std::lock_guard<std::mutex> lock(d_mutex);
        d_head = 0;
        d_tail = 0;
        d_count = 0;
        for (auto& sweep : d_buffer) {
            sweep.clear();
        }
    }

private:
    std::vector<std::vector<float>> d_buffer;
    std::size_t d_capacity;
    std::size_t d_head;
    std::size_t d_tail;
    std::size_t d_count;
    bool d_active;
    mutable std::mutex d_mutex;
    std::condition_variable d_cv;
};

} // namespace bb60c
} // namespace gr

#endif /* INCLUDED_BB60C_SWEEP_CIRCULAR_BUFFER_H */
