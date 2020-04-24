/*
 * ShiraPlayer(TM)
 * Copyright (C) 2002 Fabien Chereau
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
#include <config.h>

#include <cmath> // std::fmod

#ifdef CYGWIN
 #include <malloc.h>
#endif

#include "StelUtils.hpp"
#include "VecMath.hpp"
#include <QtOpenGL/QtOpenGL>
#include <QString>
#include <QStringList>
#include <QTextStream>
#include <QFile>
#include <QDebug>
#include <QLocale>
#include <QRegExp>
#include <QProcess>
#include <QSysInfo>
#include <cmath> // std::fmod
#include <zlib.h>

namespace StelUtils
{
//! Return the full name of stellarium, i.e. "stellarium 0.9.0"
QString getApplicationName()
{
#ifdef SVN_REVISION
        return QString("Shiraplayer")+" "+PACKAGE_VERSION+" (SVN r"+SVN_REVISION+")";
#else
        return QString("Shiraplayer")+" "+PACKAGE_VERSION;
#endif
}

//! Return the version of stellarium, i.e. "0.9.0"
QString getApplicationVersion()
{
#ifdef SVN_REVISION
	return QString(PACKAGE_VERSION)+" (SVN r"+SVN_REVISION+")";
#else
	return QString(PACKAGE_VERSION);
#endif
}

double hmsToRad(unsigned int h, unsigned int m, double s )
{
	return (double)M_PI/24.*h*2.+(double)M_PI/12.*m/60.+s*M_PI/43200.;
}

double dmsToRad(int d, unsigned int m, double s)
{
	if (d>=0)
		return (double)M_PI/180.*d+(double)M_PI/10800.*m+s*M_PI/648000.;
	return (double)M_PI/180.*d-(double)M_PI/10800.*m-s*M_PI/648000.;
}

/*************************************************************************
 Convert an angle in radian to hms
*************************************************************************/
void radToHms(double angle, unsigned int& h, unsigned int& m, double& s)
{
	angle = std::fmod(angle,2.0*M_PI);
	if (angle < 0.0) angle += 2.0*M_PI; // range: [0..2.0*M_PI)

	angle *= 12./M_PI;

	h = (unsigned int)angle;
	m = (unsigned int)((angle-h)*60);
	s = (angle-h)*3600.-60.*m;
}

/*************************************************************************
 Convert an angle in radian to dms
*************************************************************************/
void radToDms(double angle, bool& sign, unsigned int& d, unsigned int& m, double& s)
{
	angle = std::fmod(angle,2.0*M_PI);
	sign=true;
	if (angle<0)
	{
		angle *= -1;
		sign = false;
	}
	angle *= 180./M_PI;

	d = (unsigned int)angle;
	m = (unsigned int)((angle - d)*60);
	s = (angle-d)*3600-60*m;
}

/*************************************************************************
 Convert an angle in radian to a hms formatted string
 If the minute and second part are null are too small, don't print them
*************************************************************************/
QString radToHmsStrAdapt(double angle)
{
	unsigned int h,m;
	double s;
	QString buf;
	QTextStream ts(&buf);
	StelUtils::radToHms(angle+0.005*M_PI/12/(60*60), h, m, s);
	ts << h << 'h';
	if (std::fabs(s*100-(int)s*100)>=1)
	{
		ts << m << 'm';
		ts.setRealNumberNotation(QTextStream::FixedNotation);
		ts.setPadChar('0');
		ts.setFieldWidth(4);
		ts.setRealNumberPrecision(1);
		ts << s;
		ts.reset();
		ts << 's';
	}
	else if ((int)s!=0)
	{
		ts << m << 'm' << (int)s << 's';
	}
	else if (m!=0)
	{
		ts << m << 'm';
	}
	return buf;
}

/*************************************************************************
 Convert an angle in radian to a hms formatted string
 If decimal is true,  output should be like this: "  16h29m55.3s"
 If decimal is true,  output should be like this: "  16h20m0.4s"
 If decimal is false, output should be like this: "0h26m5s"
*************************************************************************/
QString radToHmsStr(double angle, bool decimal)
{
	unsigned int h,m;
	double s;
	StelUtils::radToHms(angle+0.005*M_PI/12/(60*60), h, m, s);
	int width, precision;
	QString carry;
	if (decimal)
	{
		width=4;
		precision=1;
		carry="60.0";
	}
	else
	{
		width=2;
		precision=0;
		carry="60";
	}

	// handle carry case (when seconds are rounded up)
	if (QString("%1").arg(s, 0, 'f', precision) == carry)
	{
		s=0;
		m+=1;
	}
	if (m==60)
	{
		m=0;
		h+=1;
	}
	if (h==24 && m==0 && s==0)
		h=0;

	return QString("%1h%2m%3s").arg(h, width).arg(m).arg(s, 0, 'f', precision);
}

