/*
 * ShiraPlayer(TM)
 * Copyright (C) 2006 Fabien Chereau
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
 
#ifndef STELFONTMGR_H
#define STELFONTMGR_H

#include <map>
#include <QString>

class SFont;

//! @class StelFontMgr
//! Manage fonts for Stellarium. Take into account special font for special language.
//! It also load fonts and store them in a cache to prevent duplication.
//! @author Fabien Chereau <stellarium@free.fr>
class StelFontMgr
{
public:
	StelFontMgr();
	~StelFontMgr();
	
	//! Get the standard font associated to the given language ISO code.
	//! @param langageName the ISO language name such as "fr" or "en" or "*" for default.
	//! @param size the font size in pixels.
	SFont& getStandardFont(const QString& langageName, double size=12.);
	
	//! Get the fixed font associated to the given language ISO code.
	//! @param langageName the ISO language name such as "fr" or "en" or "*" for default.
	//! @param size the font size in pixels.
	SFont& getFixedFont(const QString& langageName, double size=12.);
	
private:
	//! @class FontForLanguage
	//! Class which describes which font to use for a given language ISO code.
	class FontForLanguage
	{
	public:
		QString langageName;
		QString fontFileName;
		double fontScale;
		QString fixedFontFileName;
		double fixedFontScale;
		bool operator == (const FontForLanguage& f) const;
	};	
	
	//! @class LoadedFont
	//! Class which describes a loaded font.
	class LoadedFont
	{
	public:
		LoadedFont(const QString& fileName, int size);
		QString fileName;
		int size;	// floating point scale * 10
	};
	
	//! @class ltLoadedFonts
	//! Comparator for sorting LoadedFonts.
	struct ltLoadedFont
	{
		bool operator()(const LoadedFont l1, const LoadedFont l2) const
		{
			return (l1.fileName<l2.fileName || (l1.fileName==l2.fileName && l1.size<l2.size));
		}
	};	
	
	//! Get the structure describing the fonts and scales to use for a given language.
	FontForLanguage& getFontForLanguage(const QString& langageName);	
	
	//! Load the associations between languages and font file/scaling.
	void loadFontForLanguage(const QString& fontMapFile);
	
	//! Contains a mapping of font/langage
	std::map<QString, FontForLanguage> fontMapping;
	
	//! Keeps references on all loaded fonts
	std::map<LoadedFont, SFont*, ltLoadedFont> loadedFonts;
};

#endif
