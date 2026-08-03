/* -*- c++ -*- */
/*
 * Copyright 2026 Mohammad Haghpanah.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

/**
 * @file bb60c_sweep_impl.h
 * @brief Private implementation of the BB60C continuous sweep source block.
 */

#ifndef INCLUDED_BB60C_BB60C_SWEEP_IMPL_H
#define INCLUDED_BB60C_BB60C_SWEEP_IMPL_H

#include "sweep_circular_buffer.h"
#include <gnuradio/bb60c/bb60c_sweep.h>

#include <atomic>
#include <cstdint>
#include <mutex>
#include <thread>
#include <vector>

namespace gr {
namespace bb60c {

/**
 * @brief Implementation of @ref bb60c_sweep.
 *
 * Separates device I/O (acquisition thread) from the GNU Radio scheduler
 * (@c work) via @ref sweep_circular_buffer. Parameter setters are thread-safe
 * and request a device reconfigure without blocking the caller.
 */
class bb60c_sweep_impl : public bb60c_sweep
{
private:
    mutable std::mutex d_param_mutex; ///< Protects sweep/config parameters below.
    double d_ref_level;
    double d_start_freq;
    double d_stop_freq;
    double d_rbw;
    double d_vbw;
    double d_sweep_time;
    uint32_t d_rbw_shape;
    uint32_t d_rejection;
    uint32_t d_detector;
    uint32_t d_scale;
    uint32_t d_units;

    int d_handle;                       ///< Signal Hound device handle (-1 if closed).
    std::atomic<uint32_t> d_sweep_size; ///< Bins per sweep (from bbQueryTraceInfo).
    double d_bin_size;                  ///< Hz per bin.
    double d_trace_start_freq;          ///< Absolute start frequency of the trace (Hz).
    bool d_device_open;

    sweep_circular_buffer d_circ_buffer; ///< Producer/consumer sweep queue.
    std::atomic<bool> d_running;
    std::atomic<bool> d_reconfig_requested;
    std::thread d_acq_thread;

    std::vector<float> d_pending; ///< Partial sweep not yet fully consumed by work().
    std::size_t d_pending_offset;

    /**
     * @brief Device fetch loop; exits when @c d_running becomes false.
     */
    void acquisition_loop();

    /**
     * @brief Opens the device and applies the current configuration.
     * @return true on success.
     */
    bool configure_device();

    /**
     * @brief Applies center/span/RBW/etc. and initiates sweeping mode.
     *
     * Caller must hold @c d_param_mutex (or be single-threaded during start).
     * @return true on success.
     */
    bool apply_device_config_locked();

    /**
     * @brief Aborts and closes the device handle if open.
     */
    void close_device();

    /**
     * @brief Signals the acquisition thread to reconfigure (no-op if stopped).
     */
    void request_reconfigure();

    /**
     * @brief Publishes sweep_size / bin_size / start_freq on message ports.
     */
    void publish_sweep_meta();

public:
    /**
     * @brief Constructs the block with initial sweep parameters.
     *
     * @param buffer_capacity Max complete sweeps retained in the circular buffer.
     */
    bb60c_sweep_impl(double ref_level,
                     double start_freq,
                     double stop_freq,
                     double rbw,
                     double vbw,
                     double sweep_time,
                     int rbw_shape,
                     int rejection,
                     int detector,
                     int scale,
                     int units,
                     int buffer_capacity);
    ~bb60c_sweep_impl() override;

    uint32_t sweep_size() const override
    {
        return d_sweep_size.load(std::memory_order_relaxed);
    }

    void set_ref_level(double ref_level) override;
    void set_start_freq(double start_freq) override;
    void set_stop_freq(double stop_freq) override;
    void set_rbw(double rbw) override;
    void set_vbw(double vbw) override;
    void set_sweep_time(double sweep_time) override;
    void set_rbw_shape(int rbw_shape) override;
    void set_rejection(int rejection) override;
    void set_detector(int detector) override;
    void set_scale(int scale) override;
    void set_units(int units) override;

    bool start() override;
    bool stop() override;

    int work(int noutput_items,
             gr_vector_const_void_star& input_items,
             gr_vector_void_star& output_items) override;
};

} // namespace bb60c
} // namespace gr

#endif /* INCLUDED_BB60C_BB60C_SWEEP_IMPL_H */
