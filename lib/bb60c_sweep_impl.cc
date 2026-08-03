/* -*- c++ -*- */
/*
 * Copyright 2026 Mohammad Haghpanah.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

/**
 * @file bb60c_sweep_impl.cc
 * @brief Continuous BB60C spectrum sweep source (float stream + sweep_start tags).
 *
 * Architecture:
 *  - Acquisition thread owns the Signal Hound bb_api device handle and pushes
 *    complete sweep vectors into a circular buffer.
 *  - GNU Radio work() pops sweeps, tags the first sample with sweep_start, and
 *    streams bins as float samples for QT GUI Time Sink / Vector Sink.
 *  - Runtime setters request reconfigure; the acquisition thread applies them
 *    (abort → reconfigure → initiate) so the scheduler thread never blocks on
 *    USB/device I/O.
 */

#include "bb60c_sweep_impl.h"
#include <gnuradio/io_signature.h>
#include <pmt/pmt.h>

#include <bb_api.h>

#include <algorithm>
#include <chrono>
#include <cstdint>
#include <cstring>
#include <iostream>
#include <stdexcept>
#include <vector>

namespace gr {
namespace bb60c {

namespace {

/**
 * @brief Opens the device briefly to query sweep length for the given settings.
 *
 * Used by query_sweep_size() so flowgraphs can size Stream-to-Vector / sinks
 * before the source block starts.
 *
 * @return true on success with sweep_size > 0.
 */
bool probe_trace_info(double ref_level,
                      double start_freq,
                      double stop_freq,
                      double rbw,
                      double vbw,
                      double sweep_time,
                      uint32_t rbw_shape,
                      uint32_t rejection,
                      uint32_t detector,
                      uint32_t scale,
                      uint32_t units,
                      uint32_t* sweep_size,
                      double* bin_size,
                      double* start_f)
{
    int handle = -1;
    bbStatus status = bbOpenDevice(&handle);
    if (status != bbNoError) {
        return false;
    }
    const double f_lo = std::min(start_freq, stop_freq);
    const double f_hi = std::max(start_freq, stop_freq);
    const double fc = 0.5 * (f_lo + f_hi);
    const double span = f_hi - f_lo;
    bbConfigureRefLevel(handle, ref_level);
    bbConfigureCenterSpan(handle, fc, span);
    bbConfigureSweepCoupling(handle, rbw, vbw, sweep_time, rbw_shape, rejection);
    bbConfigureAcquisition(handle, detector, scale);
    bbConfigureProcUnits(handle, units);
    status = bbInitiate(handle, BB_SWEEPING, 0);
    if (status < bbNoError) {
        bbCloseDevice(handle);
        return false;
    }
    bbQueryTraceInfo(handle, sweep_size, bin_size, start_f);
    bbAbort(handle);
    bbCloseDevice(handle);
    return (*sweep_size > 0);
}

} // namespace

uint32_t query_sweep_size(double ref_level,
                          double start_freq,
                          double stop_freq,
                          double rbw,
                          double vbw,
                          double sweep_time,
                          int rbw_shape,
                          int rejection,
                          int detector,
                          int scale,
                          int units)
{
    uint32_t sweep_size = 0;
    double bin_size = 0.0;
    double start_f = 0.0;
    if (!probe_trace_info(ref_level,
                          start_freq,
                          stop_freq,
                          rbw,
                          vbw,
                          sweep_time,
                          static_cast<uint32_t>(rbw_shape),
                          static_cast<uint32_t>(rejection),
                          static_cast<uint32_t>(detector),
                          static_cast<uint32_t>(scale),
                          static_cast<uint32_t>(units),
                          &sweep_size,
                          &bin_size,
                          &start_f)) {
        throw std::runtime_error("bb60c_sweep: query_sweep_size failed");
    }
    return sweep_size;
}

bb60c_sweep::sptr bb60c_sweep::make(double ref_level,
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
                                    int buffer_capacity)
{
    return gnuradio::make_block_sptr<bb60c_sweep_impl>(ref_level,
                                                       start_freq,
                                                       stop_freq,
                                                       rbw,
                                                       vbw,
                                                       sweep_time,
                                                       rbw_shape,
                                                       rejection,
                                                       detector,
                                                       scale,
                                                       units,
                                                       buffer_capacity);
}

bb60c_sweep_impl::bb60c_sweep_impl(double ref_level,
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
                                   int buffer_capacity)
    : gr::sync_block(
          "bb60c_sweep",
          gr::io_signature::make(0, 0, 0),
          gr::io_signature::makev(1,
                                  2,
                                  std::vector<int>{ sizeof(float),
                                                    sizeof(std::int32_t) })),
      d_ref_level(ref_level),
      d_start_freq(start_freq),
      d_stop_freq(stop_freq),
      d_rbw(rbw),
      d_vbw(vbw),
      d_sweep_time(sweep_time),
      d_rbw_shape(static_cast<uint32_t>(rbw_shape)),
      d_rejection(static_cast<uint32_t>(rejection)),
      d_detector(static_cast<uint32_t>(detector)),
      d_scale(static_cast<uint32_t>(scale)),
      d_units(static_cast<uint32_t>(units)),
      d_handle(-1),
      d_sweep_size(0),
      d_bin_size(0.0),
      d_trace_start_freq(0.0),
      d_device_open(false),
      d_circ_buffer(static_cast<std::size_t>(std::max(1, buffer_capacity))),
      d_running(false),
      d_reconfig_requested(false),
      d_pending_offset(0)
{
    message_port_register_out(pmt::mp("meta"));
    // Pair (num_points . N) for Message Pair to Var → time_sink.set_nsamps(N)
    message_port_register_out(pmt::mp("num_points"));
}

bb60c_sweep_impl::~bb60c_sweep_impl() { stop(); }

void bb60c_sweep_impl::request_reconfigure()
{
    if (d_running.load(std::memory_order_relaxed)) {
        d_reconfig_requested.store(true, std::memory_order_release);
    }
}

void bb60c_sweep_impl::publish_sweep_meta()
{
    const uint32_t sweep_size = d_sweep_size.load(std::memory_order_relaxed);
    pmt::pmt_t meta = pmt::make_dict();
    meta = pmt::dict_add(meta, pmt::mp("sweep_size"), pmt::from_long(sweep_size));
    meta = pmt::dict_add(meta, pmt::mp("bin_size"), pmt::from_double(d_bin_size));
    meta = pmt::dict_add(
        meta, pmt::mp("start_freq"), pmt::from_double(d_trace_start_freq));
    message_port_pub(pmt::mp("meta"), meta);
    message_port_pub(pmt::mp("num_points"),
                     pmt::cons(pmt::mp("num_points"), pmt::from_long(sweep_size)));
}

bool bb60c_sweep_impl::apply_device_config_locked()
{
    // Caller holds d_param_mutex (or is single-threaded during start).
    // Why: GUI sliders can briefly set start>stop while dragging; a negative
    // span freezes the device config and the plot looks unchanged.
    const double f_lo = std::min(d_start_freq, d_stop_freq);
    const double f_hi = std::max(d_start_freq, d_stop_freq);
    const double span = f_hi - f_lo;
    const double fc = 0.5 * (f_lo + f_hi);

    if (!(span > 0.0) || !(fc > 0.0)) {
        std::cerr << "bb60c_sweep: invalid span (start=" << d_start_freq
                  << " stop=" << d_stop_freq << ")" << std::endl;
        return false;
    }

    bbConfigureRefLevel(d_handle, d_ref_level);
    bbConfigureCenterSpan(d_handle, fc, span);
    bbConfigureSweepCoupling(
        d_handle, d_rbw, d_vbw, d_sweep_time, d_rbw_shape, d_rejection);
    bbConfigureAcquisition(d_handle, d_detector, d_scale);
    bbConfigureProcUnits(d_handle, d_units);

    bbStatus status = bbInitiate(d_handle, BB_SWEEPING, 0);
    if (status < bbNoError) {
        std::cerr << "bb60c_sweep: initiate failed: " << bbGetErrorString(status)
                  << std::endl;
        return false;
    }

    uint32_t sweep_size = 0;
    bbQueryTraceInfo(d_handle, &sweep_size, &d_bin_size, &d_trace_start_freq);
    if (sweep_size == 0) {
        return false;
    }
    d_sweep_size.store(sweep_size, std::memory_order_relaxed);
    d_circ_buffer.clear();

    std::cout << "bb60c_sweep: sweep_size=" << sweep_size
              << " bins. For stationary Time Sink: Trigger Mode=Tag, "
                 "Tag Key=sweep_start, Num Points>="
              << sweep_size << std::endl;

    publish_sweep_meta();
    return true;
}

bool bb60c_sweep_impl::configure_device()
{
    bbStatus status = bbOpenDevice(&d_handle);
    if (status != bbNoError) {
        std::cerr << "bb60c_sweep: open failed: " << bbGetErrorString(status)
                  << std::endl;
        return false;
    }
    d_device_open = true;

    std::lock_guard<std::mutex> lock(d_param_mutex);
    if (!apply_device_config_locked()) {
        close_device();
        return false;
    }
    return true;
}

void bb60c_sweep_impl::close_device()
{
    if (!d_device_open) {
        return;
    }
    bbAbort(d_handle);
    bbCloseDevice(d_handle);
    d_handle = -1;
    d_device_open = false;
}

void bb60c_sweep_impl::set_ref_level(double ref_level)
{
    {
        std::lock_guard<std::mutex> lock(d_param_mutex);
        if (d_ref_level == ref_level) {
            return;
        }
        d_ref_level = ref_level;
    }
    request_reconfigure();
}

void bb60c_sweep_impl::set_start_freq(double start_freq)
{
    {
        std::lock_guard<std::mutex> lock(d_param_mutex);
        if (d_start_freq == start_freq) {
            return;
        }
        d_start_freq = start_freq;
    }
    request_reconfigure();
}

void bb60c_sweep_impl::set_stop_freq(double stop_freq)
{
    {
        std::lock_guard<std::mutex> lock(d_param_mutex);
        if (d_stop_freq == stop_freq) {
            return;
        }
        d_stop_freq = stop_freq;
    }
    request_reconfigure();
}

void bb60c_sweep_impl::set_rbw(double rbw)
{
    {
        std::lock_guard<std::mutex> lock(d_param_mutex);
        if (d_rbw == rbw) {
            return;
        }
        d_rbw = rbw;
    }
    request_reconfigure();
}

void bb60c_sweep_impl::set_vbw(double vbw)
{
    {
        std::lock_guard<std::mutex> lock(d_param_mutex);
        if (d_vbw == vbw) {
            return;
        }
        d_vbw = vbw;
    }
    request_reconfigure();
}

void bb60c_sweep_impl::set_sweep_time(double sweep_time)
{
    {
        std::lock_guard<std::mutex> lock(d_param_mutex);
        if (d_sweep_time == sweep_time) {
            return;
        }
        d_sweep_time = sweep_time;
    }
    request_reconfigure();
}

void bb60c_sweep_impl::set_rbw_shape(int rbw_shape)
{
    {
        std::lock_guard<std::mutex> lock(d_param_mutex);
        const auto v = static_cast<uint32_t>(rbw_shape);
        if (d_rbw_shape == v) {
            return;
        }
        d_rbw_shape = v;
    }
    request_reconfigure();
}

void bb60c_sweep_impl::set_rejection(int rejection)
{
    {
        std::lock_guard<std::mutex> lock(d_param_mutex);
        const auto v = static_cast<uint32_t>(rejection);
        if (d_rejection == v) {
            return;
        }
        d_rejection = v;
    }
    request_reconfigure();
}

void bb60c_sweep_impl::set_detector(int detector)
{
    {
        std::lock_guard<std::mutex> lock(d_param_mutex);
        const auto v = static_cast<uint32_t>(detector);
        if (d_detector == v) {
            return;
        }
        d_detector = v;
    }
    request_reconfigure();
}

void bb60c_sweep_impl::set_scale(int scale)
{
    {
        std::lock_guard<std::mutex> lock(d_param_mutex);
        const auto v = static_cast<uint32_t>(scale);
        if (d_scale == v) {
            return;
        }
        d_scale = v;
    }
    request_reconfigure();
}

void bb60c_sweep_impl::set_units(int units)
{
    {
        std::lock_guard<std::mutex> lock(d_param_mutex);
        const auto v = static_cast<uint32_t>(units);
        if (d_units == v) {
            return;
        }
        d_units = v;
    }
    request_reconfigure();
}

void bb60c_sweep_impl::acquisition_loop()
{
    while (d_running.load(std::memory_order_relaxed)) {
        // Only the acquisition thread touches the BB60C device API.
        if (d_reconfig_requested.exchange(false, std::memory_order_acq_rel)) {
            bbAbort(d_handle);
            bool ok = false;
            {
                std::lock_guard<std::mutex> lock(d_param_mutex);
                // Guard equal endpoints so abort never leaves the device idle.
                if (d_start_freq == d_stop_freq) {
                    d_stop_freq = d_start_freq + 1e6;
                }
                ok = apply_device_config_locked();
            }
            if (!ok) {
                std::cerr << "bb60c_sweep: runtime reconfigure failed" << std::endl;
                break;
            }
            continue;
        }

        const uint32_t sweep_size = d_sweep_size.load(std::memory_order_relaxed);
        if (sweep_size == 0) {
            break;
        }

        std::vector<float> single_sweep(sweep_size);
        std::vector<float> sweep_min;
        float* min_ptr = nullptr;
        // Snapshot detector under lock so it matches current config.
        uint32_t detector = 0;
        {
            std::lock_guard<std::mutex> lock(d_param_mutex);
            detector = d_detector;
        }
        if (detector == BB_MIN_AND_MAX) {
            sweep_min.resize(sweep_size);
            min_ptr = sweep_min.data();
        }

        bbStatus status = bbFetchTrace_32f(
            d_handle, static_cast<int>(sweep_size), min_ptr, single_sweep.data());

        if (!d_running.load(std::memory_order_relaxed)) {
            break;
        }
        if (status != bbNoError) {
            std::cerr << "bb60c_sweep: fetch status: " << bbGetErrorString(status)
                      << std::endl;
            if (status < bbNoError) {
                break;
            }
        }

        d_circ_buffer.push(std::move(single_sweep));
    }
}

bool bb60c_sweep_impl::start()
{
    if (d_running.load()) {
        return true;
    }
    d_reconfig_requested.store(false);
    d_circ_buffer.set_active(true);
    d_pending.clear();
    d_pending_offset = 0;
    if (!configure_device()) {
        throw std::runtime_error("bb60c_sweep: failed to configure BB60C device");
    }
    d_running.store(true);
    d_acq_thread = std::thread(&bb60c_sweep_impl::acquisition_loop, this);
    return true;
}

bool bb60c_sweep_impl::stop()
{
    if (!d_running.exchange(false)) {
        d_circ_buffer.set_active(false);
        close_device();
        return true;
    }
    d_circ_buffer.set_active(false);
    if (d_device_open) {
        bbAbort(d_handle);
    }
    if (d_acq_thread.joinable()) {
        d_acq_thread.join();
    }
    close_device();
    return true;
}

int bb60c_sweep_impl::work(int noutput_items,
                           gr_vector_const_void_star& /*input_items*/,
                           gr_vector_void_star& output_items)
{
    auto* out = static_cast<float*>(output_items[0]);
    std::int32_t* num_points_out = nullptr;
    if (output_items.size() > 1 && output_items[1] != nullptr) {
        num_points_out = static_cast<std::int32_t*>(output_items[1]);
    }
    const auto np =
        static_cast<std::int32_t>(d_sweep_size.load(std::memory_order_relaxed));

    // Drop stale pending when sweep length changed after reconfigure.
    if (!d_pending.empty() &&
        static_cast<std::int32_t>(d_pending.size()) != np) {
        d_pending.clear();
        d_pending_offset = 0;
    }

    int produced = 0;

    while (produced < noutput_items) {
        if (d_pending_offset < d_pending.size()) {
            const int available =
                static_cast<int>(d_pending.size() - d_pending_offset);
            const int to_copy = std::min(available, noutput_items - produced);
            std::memcpy(out + produced,
                        d_pending.data() + d_pending_offset,
                        static_cast<std::size_t>(to_copy) * sizeof(float));
            if (num_points_out != nullptr) {
                for (int i = 0; i < to_copy; ++i) {
                    num_points_out[produced + i] = np;
                }
            }
            d_pending_offset += static_cast<std::size_t>(to_copy);
            produced += to_copy;
            if (d_pending_offset >= d_pending.size()) {
                d_pending.clear();
                d_pending_offset = 0;
            }
            continue;
        }

        if (!d_running.load(std::memory_order_relaxed)) {
            break;
        }

        std::vector<float> sweep;
        if (!d_circ_buffer.pop(sweep, std::chrono::milliseconds(50))) {
            break;
        }

        // Publish every sweep so helpers can call time_sink.set_nsamps(N).
        message_port_pub(
            pmt::mp("num_points"),
            pmt::cons(pmt::mp("num_points"), pmt::from_long(np)));

        add_item_tag(0,
                     nitems_written(0) + produced,
                     pmt::mp("sweep_start"),
                     pmt::from_double(d_trace_start_freq));
        if (num_points_out != nullptr) {
            add_item_tag(1,
                         nitems_written(1) + produced,
                         pmt::mp("num_points"),
                         pmt::from_long(np));
        }

        d_pending = std::move(sweep);
        d_pending_offset = 0;
    }

    if (produced == 0 && d_running.load(std::memory_order_relaxed)) {
        return 0;
    }
    return produced;
}

} /* namespace bb60c */
} /* namespace gr */
