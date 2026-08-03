#!/usr/bin/env python3
# -*- coding: utf-8 -*-

#
# SPDX-License-Identifier: GPL-3.0
#
# GNU Radio Python Flow Graph
# Title: BB60C Sweep
# GNU Radio version: 3.10.1.1

from packaging.version import Version as StrictVersion

if __name__ == '__main__':
    import ctypes
    import sys
    if sys.platform.startswith('linux'):
        try:
            x11 = ctypes.cdll.LoadLibrary('libX11.so')
            x11.XInitThreads()
        except:
            print("Warning: failed to XInitThreads()")

from PyQt5 import Qt
from PyQt5.QtCore import QObject, pyqtSlot
from gnuradio import qtgui
from gnuradio.filter import firdes
import sip
from gnuradio import bb60c
from gnuradio import blocks
from gnuradio import gr
from gnuradio.fft import window
import sys
import signal
from argparse import ArgumentParser
from gnuradio.eng_arg import eng_float, intx
from gnuradio import eng_notation
from gnuradio.qtgui import Range, RangeWidget
from PyQt5 import QtCore



from gnuradio import qtgui

class untitled(gr.top_block, Qt.QWidget):

    def __init__(self):
        gr.top_block.__init__(self, "BB60C Sweep", catch_exceptions=True)
        Qt.QWidget.__init__(self)
        self.setWindowTitle("BB60C Sweep")
        qtgui.util.check_set_qss()
        try:
            self.setWindowIcon(Qt.QIcon.fromTheme('gnuradio-grc'))
        except:
            pass
        self.top_scroll_layout = Qt.QVBoxLayout()
        self.setLayout(self.top_scroll_layout)
        self.top_scroll = Qt.QScrollArea()
        self.top_scroll.setFrameStyle(Qt.QFrame.NoFrame)
        self.top_scroll_layout.addWidget(self.top_scroll)
        self.top_scroll.setWidgetResizable(True)
        self.top_widget = Qt.QWidget()
        self.top_scroll.setWidget(self.top_widget)
        self.top_layout = Qt.QVBoxLayout(self.top_widget)
        self.top_grid_layout = Qt.QGridLayout()
        self.top_layout.addLayout(self.top_grid_layout)

        self.settings = Qt.QSettings("GNU Radio", "untitled")

        try:
            if StrictVersion(Qt.qVersion()) < StrictVersion("5.0.0"):
                self.restoreGeometry(self.settings.value("geometry").toByteArray())
            else:
                self.restoreGeometry(self.settings.value("geometry"))
        except:
            pass

        ##################################################
        # Variables
        ##################################################
        self.vbw = vbw = 100e3
        self.units = units = 0
        self.sweep_time = sweep_time = 0.01
        self.stop_freq = stop_freq = 1000e6
        self.start_freq = start_freq = 900e6
        self.scale = scale = 0
        self.samp_rate = samp_rate = 32000
        self.rejection = rejection = 0
        self.ref_level = ref_level = -20.0
        self.rbw_shape = rbw_shape = 1
        self.rbw = rbw = 100e3
        self.detector = detector = 1
        self.buffer_capacity = buffer_capacity = 32

        ##################################################
        # Blocks
        ##################################################
        self._vbw_range = Range(1e3, 10e6, 1e3, 100e3, 200)
        self._vbw_win = RangeWidget(self._vbw_range, self.set_vbw, "VBW (Hz)", "counter_slider", float, QtCore.Qt.Horizontal)
        self.top_grid_layout.addWidget(self._vbw_win, 2, 1, 1, 1)
        for r in range(2, 3):
            self.top_grid_layout.setRowStretch(r, 1)
        for c in range(1, 2):
            self.top_grid_layout.setColumnStretch(c, 1)
        # Create the options list
        self._units_options = [0, 1, 2, 3]
        # Create the labels list
        self._units_labels = ['Log (dBm)', 'Voltage', 'Power', 'Sample']
        # Create the combo box
        self._units_tool_bar = Qt.QToolBar(self)
        self._units_tool_bar.addWidget(Qt.QLabel("Units" + ": "))
        self._units_combo_box = Qt.QComboBox()
        self._units_tool_bar.addWidget(self._units_combo_box)
        for _label in self._units_labels: self._units_combo_box.addItem(_label)
        self._units_callback = lambda i: Qt.QMetaObject.invokeMethod(self._units_combo_box, "setCurrentIndex", Qt.Q_ARG("int", self._units_options.index(i)))
        self._units_callback(self.units)
        self._units_combo_box.currentIndexChanged.connect(
            lambda i: self.set_units(self._units_options[i]))
        # Create the radio buttons
        self.top_grid_layout.addWidget(self._units_tool_bar, 5, 0, 1, 1)
        for r in range(5, 6):
            self.top_grid_layout.setRowStretch(r, 1)
        for c in range(0, 1):
            self.top_grid_layout.setColumnStretch(c, 1)
        self._sweep_time_range = Range(0.001, 1, 0.001, 0.01, 200)
        self._sweep_time_win = RangeWidget(self._sweep_time_range, self.set_sweep_time, "Sweep Time (s)", "counter_slider", float, QtCore.Qt.Horizontal)
        self.top_grid_layout.addWidget(self._sweep_time_win, 1, 1, 1, 1)
        for r in range(1, 2):
            self.top_grid_layout.setRowStretch(r, 1)
        for c in range(1, 2):
            self.top_grid_layout.setColumnStretch(c, 1)
        self._stop_freq_range = Range(9e6, 6e9, 1e6, 1000e6, 200)
        self._stop_freq_win = RangeWidget(self._stop_freq_range, self.set_stop_freq, "Stop Freq (Hz)", "counter_slider", float, QtCore.Qt.Horizontal)
        self.top_grid_layout.addWidget(self._stop_freq_win, 0, 1, 1, 1)
        for r in range(0, 1):
            self.top_grid_layout.setRowStretch(r, 1)
        for c in range(1, 2):
            self.top_grid_layout.setColumnStretch(c, 1)
        self._start_freq_range = Range(9e6, 6e9, 1e6, 900e6, 200)
        self._start_freq_win = RangeWidget(self._start_freq_range, self.set_start_freq, "Start Freq (Hz)", "counter_slider", float, QtCore.Qt.Horizontal)
        self.top_grid_layout.addWidget(self._start_freq_win, 0, 0, 1, 1)
        for r in range(0, 1):
            self.top_grid_layout.setRowStretch(r, 1)
        for c in range(0, 1):
            self.top_grid_layout.setColumnStretch(c, 1)
        # Create the options list
        self._scale_options = [0, 1, 2, 3]
        # Create the labels list
        self._scale_labels = ['Log', 'Linear', 'Log Full Scale', 'Linear Full Scale']
        # Create the combo box
        self._scale_tool_bar = Qt.QToolBar(self)
        self._scale_tool_bar.addWidget(Qt.QLabel("Scale" + ": "))
        self._scale_combo_box = Qt.QComboBox()
        self._scale_tool_bar.addWidget(self._scale_combo_box)
        for _label in self._scale_labels: self._scale_combo_box.addItem(_label)
        self._scale_callback = lambda i: Qt.QMetaObject.invokeMethod(self._scale_combo_box, "setCurrentIndex", Qt.Q_ARG("int", self._scale_options.index(i)))
        self._scale_callback(self.scale)
        self._scale_combo_box.currentIndexChanged.connect(
            lambda i: self.set_scale(self._scale_options[i]))
        # Create the radio buttons
        self.top_grid_layout.addWidget(self._scale_tool_bar, 4, 1, 1, 1)
        for r in range(4, 5):
            self.top_grid_layout.setRowStretch(r, 1)
        for c in range(1, 2):
            self.top_grid_layout.setColumnStretch(c, 1)
        # Create the options list
        self._rejection_options = [0, 1]
        # Create the labels list
        self._rejection_labels = ['No Spur Reject', 'Spur Reject']
        # Create the combo box
        self._rejection_tool_bar = Qt.QToolBar(self)
        self._rejection_tool_bar.addWidget(Qt.QLabel("Spur Rejection" + ": "))
        self._rejection_combo_box = Qt.QComboBox()
        self._rejection_tool_bar.addWidget(self._rejection_combo_box)
        for _label in self._rejection_labels: self._rejection_combo_box.addItem(_label)
        self._rejection_callback = lambda i: Qt.QMetaObject.invokeMethod(self._rejection_combo_box, "setCurrentIndex", Qt.Q_ARG("int", self._rejection_options.index(i)))
        self._rejection_callback(self.rejection)
        self._rejection_combo_box.currentIndexChanged.connect(
            lambda i: self.set_rejection(self._rejection_options[i]))
        # Create the radio buttons
        self.top_grid_layout.addWidget(self._rejection_tool_bar, 3, 1, 1, 1)
        for r in range(3, 4):
            self.top_grid_layout.setRowStretch(r, 1)
        for c in range(1, 2):
            self.top_grid_layout.setColumnStretch(c, 1)
        self._ref_level_range = Range(-50, 20, 1, -20.0, 200)
        self._ref_level_win = RangeWidget(self._ref_level_range, self.set_ref_level, "Ref Level (dBm)", "counter_slider", float, QtCore.Qt.Horizontal)
        self.top_grid_layout.addWidget(self._ref_level_win, 1, 0, 1, 1)
        for r in range(1, 2):
            self.top_grid_layout.setRowStretch(r, 1)
        for c in range(0, 1):
            self.top_grid_layout.setColumnStretch(c, 1)
        # Create the options list
        self._rbw_shape_options = [0, 1, 2]
        # Create the labels list
        self._rbw_shape_labels = ['Nuttall', 'Flattop', 'CISPR']
        # Create the combo box
        self._rbw_shape_tool_bar = Qt.QToolBar(self)
        self._rbw_shape_tool_bar.addWidget(Qt.QLabel("RBW Shape" + ": "))
        self._rbw_shape_combo_box = Qt.QComboBox()
        self._rbw_shape_tool_bar.addWidget(self._rbw_shape_combo_box)
        for _label in self._rbw_shape_labels: self._rbw_shape_combo_box.addItem(_label)
        self._rbw_shape_callback = lambda i: Qt.QMetaObject.invokeMethod(self._rbw_shape_combo_box, "setCurrentIndex", Qt.Q_ARG("int", self._rbw_shape_options.index(i)))
        self._rbw_shape_callback(self.rbw_shape)
        self._rbw_shape_combo_box.currentIndexChanged.connect(
            lambda i: self.set_rbw_shape(self._rbw_shape_options[i]))
        # Create the radio buttons
        self.top_grid_layout.addWidget(self._rbw_shape_tool_bar, 3, 0, 1, 1)
        for r in range(3, 4):
            self.top_grid_layout.setRowStretch(r, 1)
        for c in range(0, 1):
            self.top_grid_layout.setColumnStretch(c, 1)
        self._rbw_range = Range(1e3, 10e6, 1e3, 100e3, 200)
        self._rbw_win = RangeWidget(self._rbw_range, self.set_rbw, "RBW (Hz)", "counter_slider", float, QtCore.Qt.Horizontal)
        self.top_grid_layout.addWidget(self._rbw_win, 2, 0, 1, 1)
        for r in range(2, 3):
            self.top_grid_layout.setRowStretch(r, 1)
        for c in range(0, 1):
            self.top_grid_layout.setColumnStretch(c, 1)
        # Create the options list
        self._detector_options = [0, 1]
        # Create the labels list
        self._detector_labels = ['Min and Max', 'Average']
        # Create the combo box
        self._detector_tool_bar = Qt.QToolBar(self)
        self._detector_tool_bar.addWidget(Qt.QLabel("Detector" + ": "))
        self._detector_combo_box = Qt.QComboBox()
        self._detector_tool_bar.addWidget(self._detector_combo_box)
        for _label in self._detector_labels: self._detector_combo_box.addItem(_label)
        self._detector_callback = lambda i: Qt.QMetaObject.invokeMethod(self._detector_combo_box, "setCurrentIndex", Qt.Q_ARG("int", self._detector_options.index(i)))
        self._detector_callback(self.detector)
        self._detector_combo_box.currentIndexChanged.connect(
            lambda i: self.set_detector(self._detector_options[i]))
        # Create the radio buttons
        self.top_grid_layout.addWidget(self._detector_tool_bar, 4, 0, 1, 1)
        for r in range(4, 5):
            self.top_grid_layout.setRowStretch(r, 1)
        for c in range(0, 1):
            self.top_grid_layout.setColumnStretch(c, 1)
        self._buffer_capacity_range = Range(1, 256, 1, 32, 200)
        self._buffer_capacity_win = RangeWidget(self._buffer_capacity_range, self.set_buffer_capacity, "Buffer Capacity", "counter_slider", int, QtCore.Qt.Horizontal)
        self.top_grid_layout.addWidget(self._buffer_capacity_win, 5, 1, 1, 1)
        for r in range(5, 6):
            self.top_grid_layout.setRowStretch(r, 1)
        for c in range(1, 2):
            self.top_grid_layout.setColumnStretch(c, 1)
        self.qtgui_time_sink_x_0 = qtgui.time_sink_f(
            1024, #size
            samp_rate, #samp_rate
            "", #name
            1, #number of inputs
            None # parent
        )
        self.qtgui_time_sink_x_0.set_update_time(0.10)
        self.qtgui_time_sink_x_0.set_y_axis(-1, 1)

        self.qtgui_time_sink_x_0.set_y_label('Amplitude', "")

        self.qtgui_time_sink_x_0.enable_tags(True)
        self.qtgui_time_sink_x_0.set_trigger_mode(qtgui.TRIG_MODE_TAG, qtgui.TRIG_SLOPE_POS, 0.0, 0, 0, "sweep_start")
        self.qtgui_time_sink_x_0.enable_autoscale(True)
        self.qtgui_time_sink_x_0.enable_grid(True)
        self.qtgui_time_sink_x_0.enable_axis_labels(False)
        self.qtgui_time_sink_x_0.enable_control_panel(True)
        self.qtgui_time_sink_x_0.enable_stem_plot(False)

        self.qtgui_time_sink_x_0.disable_legend()

        labels = ['', 'Signal 2', 'Signal 3', 'Signal 4', 'Signal 5',
            'Signal 6', 'Signal 7', 'Signal 8', 'Signal 9', 'Signal 10']
        widths = [1, 1, 1, 1, 1,
            1, 1, 1, 1, 1]
        colors = ['yellow', 'red', 'green', 'black', 'cyan',
            'magenta', 'yellow', 'dark red', 'dark green', 'dark blue']
        alphas = [1.0, 1.0, 1.0, 1.0, 1.0,
            1.0, 1.0, 1.0, 1.0, 1.0]
        styles = [1, 1, 1, 1, 1,
            1, 1, 1, 1, 1]
        markers = [-1, -1, -1, -1, -1,
            -1, -1, -1, -1, -1]


        for i in range(1):
            if len(labels[i]) == 0:
                self.qtgui_time_sink_x_0.set_line_label(i, "Data {0}".format(i))
            else:
                self.qtgui_time_sink_x_0.set_line_label(i, labels[i])
            self.qtgui_time_sink_x_0.set_line_width(i, widths[i])
            self.qtgui_time_sink_x_0.set_line_color(i, colors[i])
            self.qtgui_time_sink_x_0.set_line_style(i, styles[i])
            self.qtgui_time_sink_x_0.set_line_marker(i, markers[i])
            self.qtgui_time_sink_x_0.set_line_alpha(i, alphas[i])

        self._qtgui_time_sink_x_0_win = sip.wrapinstance(self.qtgui_time_sink_x_0.qwidget(), Qt.QWidget)
        self.top_grid_layout.addWidget(self._qtgui_time_sink_x_0_win, 6, 0, 4, 2)
        for r in range(6, 10):
            self.top_grid_layout.setRowStretch(r, 1)
        for c in range(0, 2):
            self.top_grid_layout.setColumnStretch(c, 1)
        self.bb60c_set_time_sink_nsamps_0 = blocks.msg_pair_to_var(getattr(self, 'qtgui_time_sink_x_0').set_nsamps)
        self.bb60c_bb60c_sweep_0 = bb60c.bb60c_sweep(ref_level, start_freq, stop_freq, rbw, vbw, sweep_time, rbw_shape, rejection, detector, scale, units, buffer_capacity)


        ##################################################
        # Connections
        ##################################################
        self.msg_connect((self.bb60c_bb60c_sweep_0, 'num_points'), (self.bb60c_set_time_sink_nsamps_0, 'inpair'))
        self.connect((self.bb60c_bb60c_sweep_0, 0), (self.qtgui_time_sink_x_0, 0))


    def closeEvent(self, event):
        self.settings = Qt.QSettings("GNU Radio", "untitled")
        self.settings.setValue("geometry", self.saveGeometry())
        self.stop()
        self.wait()

        event.accept()

    def get_vbw(self):
        return self.vbw

    def set_vbw(self, vbw):
        self.vbw = vbw

    def get_units(self):
        return self.units

    def set_units(self, units):
        self.units = units
        self._units_callback(self.units)

    def get_sweep_time(self):
        return self.sweep_time

    def set_sweep_time(self, sweep_time):
        self.sweep_time = sweep_time

    def get_stop_freq(self):
        return self.stop_freq

    def set_stop_freq(self, stop_freq):
        self.stop_freq = stop_freq

    def get_start_freq(self):
        return self.start_freq

    def set_start_freq(self, start_freq):
        self.start_freq = start_freq

    def get_scale(self):
        return self.scale

    def set_scale(self, scale):
        self.scale = scale
        self._scale_callback(self.scale)

    def get_samp_rate(self):
        return self.samp_rate

    def set_samp_rate(self, samp_rate):
        self.samp_rate = samp_rate
        self.qtgui_time_sink_x_0.set_samp_rate(self.samp_rate)

    def get_rejection(self):
        return self.rejection

    def set_rejection(self, rejection):
        self.rejection = rejection
        self._rejection_callback(self.rejection)

    def get_ref_level(self):
        return self.ref_level

    def set_ref_level(self, ref_level):
        self.ref_level = ref_level

    def get_rbw_shape(self):
        return self.rbw_shape

    def set_rbw_shape(self, rbw_shape):
        self.rbw_shape = rbw_shape
        self._rbw_shape_callback(self.rbw_shape)

    def get_rbw(self):
        return self.rbw

    def set_rbw(self, rbw):
        self.rbw = rbw

    def get_detector(self):
        return self.detector

    def set_detector(self, detector):
        self.detector = detector
        self._detector_callback(self.detector)

    def get_buffer_capacity(self):
        return self.buffer_capacity

    def set_buffer_capacity(self, buffer_capacity):
        self.buffer_capacity = buffer_capacity




def main(top_block_cls=untitled, options=None):

    if StrictVersion("4.5.0") <= StrictVersion(Qt.qVersion()) < StrictVersion("5.0.0"):
        style = gr.prefs().get_string('qtgui', 'style', 'raster')
        Qt.QApplication.setGraphicsSystem(style)
    qapp = Qt.QApplication(sys.argv)

    tb = top_block_cls()

    tb.start()

    tb.show()

    def sig_handler(sig=None, frame=None):
        tb.stop()
        tb.wait()

        Qt.QApplication.quit()

    signal.signal(signal.SIGINT, sig_handler)
    signal.signal(signal.SIGTERM, sig_handler)

    timer = Qt.QTimer()
    timer.start(500)
    timer.timeout.connect(lambda: None)

    qapp.exec_()

if __name__ == '__main__':
    main()