/*************************************************************************
 Convert an angle in radian to a dms formatted string
 If the minute and second part are null are too small, don't print them
*************************************************************************/
QString radToDmsStrAdapt(double angle, bool useD)
{
	QChar degsign('d');
	if (!useD)
	{
		degsign = 0x00B0;
	}
	bool sign;
	unsigned int d,m;
	double s;
	StelUtils::radToDms(angle+0.005*M_PI/180/(60*60)*(angle<0?-1.:1.), sign, d, m, s);
	QString str;
	QTextStream os(&str);

	os << (sign?'+':'-') << d << degsign;
	if (std::fabs(s*100-(int)s*100)>=1)
	{
		os << m << '\'' << fixed << qSetRealNumberPrecision(2) << qSetFieldWidth(5) << qSetPadChar('0') << s << qSetFieldWidth(0) << '\"';
	}
	else if ((int)s!=0)
	{
		os << m << '\'' << (int)s << '\"';
	}
	else if (m!=0)
	{
		os << m << '\'';
	}
	//qDebug() << "radToDmsStrAdapt(" << angle << ", " << useD << ") = " << str;
	return str;
}


/*************************************************************************
 Convert an angle in radian to a dms formatted string
*************************************************************************/
QString radToDmsStr(double angle, bool decimal, bool useD)
{
	QChar degsign('d');
	if (!useD)
	{
		degsign = 0x00B0;
	}
	bool sign;
	unsigned int d,m;
	double s;
	StelUtils::radToDms(angle+0.005*M_PI/180/(60*60)*(angle<0?-1.:1.), sign, d, m, s);
	QString str;
	QTextStream os(&str);
	os << (sign?'+':'-') << d << degsign;

	int width = 2;
	if (decimal)
	{
		os << qSetRealNumberPrecision(1);
		width = 4;
	}
	else
	{
		os << qSetRealNumberPrecision(0);
		width = 2;
	}

	os << qSetFieldWidth(width) << m << qSetFieldWidth(0) << '\''
		<< fixed << qSetFieldWidth(width) << qSetPadChar('0') << s
		<< qSetFieldWidth(0) << '\"';

	return str;
}

// Obtains a Vec3f from a string with the form x,y,z
Vec3f strToVec3f(const QStringList& s)
{
	if (s.size()<3)
		 return Vec3f(0.f,0.f,0.f);

	return Vec3f(s[0].toFloat(),s[1].toFloat(),s[2].toFloat());
}

Vec3f strToVec3f(const QString& s)
{
	return strToVec3f(s.split(","));
}

Vec4d strToVec4d(const QStringList &s)
{
    if(s.size()<4)
        return Vec4d(0.0,0.0,0.0,0.0);

    return Vec4d(s[0].toDouble(), s[1].toDouble(), s[2].toDouble(), s[3].toDouble());
}

Vec4d strToVec4d(const QString& str)
{
    return strToVec4d(str.split(","));
}

QString vec3fToStr(const Vec3f &v)
{
    return QString("%1,%2,%3")
        .arg(v[0],0,'f',6)
        .arg(v[1],0,'f',6)
        .arg(v[2],0,'f',6);
}

QString vec4dToStr(const Vec4d &v)
{
    return QString("%1,%2,%3,%4")
        .arg(v[0],0,'f',10)
        .arg(v[1],0,'f',10)
        .arg(v[2],0,'f',10)
        .arg(v[3],0,'f',10);
}

// Converts a Vec3f to HTML color notation.
QString vec3fToHtmlColor(const Vec3f& v)
{
	return QString("#%1%2%3")
		.arg(qMin(255, int(v[0] * 255)), 2, 16, QChar('0'))
		.arg(qMin(255, int(v[1] * 255)), 2, 16, QChar('0'))
		.arg(qMin(255, int(v[2] * 255)), 2, 16, QChar('0'));
}

