/* -*- c++ -*- */
/*
 * Copyright 2026 Mohammad Haghpanah.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

/**
 * @file bb60c_sweep.h
 * @brief Public API for the BB60C continuous spectrum sweep source block.
 */

#ifndef INCLUDED_BB60C_BB60C_SWEEP_H
#define INCLUDED_BB60C_BB60C_SWEEP_H

#include <gnuradio/bb60c/api.h>
#include <gnuradio/sync_block.h>

#include <cstdint>

namespace gr {
namespace bb60c {

/*!
 * \brief Query BB60C for bins per sweep (optional helper for Stream to Vector).
 * \ingroup bb60c
 *
 * Opens the device briefly, configures a sweep with the given parameters,
 * reads the trace length, then closes the device.
 */
BB60C_API uint32_t query_sweep_size(double ref_level = -20.0,
                                    double start_freq = 100e6,
                                    double stop_freq = 900e6,
                                    double rbw = 10e3,
                                    double vbw = 10e3,
                                    double sweep_time = 0.001,
                                    int rbw_shape = 1,
                                    int rejection = 0,
                                    int detector = 1,
                                    int scale = 0,
                                    int units = 0);

/*!
 * \brief Continuous BB60C spectrum sweep source (float stream).
 * \ingroup bb60c
 *
 * Each sweep is tagged with \c sweep_start at the first sample. For a
 * stationary spectrum display in QT GUI Time Sink, set:
 *   Trigger Mode = Tag, Trigger Tag Key = sweep_start.
 *
 * Alternatively use blocks.stream_to_vector + QT GUI Vector Sink with
 * Num Items / Vector Size = query_sweep_size(...).
 */
class BB60C_API bb60c_sweep : virtual public gr::sync_block
{
public:
    typedef std::shared_ptr<bb60c_sweep> sptr;

    static sptr make(double ref_level = -20.0,
                     double start_freq = 100e6,
                     double stop_freq = 900e6,
                     double rbw = 10e3,
                     double vbw = 10e3,
                     double sweep_time = 0.001,
                     int rbw_shape = 1,
                     int rejection = 0,
                     int detector = 1,
                     int scale = 0,
                     int units = 0,
                     int buffer_capacity = 32);

    virtual uint32_t sweep_size() const = 0;

    /*!
     * @brief Updates reference level and requests device reconfigure if running.
     * @param ref_level Reference level in dBm.
     */
    virtual void set_ref_level(double ref_level) = 0;

    /*!
     * @brief Updates sweep start frequency (Hz) and reconfigures if running.
     * @param start_freq Start frequency in Hz.
     */
    virtual void set_start_freq(double start_freq) = 0;

    /*!
     * @brief Updates sweep stop frequency (Hz) and reconfigures if running.
     * @param stop_freq Stop frequency in Hz.
     */
    virtual void set_stop_freq(double stop_freq) = 0;

    /*!
     * @brief Updates RBW (Hz) and reconfigures if running.
     * @param rbw Resolution bandwidth in Hz.
     */
    virtual void set_rbw(double rbw) = 0;

    /*!
     * @brief Updates VBW (Hz) and reconfigures if running.
     * @param vbw Video bandwidth in Hz.
     */
    virtual void set_vbw(double vbw) = 0;

    /*!
     * @brief Updates sweep time (s) and reconfigures if running.
     * @param sweep_time Sweep time in seconds.
     */
    virtual void set_sweep_time(double sweep_time) = 0;

    /*!
     * @brief Updates RBW window shape and reconfigures if running.
     * @param rbw_shape 0=Nuttall, 1=Flattop, 2=CISPR.
     */
    virtual void set_rbw_shape(int rbw_shape) = 0;

    /*!
     * @brief Updates spur rejection mode and reconfigures if running.
     * @param rejection 0=off, 1=on.
     */
    virtual void set_rejection(int rejection) = 0;

    /*!
     * @brief Updates detector mode and reconfigures if running.
     * @param detector 0=min/max, 1=average.
     */
    virtual void set_detector(int detector) = 0;

    /*!
     * @brief Updates scale mode and reconfigures if running.
     * @param scale 0=log, 1=linear, 2=log full scale, 3=linear full scale.
     */
    virtual void set_scale(int scale) = 0;

    /*!
     * @brief Updates processing units and reconfigures if running.
     * @param units 0=dBm, 1=voltage, 2=power, 3=sample.
     */
    virtual void set_units(int units) = 0;
};

} // namespace bb60c
} // namespace gr

#endif /* INCLUDED_BB60C_BB60C_SWEEP_H */
