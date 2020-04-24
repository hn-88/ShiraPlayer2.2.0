// glQuickText.h
// Geometric text renderer
//
// Copyright ©2009 Brigham Toskin
// This software is part of the Rogue-Opcode game framework. It is distributable
// under the terms of a modified MIT License. You should have received a copy of
// the license in the file LICENSE. If not, see:
// <http://code.google.com/p/rogue-op/wiki/LICENSE>
//
// Based on code written 2004 by <mgix@mgix.com>
// (This code was in the public domain.)
// See <http://www.mgix.com/snippets/?GLQuickText> for details.
//
// Formatting:
//	80 cols ; tabwidth 4
////////////////////////////////////////////////////////////////////////////////


#pragma once


class glQuickText
{
public:

	static void stringBox(
		double      *box,
		double      scale,
		const char  *format,
		...
	);

	static void printfAt(
		double      xPos,
		double      yPos,
		double      zPos,
		double      scale,
		const char  *format,
		...
	);

	static double getFontHeight(
		double scale = 1.0
	);
        //ASAF
        static double getSpaceWidth(
                double scale = 1.0
        );
};