Vec3f htmlColorToVec3f(const QString& c)
{
	Vec3f v;
	QRegExp re("^#([0-9a-fA-F]{2})([0-9a-fA-F]{2})([0-9a-fA-F]{2})$");
	if (re.exactMatch(c))
	{
		bool ok;
		int i = re.capturedTexts().at(1).toInt(&ok, 16);
		v[0] = (float)i / 255.;
		i = re.capturedTexts().at(2).toInt(&ok, 16);
		v[1] = (float)i / 255.;
		i = re.capturedTexts().at(3).toInt(&ok, 16);
		v[2] = (float)i / 255.;
	}
	else
	{
		v[0] = 0.;
		v[1] = 0.;
		v[2] = 0.;
	}
	return v;
}

void spheToRect(double lng, double lat, Vec3d& v)
{
	const double cosLat = cos(lat);
	v.set(cos(lng) * cosLat, sin(lng) * cosLat, sin(lat));
}

void spheToRect(float lng, float lat, Vec3f& v)
{
	const double cosLat = cos(lat);
	v.set(cos(lng) * cosLat, sin(lng) * cosLat, sin(lat));
}

void rectToSphe(double *lng, double *lat, const Vec3d& v)
{
	double r = v.length();
	*lat = asin(v[2]/r);
	*lng = atan2(v[1],v[0]);
}

void rectToSphe(float *lng, float *lat, const Vec3d& v)
{
	double r = v.length();
	*lat = asin(v[2]/r);
	*lng = atan2(v[1],v[0]);
}

void rectToSphe(float *lng, float *lat, const Vec3f& v)
{
	double r = v.length();
	*lat = asin(v[2]/r);
	*lng = atan2(v[1],v[0]);
}

void rectToSphe(double *lng, double *lat, const Vec3f& v)
{
	double r = v.length();
	*lat = asin(v[2]/r);
	*lng = atan2(v[1],v[0]);
}

double getDecAngle(const QString& str)
{
	QRegExp re1("^\\s*([\\+\\-])?\\s*(\\d+)\\s*([hHDd\xBA])\\s*(\\d+)\\s*['Mm]\\s*(\\d+(\\.\\d+)?)\\s*[\"Ss]\\s*([NSEWnsew])?\\s*$"); // DMS/HMS
	QRegExp re2("^\\s*([\\+\\-])?\\s*(\\d+(\\.\\d+)?).?([NSEWnsew])?\\s*$"); // Decimal

	if (re1.exactMatch(str))
	{
		bool neg = (re1.capturedTexts().at(1) == "-");
		double d = re1.capturedTexts().at(2).toDouble();
		double m = re1.capturedTexts().at(4).toDouble();
		double s = re1.capturedTexts().at(5).toDouble();
		if (re1.capturedTexts().at(3).toUpper() == "H")
		{
			d *= 15;
			m *= 15;
			s *= 15;
		}
		QString cardinal = re1.capturedTexts().at(7);
		double deg = d + (m/60) + (s/3600);
		if (cardinal.toLower() == "s" || cardinal.toLower() == "w" || neg)
			deg *= -1.;
		return (deg * 2 * M_PI / 360.);
	}
	else if (re2.exactMatch(str))
	{
		bool neg = (re2.capturedTexts().at(1) == "-");
		double deg = re2.capturedTexts().at(2).toDouble();
		QString cardinal = re2.capturedTexts().at(4);
		if (cardinal.toLower() == "s" || cardinal.toLower() == "w" || neg)
			deg *= -1.;
		return (deg * 2 * M_PI / 360.);
	}

	qDebug() << "getDecAngle failed to parse angle string:" << str;
	return -0.0;
}

// Check if a number is a power of 2
bool isPowerOfTwo(int value)
{
	return (value & -value) == value;
}

// Return the first power of two bigger than the given value
int getBiggerPowerOfTwo(int value)
{
	int p=1;
	while (p<value)
		p<<=1;
	return p;
}

// Return the inverse sinus hyperbolic of z
double asinh(double z)
{
	return std::log(z+std::sqrt(z*z+1));
}

/*************************************************************************
 Convert a QT QDateTime class to julian day
*************************************************************************/
double qDateTimeToJd(const QDateTime& dateTime)
{
   return (double)(dateTime.date().toJulianDay())+(double)1./(24*60*60*1000)*QTime(0, 0, 0, 0).msecsTo(dateTime.time())-0.5;
}

