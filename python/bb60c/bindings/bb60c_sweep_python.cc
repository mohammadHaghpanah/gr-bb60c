/*
 * Copyright 2026 Mohammad Haghpanah.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include <pybind11/complex.h>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

namespace py = pybind11;

#include <gnuradio/bb60c/bb60c_sweep.h>
#include <bb60c_sweep_pydoc.h>

void bind_bb60c_sweep(py::module& m)
{
    m.def("query_sweep_size",
          &gr::bb60c::query_sweep_size,
          py::arg("ref_level") = -20.0,
          py::arg("start_freq") = 100e6,
          py::arg("stop_freq") = 900e6,
          py::arg("rbw") = 10e3,
          py::arg("vbw") = 10e3,
          py::arg("sweep_time") = 0.001,
          py::arg("rbw_shape") = 1,
          py::arg("rejection") = 0,
          py::arg("detector") = 1,
          py::arg("scale") = 0,
          py::arg("units") = 0);

    using bb60c_sweep = gr::bb60c::bb60c_sweep;

    py::class_<bb60c_sweep,
               gr::sync_block,
               gr::block,
               gr::basic_block,
               std::shared_ptr<bb60c_sweep>>(m, "bb60c_sweep", D(bb60c_sweep))
        .def(py::init(&bb60c_sweep::make),
             py::arg("ref_level") = -20.0,
             py::arg("start_freq") = 100e6,
             py::arg("stop_freq") = 900e6,
             py::arg("rbw") = 10e3,
             py::arg("vbw") = 10e3,
             py::arg("sweep_time") = 0.001,
             py::arg("rbw_shape") = 1,
             py::arg("rejection") = 0,
             py::arg("detector") = 1,
             py::arg("scale") = 0,
             py::arg("units") = 0,
             py::arg("buffer_capacity") = 32,
             D(bb60c_sweep, make))
        .def("sweep_size", &bb60c_sweep::sweep_size)
        .def("set_ref_level", &bb60c_sweep::set_ref_level, py::arg("ref_level"))
        .def("set_start_freq", &bb60c_sweep::set_start_freq, py::arg("start_freq"))
        .def("set_stop_freq", &bb60c_sweep::set_stop_freq, py::arg("stop_freq"))
        .def("set_rbw", &bb60c_sweep::set_rbw, py::arg("rbw"))
        .def("set_vbw", &bb60c_sweep::set_vbw, py::arg("vbw"))
        .def("set_sweep_time", &bb60c_sweep::set_sweep_time, py::arg("sweep_time"))
        .def("set_rbw_shape", &bb60c_sweep::set_rbw_shape, py::arg("rbw_shape"))
        .def("set_rejection", &bb60c_sweep::set_rejection, py::arg("rejection"))
        .def("set_detector", &bb60c_sweep::set_detector, py::arg("detector"))
        .def("set_scale", &bb60c_sweep::set_scale, py::arg("scale"))
        .def("set_units", &bb60c_sweep::set_units, py::arg("units"));
}
