/*
 * ShiraPlayer(TM)
 * Copyright (C) 2008 Fabien Chereau
 * Copyright (C) 2011 Asaf Yurdakul
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 * ShiraPlayer is a trademark of Sureyyasoft.
 */
#ifndef _STELPROJECTORTYPE_HPP_
#define _STELPROJECTORTYPE_HPP_

#include <QSharedPointer>

//! @file StelProjectorType.hpp
//! Define the StelProjectorP type.

class StelProjector;

//! @typedef StelProjectorP
//! Shared pointer on a StelProjector instance (implement reference counting)
typedef QSharedPointer<StelProjector> StelProjectorP;

#endif // _STELPROJECTORTYPE_HPP_
