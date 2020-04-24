/*
 * ShiraPlayer(TM)
 * Copyright (C) 2007 Fabien Chereau
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
#ifndef _STELOBJECTTYPE_HPP_
#define _STELOBJECTTYPE_HPP_

#include <QSharedPointer>

//! Special version of QSharedPointer which by default doesn't delete the referenced pointer when
//! the reference count reaches 0.
template <class T> class QSharedPointerNoDelete : public QSharedPointer<T>
{
public:
	QSharedPointerNoDelete() {;}
	QSharedPointerNoDelete(T *ptr) : QSharedPointer<T>(ptr, QSharedPointerNoDelete::noDelete) {;}
	QSharedPointerNoDelete(T *ptr, bool own) : QSharedPointer<T>(ptr) {Q_ASSERT(own==true);}
	QSharedPointerNoDelete(const QSharedPointer<T>& ptr) : QSharedPointer<T>(ptr) {;}
	static void noDelete(T *ptr) {;}
	inline operator const QSharedPointer<T>&() const {return *(static_cast<QSharedPointer<T> >(this));}
};

//! @file StelObjectType.hpp
//! Define the StelObjectP type.

class StelObject;

//! @typedef StelObjectP
//! Intrusive pointer used to manage StelObject with smart pointers
typedef QSharedPointerNoDelete<StelObject> StelObjectP;

#endif // _STELOBJECTTYPE_HPP_