QDateTime jdToQDateTime(const double& jd)
{
	int year, month, day;
	getDateFromJulianDay(jd, &year, &month, &day);
	QDateTime result = QDateTime::fromString(QString("%1.%2.%3").arg(year, 4, 10, QLatin1Char('0')).arg(month).arg(day), "yyyy.M.d");
	result.setTime(jdFractionToQTime(jd));
	return result;
}

// based on QDateTime's original handling, but expanded to handle 0.0 and earlier.
void getDateFromJulianDay(double jd, int *year, int *month, int *day)
{
	int y, m, d;

	// put us in the right calendar day for the time of day.
	double fraction = jd - floor(jd);
	if (fraction >= .5)
	{
		jd += 1.0;
	}

	if (jd >= 2299161)
	{
		// Gregorian calendar starting from October 15, 1582
		// This algorithm is from Henry F. Fliegel and Thomas C. Van Flandern
		qulonglong ell, n, i, j;
		ell = qulonglong(floor(jd)) + 68569;
		n = (4 * ell) / 146097;
		ell = ell - (146097 * n + 3) / 4;
		i = (4000 * (ell + 1)) / 1461001;
		ell = ell - (1461 * i) / 4 + 31;
		j = (80 * ell) / 2447;
		d = ell - (2447 * j) / 80;
		ell = j / 11;
		m = j + 2 - (12 * ell);
		y = 100 * (n - 49) + i + ell;
	}
	else
	{
		// Julian calendar until October 4, 1582
		// Algorithm from Frequently Asked Questions about Calendars by Claus Toendering
		int julianDay = (int)floor(jd);
		julianDay += 32082;
		int dd = (4 * julianDay + 3) / 1461;
		int ee = julianDay - (1461 * dd) / 4;
		int mm = ((5 * ee) + 2) / 153;
		d = ee - (153 * mm + 2) / 5 + 1;
		m = mm + 3 - 12 * (mm / 10);
		y = dd - 4800 + (mm / 10);
	}
	*year = y;
	*month = m;
	*day = d;
}

void getTimeFromJulianDay(double julianDay, int *hour, int *minute, int *second)
{
	double frac = julianDay - (floor(julianDay));
	int s = (int)floor(frac * 24 * 60 * 60);

	*hour = ((s / (60 * 60))+12)%24;
	*minute = (s/(60))%60;
	*second = s % 60;
}

QString sixIntsToIsoString( int year, int month, int day, int hour, int minute, int second )
{
	// formatting a negative doesnt work the way i expect

	QString dt = QString("%1-%2-%3T%4:%5:%6")
				 .arg((year >= 0 ? year : -1* year),4,10,QLatin1Char('0'))
				 .arg(month,2,10,QLatin1Char('0'))
				 .arg(day,2,10,QLatin1Char('0'))
				 .arg(hour,2,10,QLatin1Char('0'))
				 .arg(minute,2,10,QLatin1Char('0'))
				 .arg(second,2,10,QLatin1Char('0'));

	if (year < 0)
	{
		dt.prepend("-");
	}
	return dt;
}

QString jdToIsoString(double jd)
{
	int year, month, day, hour, minute, second;
	getDateFromJulianDay(jd, &year, &month, &day);
	getTimeFromJulianDay(jd, &hour, &minute, &second);

	return sixIntsToIsoString(year, month, day, hour, minute, second);
}

