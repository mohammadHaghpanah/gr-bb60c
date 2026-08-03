/**
 * @file api.h
 * @brief Export/import macros for the gnuradio-bb60c shared library.
 *
 * Copyright 2011 Free Software Foundation, Inc.
 * Copyright 2026 Mohammad Haghpanah.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#ifndef INCLUDED_BB60C_API_H
#define INCLUDED_BB60C_API_H

#include <gnuradio/attributes.h>

#ifdef gnuradio_bb60c_EXPORTS
#define BB60C_API __GR_ATTR_EXPORT
#else
#define BB60C_API __GR_ATTR_IMPORT
#endif

#endif /* INCLUDED_BB60C_API_H */
