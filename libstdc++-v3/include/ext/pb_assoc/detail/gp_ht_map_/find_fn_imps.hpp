// -*- C++ -*-

// Copyright (C) 2005 Free Software Foundation, Inc.
//
// This file is part of the GNU ISO C++ Library.  This library is free
// software; you can redistribute it and/or modify it under the
// terms of the GNU General Public License as published by the
// Free Software Foundation; either version 2, or (at your option)
// any later version.

// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License along
// with this library; see the file COPYING.  If not, write to the Free
// Software Foundation, 59 Temple Place - Suite 330, Boston, MA 02111-1307,
// USA.

// As a special exception, you may use this file as part of a free software
// library without restriction.  Specifically, if other files instantiate
// templates or use macros or inline functions from this file, or you compile
// this file and link it with other files to produce an executable, this
// file does not by itself cause the resulting executable to be covered by
// the GNU General Public License.  This exception does not however
// invalidate any other reasons why the executable file might be covered by
// the GNU General Public License.

// Copyright (C) 2004 Ami Tavory and Vladimir Dreizin, IBM-HRL.

// Permission to use, copy, modify, sell, and distribute this software
// is hereby granted without fee, provided that the above copyright
// notice appears in all copies, and that both that copyright notice and
// this permission notice appear in supporting documentation. None of
// the above authors, nor IBM Haifa Research Laboratories, make any
// representation about the suitability of this software for any
// purpose. It is provided "as is" without express or implied warranty.

/**
 * @file find_fn_imps.hpp
 * Contains implementations of gp_ht_map_'s find related functions.
 */

PB_ASSOC_CLASS_T_DEC
inline typename PB_ASSOC_CLASS_C_DEC::find_iterator
PB_ASSOC_CLASS_C_DEC::
find(const_key_reference r_key)
{
  PB_ASSOC_DBG_ONLY(assert_valid();)

    return ((find_key_pointer(r_key, my_hash_traits_base::s_store_hash_indicator)));
}

PB_ASSOC_CLASS_T_DEC
inline typename PB_ASSOC_CLASS_C_DEC::const_find_iterator
PB_ASSOC_CLASS_C_DEC::
find(const_key_reference r_key) const
{
  PB_ASSOC_DBG_ONLY(assert_valid();)

    return (const_cast<PB_ASSOC_CLASS_C_DEC& >(*this).
	    find_key_pointer(r_key, my_hash_traits_base::s_store_hash_indicator));
}

PB_ASSOC_CLASS_T_DEC
inline typename PB_ASSOC_CLASS_C_DEC::find_iterator
PB_ASSOC_CLASS_C_DEC::
find_end()
{
  return (NULL);
}

PB_ASSOC_CLASS_T_DEC
inline typename PB_ASSOC_CLASS_C_DEC::const_find_iterator
PB_ASSOC_CLASS_C_DEC::
find_end() const
{
  return (NULL);
}

PB_ASSOC_CLASS_T_DEC
inline typename PB_ASSOC_CLASS_C_DEC::const_data_reference
PB_ASSOC_CLASS_C_DEC::
const_subscript_imp(const_key_reference r_key) const
{
  PB_ASSOC_DBG_ONLY(PB_ASSOC_CLASS_C_DEC::assert_valid();)

    return (const_subscript_imp(r_key, my_hash_traits_base::s_store_hash_indicator));
}

#include <ext/pb_assoc/detail/gp_ht_map_/find_no_store_hash_fn_imps.hpp>
#include <ext/pb_assoc/detail/gp_ht_map_/find_store_hash_fn_imps.hpp>