// Format the date per the fmt.
QString localeDateString(int year, int month, int day, int dayOfWeek, QString fmt)
{
	/* we have to handle the year zero, and the years before qdatetime can represent. */
	const QLatin1Char quote('\'');
	QString out;
	int quotestartedat = -1;

	for (int i = 0; i < (int)fmt.length(); i++)
	{
		if (fmt.at(i) == quote)
		{
			if (quotestartedat >= 0)
			{
				if ((quotestartedat+1) == i)
				{
					out += quote;
					quotestartedat = -1;
				}
				else
				{
					quotestartedat = -1;
				}
			}
			else
			{
				quotestartedat = i;
			}
		}
		else if (quotestartedat > 0)
		{
			out += fmt.at(i);
		}
		else if (fmt.at(i) == QLatin1Char('d') ||
				 fmt.at(i) == QLatin1Char('M') ||
				 fmt.at(i) == QLatin1Char('y'))
		{
			int j = i+1;
			while ((j < fmt.length()) && (fmt.at(j) == fmt.at(i)) && (4 >= (j-i+1)))
			{
				j++;
			}

			QString frag = fmt.mid(i,(j-i));

			if (frag == "d")
			{
				out += QString("%1").arg(day);
			}
			else if (frag == "dd")
			{
				out += QString("%1").arg(day, 2, 10, QLatin1Char('0'));
			}
			else if (frag == "ddd")
			{
				out += QDate::shortDayName(dayOfWeek+1);
			}
			else if (frag == "dddd")
			{
				out += QDate::longDayName(dayOfWeek+1);
			}
			else if (frag == "M")
			{
				out += QString("%1").arg(month);
			}
			else if (frag == "MM")
			{
				out += QString("%1").arg(month, 2, 10, QLatin1Char('0'));
			}
			else if (frag == "MMM")
			{
				out += QDate::shortMonthName(month);
			}
			else if (frag == "MMMM")
			{
				out += QDate::longMonthName(month);
			}
			else if (frag == "y")
			{
				out += frag;
			}
			else if (frag == "yy")
			{
				int dispyear = year % 100;
				out += QString("%1").arg(dispyear,2,10,QLatin1Char('0'));
			}
			else if (frag == "yyy")
			{
				// assume greedy: understand yy before y.
				int dispyear = year % 100;
				out += QString("%1").arg(dispyear,2,10,QLatin1Char('0'));
				out += QLatin1Char('y');
			}
			else if (frag == "yyyy")
			{
				int dispyear = (year >= 0 ? year : -1 * year);
				if (year <  0)
				{
					out += QLatin1Char('-');
				}
				out += QString("%1").arg(dispyear,4,10,QLatin1Char('0'));
			}

			i = j-1;
		}
		else
		{
			out += fmt.at(i);
		}


	}

	return out;
}

//! try to get a reasonable locale date string from the system, trying to work around
//! limitations of qdatetime for large dates in the past.  see QDateTime::toString().
QString localeDateString(int year, int month, int day, int dayOfWeek)
{

	// try the QDateTime first
	QDate test(year, month, day);

	// try to avoid QDate's non-astronomical time here, don't do BCE or year 0.
	if (year > 0 && test.isValid() && !test.toString(Qt::LocaleDate).isEmpty())
	{
		return test.toString(Qt::LocaleDate);
	}
	else
	{
		return localeDateString(year,month,day,dayOfWeek,QLocale().dateFormat(QLocale::ShortFormat));
	}
}


//! use QDateTime to get a Julian Date from the system's current time.
//! this is an acceptable use of QDateTime because the system's current
//! time is more than likely always going to be expressible by QDateTime.
double getJDFromSystem(void)
{
	return qDateTimeToJd(QDateTime::currentDateTime().toUTC());
}

double qTimeToJDFraction(const QTime& time)
{
    return (double)1./(24*60*60*1000)*QTime(0,0,0,0).msecsTo(time)-0.5;
}

QTime jdFractionToQTime(const double jd)
{
	double decHours = std::fmod(jd+0.5, 1.0);
	int hours = (int)(decHours/0.041666666666666666666);
	int mins = (int)((decHours-(hours*0.041666666666666666666))/0.00069444444444444444444);
	return QTime::fromString(QString("%1.%2").arg(hours).arg(mins), "h.m");
}

// Use Qt's own sense of time and offset instead of platform specific code.
float getGMTShiftFromQT(double JD)
{
	int year, month, day, hour, minute, second;
	getDateFromJulianDay(JD, &year, &month, &day);
	getTimeFromJulianDay(JD, &hour, &minute, &second);
	// as analogous to second statement in getJDFromDate, nkerr
	if ( year <= 0 )
	{
		year = year - 1;
	}
	QDateTime current(QDate(year, month, day), QTime(hour, minute, second));
	if (! current.isValid())
	{
		//qWarning() << "JD " << QString("%1").arg(JD) << " out of bounds of QT help with GMT shift, using current datetime";
		// Assumes the GMT shift was always the same before year -4710
		current = QDateTime(QDate(-4710, month, day), QTime(hour, minute, second));
	}

	//Both timezones should be set to UTC because secsTo() converts both
	//times to UTC if their zones have different daylight saving time rules.
	QDateTime local = current; local.setTimeSpec(Qt::UTC);
	QDateTime universal = current.toUTC();

	int shiftInSeconds = universal.secsTo(local);
	float shiftInHours = shiftInSeconds / 3600.0f;
	return shiftInHours;
}

// UTC !
bool getJDFromDate(double* newjd, int y, int m, int d, int h, int min, int s)
{
	double deltaTime = (h / 24.0) + (min / (24.0*60.0)) + (s / (24.0 * 60.0 * 60.0)) - 0.5;
	QDate test((y <= 0 ? y-1 : y), m, d);
	// if QDate will oblige, do so.
	if ( test.isValid() )
	{
		double qdjd = (double)test.toJulianDay();
		qdjd += deltaTime;
		*newjd = qdjd;
		return true;
	} else
	{
		double jd = (double)((1461 * (y + 4800 + (m - 14) / 12)) / 4 + (367 * (m - 2 - 12 * ((m - 14) / 12))) / 12 - (3 * ((y + 4900 + (m - 14) / 12) / 100)) / 4 + d - 32075) - 38;
		jd += deltaTime;
		*newjd = jd;
		return true;
	}
	return false;
}

double getJDFromDate_alg2(int y, int m, int d, int h, int min, int s)
{
	double extra = (100.0* y) + m - 190002.5;
	double rjd = 367.0 * y;
	rjd -= floor(7.0*(y+floor((m+9.0)/12.0))/4.0);
	rjd += floor(275.0*m/9.0) ;
	rjd += d;
	rjd += (h + (min + s/60.0)/60.)/24.0;
	rjd += 1721013.5;
	rjd -= 0.5*extra/std::fabs(extra);
	rjd += 0.5;
	return rjd;
}

int numberOfDaysInMonthInYear(int month, int year)
{
	switch(month)
	{
		case 1:
		case 3:
		case 5:
		case 7:
		case 8:
		case 10:
		case 12:
			return 31;
			break;
		case 4:
		case 6:
		case 9:
		case 11:
			return 30;
			break;

		case 2:
			if ( year > 1582 )
			{
				if ( year % 4 == 0 )
				{
					if ( year % 100 == 0 )
					{
						if ( year % 400 == 0 )
						{
							return 29;
						}
						else
						{
							return 28;
						}
					}
					else
					{
						return 29;
					}
				}
				else
				{
					return 28;
				}
			}
			else
			{
				if ( year % 4 == 0 )
				{
					return 29;
				}
				else
				{
					return 28;
				}
			}
			break;

		case 0:
			return numberOfDaysInMonthInYear(12, year-1);
			break;
		case 13:
			return numberOfDaysInMonthInYear(1, year+1);
			break;
		default:
			break;
	}

	return 0;
}

//! given the submitted year/month/day hour:minute:second, try to
//! normalize into an actual year/month/day.  values can be positive, 0,
//! or negative.  start assessing from seconds to larger increments.
bool changeDateTimeForRollover(int oy, int om, int od, int oh, int omin, int os,
				int* ry, int* rm, int* rd, int* rh, int* rmin, int* rs)
{
	bool change = false;

	while ( os > 59 ) {
		os -= 60;
		omin += 1;
		change = true;
	}
	while ( os < 0 ) {
		os += 60;
		omin -= 1;
		change = true;
	}

	while (omin > 59 ) {
		omin -= 60;
		oh += 1;
		change = true;
	}
	while (omin < 0 ) {
		omin += 60;
		oh -= 1;
		change = true;
	}

	while ( oh > 23 ) {
		oh -= 24;
		od += 1;
		change = true;
	}
	while ( oh < 0 ) {
		oh += 24;
		od -= 1;
		change = true;
	}

	while ( od > numberOfDaysInMonthInYear(om, oy) ) {
		od -= numberOfDaysInMonthInYear(om, oy);
		om++;
		if ( om > 12 ) {
		om -= 12;
		oy += 1;
		}
		change = true;
	}
	while ( od < 1 ) {
		od += numberOfDaysInMonthInYear(om-1,oy);
		om--;
		if ( om < 1 ) {
		om += 12;
		oy -= 1;
		}
		change = true;
	}

	while ( om > 12 ) {
		om -= 12;
		oy += 1;
		change = true;
	}
	while ( om < 1 ) {
		om += 12;
		oy -= 1;
		change = true;
	}

	// and the julian-gregorian epoch hole: round up to the 15th
	if ( oy == 1582 && om == 10 && ( od > 4 && od < 15 ) ) {
		od = 15;
		change = true;
	}

	if ( change ) {
		*ry = oy;
		*rm = om;
		*rd = od;
		*rh = oh;
		*rmin = omin;
		*rs = os;
	}
	return change;
}

void debugQVariantMap(const QVariant& m, const QString& indent, const QString& key)
{
	QVariant::Type t = m.type();
	if (t == QVariant::Map)
	{
		qDebug() << indent + key + "(map):";
		QList<QString> keys = m.toMap().keys();
		qSort(keys);
		foreach(QString k, keys)
		{
			debugQVariantMap(m.toMap()[k], indent + "    ", k);
		}
	}
	else if (t == QVariant::List)
	{
		qDebug() << indent + key + "(list):";
		foreach(QVariant item, m.toList())
		{
			debugQVariantMap(item, indent + "    ");
		}
	}
	else
		qDebug() << indent + key + " => " + m.toString();
}


QList<int> getIntsFromISO8601String(const QString & dt)
{
	// Represents a valid, complete date string.
	QRegExp finalRe("(-0*[1-9][0-9]{0,5}|0+|0*[1-9][0-9]{0,5})-(0[1-9]|1[0-2])-(0[1-9]|[12][0-9]|3[10])[T ]([01][0-9]|2[0123]):([012345][0-9]):([012345][0-9])");

	QList<int> retval;
	if (finalRe.exactMatch(dt))
	{
		QStringList number_strings = finalRe.capturedTexts();
		bool ok;
		int v;
		for (int i = 1; i < number_strings.size(); i++)
		{
			qWarning() << ":: at capture " << i << " got a " << number_strings[i];
			ok = true;
			v = number_strings[i].toInt(&ok, 10);
			qWarning() << "  :: and it was a " << v << " " << ok;
			if (ok)
			{
				retval.push_back(v);
			}
			else
			{
				retval.clear();
				qWarning() << "StelUtils::getIntsFromISO8601String: input string failed to be an exact date at capture " << i << ", returning nothing: " << dt;
				break;
			}
		}
	}
	else
	{
		qWarning() << "StelUtils::getIntsFromISO8601String: input string failed to be an exact date, returning nothing: " << dt;
	}
	return retval;
}

//! Uncompress gzip or zlib compressed data.
QByteArray uncompress(const QByteArray& data)
{
    if (data.size() <= 4)
        return QByteArray();

    //needed for const-correctness, no deep copy performed
    QByteArray dataNonConst(data);
    QBuffer buf(&dataNonConst);
    buf.open(QIODevice::ReadOnly);

    return uncompress(buf);
}

//! Uncompress (gzip/zlib) data from this QIODevice, which must be open and readable.
//! @param device The device to read from, must already be opened with an OpenMode supporting reading
//! @param maxBytes The max. amount of bytes to read from the device, or -1 to read until EOF.  Note that it
//! always stops when inflate() returns Z_STREAM_END. Positive values can be used for interleaving compressed data
//! with other data.
QByteArray uncompress(QIODevice& device, qint64 maxBytes)
{
    // this is a basic zlib decompression routine, similar to:
    // http://zlib.net/zlib_how.html

    // buffer size 256k, zlib recommended size
    static const int CHUNK = 262144;
    QByteArray readBuffer(CHUNK, 0);
    QByteArray inflateBuffer(CHUNK, 0);
    QByteArray out;

    // zlib stream
    z_stream strm;
    strm.zalloc = Z_NULL;
    strm.zfree = Z_NULL;
    strm.opaque = Z_NULL;
    strm.avail_in = Z_NULL;
    strm.next_in = Z_NULL;

    // the amount of bytes already read from the device
    qint64 bytesRead = 0;

    // initialize zlib
    // 15 + 32 for gzip automatic header detection.
    int ret = inflateInit2(&strm, 15 +  32);
    if (ret != Z_OK)
    {
        qWarning()<<"zlib init error ("<<ret<<"), can't uncompress";
        if(strm.msg)
            qWarning()<<"zlib message: "<<QString(strm.msg);
        return QByteArray();
    }

    //zlib double loop - one for reading from file, one for inflating
    do
    {
        qint64 bytesToRead = CHUNK;
        if(maxBytes>=0)
        {
            //check if we reach the desired limit with the next read
            bytesToRead = qMin((qint64)CHUNK,maxBytes-bytesRead);
        }

        if(bytesToRead==0)
            break;

        //perform read from device
        qint64 read = device.read(readBuffer.data(), bytesToRead);
        if (read<0)
        {
            qWarning()<<"Error while reading from device";
            inflateEnd(&strm);
            return QByteArray();
        }

        bytesRead += read;
        strm.next_in = reinterpret_cast<Bytef*>(readBuffer.data());
        strm.avail_in = read;

        if(read==0)
            break;

        //inflate loop
        do
        {
            strm.avail_out = CHUNK;
            strm.next_out = reinterpret_cast<Bytef*>(inflateBuffer.data());
            ret = inflate(&strm,Z_NO_FLUSH);
            Q_ASSERT(ret != Z_STREAM_ERROR); // must never happen, indicates a programming error

            if(ret < 0 || ret == Z_NEED_DICT)
            {
                qWarning()<<"zlib inflate error ("<<ret<<"), can't uncompress";
                if(strm.msg)
                    qWarning()<<"zlib message: "<<QString(strm.msg);
                inflateEnd(&strm);
                return QByteArray();
            }

            out.append(inflateBuffer.constData(), CHUNK - strm.avail_out);

        }while(strm.avail_out == 0); //if zlib has more data for us, repeat

    }while(ret!=Z_STREAM_END);

    // close zlib
    inflateEnd(&strm);

    if(ret!=Z_STREAM_END)
    {
        qWarning()<<"Premature end of compressed stream";
        if(strm.msg)
            qWarning()<<"zlib message: "<<QString(strm.msg);
        return QByteArray();
    }

    return out;
}

bool isUnicodeInclude(QString str)
{
    if(str.toLatin1() == str)
        return false;
    return true;
}

QString getOperatingSystemInfo()
{
    QString OS = "Unknown operating system";

    #ifdef Q_OS_WIN
    switch(QSysInfo::WindowsVersion)
    {
        case QSysInfo::WV_95:
            OS = "Windows 95";
            break;
        case QSysInfo::WV_98:
            OS = "Windows 98";
            break;
        case QSysInfo::WV_Me:
            OS = "Windows Me";
            break;
        case QSysInfo::WV_NT:
            OS = "Windows NT";
            break;
        case QSysInfo::WV_2000:
            OS = "Windows 2000";
            break;
        case QSysInfo::WV_XP:
            OS = "Windows XP";
            break;
        case QSysInfo::WV_2003:
            OS = "Windows Server 2003";
            break;
        case QSysInfo::WV_VISTA:
            OS = "Windows Vista";
            break;
        case QSysInfo::WV_WINDOWS7:
            OS = "Windows 7";
            break;
        case QSysInfo::WV_WINDOWS8:
            OS = "Windows 8";
            break;
        case QSysInfo::WV_WINDOWS8_1:
            OS = "Windows 8.1";
            break;
        case 0x00c0:
            OS = "Windows 10";
            break;
        default:
            OS = "Unsupported Windows version";
            break;
    }

    // somebody writing something useful for Macs would be great here
    #elif defined Q_OS_MAC
    switch(QSysInfo::MacintoshVersion)
    {
        case QSysInfo::MV_PANTHER:
            OS = "Mac OS X 10.3 series";
            break;
        case QSysInfo::MV_TIGER:
            OS = "Mac OS X 10.4 series";
            break;
        case QSysInfo::MV_LEOPARD:
            OS = "Mac OS X 10.5 series";
            break;
        case QSysInfo::MV_SNOWLEOPARD:
            OS = "Mac OS X 10.6 series";
            break;
        case QSysInfo::MV_LION:
            OS = "Mac OS X 10.7 series";
            break;
        case QSysInfo::MV_MOUNTAINLION:
            OS = "Mac OS X 10.8 series";
            break;
        case QSysInfo::MV_MAVERICKS:
            OS = "Mac OS X 10.9 series";
            break;
        #ifdef MV_YOSEMITE
        case QSysInfo::MV_YOSEMITE:
            OS = "Mac OS X 10.10 series";
            break;
        #endif
        default:
            OS = "Unsupported Mac version";
            break;
    }

    #elif defined Q_OS_LINUX
    QFile procVersion("/proc/version");
    if(!procVersion.open(QIODevice::ReadOnly | QIODevice::Text))
        OS = "Unknown Linux version";
    else
    {
        QString version = procVersion.readAll();
        if(version.right(1) == "\n")
            version.chop(1);
        OS = version;
        procVersion.close();
    }
    #elif defined Q_OS_BSD4
    // Check FreeBSD, NetBSD, OpenBSD and DragonFly BSD
    QProcess uname;
    uname.start("/usr/bin/uname -srm");
    uname.waitForStarted();
    uname.waitForFinished();
    const QString BSDsystem = uname.readAllStandardOutput();
    OS = BSDsystem.trimmed();
    #endif

    return OS;
}

} // end of the StelUtils namespace

