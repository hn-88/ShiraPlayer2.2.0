/*
 * Author and Copyright of this file and of the stellarium telescope feature:
 * Johannes Gajdosik, 2006
 *
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
 */

#include "Telescope.hpp"
#include "StelUtils.hpp"
#include "StelTranslator.hpp"
#include "StelCore.hpp"

#include <cmath>

#include <QDebug>
#include <QtNetwork/QHostAddress>
#include <QtNetwork/QHostInfo>
#include <QRegExp>
#include <QString>
#include <QtNetwork/QTcpSocket>
#include <QTextStream>

#ifdef Q_OS_WIN
	#include <windows.h> // GetSystemTimeAsFileTime()
#else
	#include <sys/time.h>
#endif

namespace Stel {

//! Example Telescope class. A physical telescope does not exist.
//! This can be used as a starting point for implementing a derived
//! Telescope class.
class TelescopeDummy : public Telescope
{
public:
	TelescopeDummy(const QString &name,const QString &params) : Telescope(name)
	{
		desired_pos[0] = XYZ[0] = 1.0;
		desired_pos[1] = XYZ[1] = 0.0;
		desired_pos[2] = XYZ[2] = 0.0;
	}
private:
	bool isConnected(void) const
	{
		return true;
	}
	bool hasKnownPosition(void) const
	{
		return true;
	}
	Vec3d getJ2000EquatorialPos(const StelNavigator *nav=0) const
	{
		return XYZ;
	}
	bool prepareCommunication()
	{
		XYZ = XYZ*31.0+desired_pos;
		const double lq = XYZ.lengthSquared();
		if (lq > 0.0) XYZ *= (1.0/sqrt(lq));
		else XYZ = desired_pos;
		return true;
	}
	void telescopeGoto(const Vec3d &j2000Pos)
	{
		desired_pos = j2000Pos;
		desired_pos.normalize();
	}
	Vec3d XYZ; // j2000 position
	Vec3d desired_pos;
};

Telescope *Telescope::create(const QString &url)
{
	// example url: My_first_telescope:TCP:localhost:10000:500000
	// split to:
	// name    = My_first_telescope
	// type    = TCP
	// params  = localhost:10000:500000
	//
	// The params part is optional.  We will use QRegExp to validate
	// the url and extact the components.

	// note, in a reg exp, [^:] matches any chararacter except ':'
	QRegExp recRx("^([^:]*):([^:]*)(:(.*))?$");
	QString name, type, params;
	if (recRx.exactMatch(url))
	{
		// trimmed removes whitespace on either end of a QString
		name = recRx.capturedTexts().at(1).trimmed();
		type = recRx.capturedTexts().at(2).trimmed();
		params = recRx.capturedTexts().at(4).trimmed();
	}
	else
	{
		qWarning() << "WARNING - telescope definition" << url << "not recognised";
		return NULL;
	}

	qDebug() << "Creating telescope" << url << "; name/type/params:" << name << type << params;

	Telescope * newTelescope = 0;
	if (type == "Dummy")
	{
		newTelescope = new TelescopeDummy(name,params);
	}
	else if (type == "TCP")
	{
		newTelescope = new TelescopeTcp(name,params);
	}
	else
	{
		qWarning() << "WARNING - unknown telescope type" << type << "- not creating a telescope object for url" << url;
	}
	if (newTelescope && !newTelescope->isInitialized())
	{
		delete newTelescope;
		newTelescope = 0;
	}
	return newTelescope;
}


Telescope::Telescope(const QString &name) : name(name)
{
	nameI18n = name;
}

QString Telescope::getInfoString(const StelCore* core, const InfoStringGroup& flags) const
{
	QString str;
	QTextStream oss(&str);
	if (flags&Name)
	{
		oss << "<h2>" << nameI18n << "</h2>";
	}

	oss << getPositionInfoString(core, flags);

	postProcessInfoString(str, flags);

	return str;
}

}//namespace Stel

//! returns the current system time in microseconds since the Epoch
qint64 getNow(void)
{
// At the moment this can't be done in a platform-independent way with Qt
// (QDateTime and QTime don't support microsecond precision)
#ifdef Q_OS_WIN
	FILETIME file_time;
	GetSystemTimeAsFileTime(&file_time);
	return (*((__int64*)(&file_time))/10) - 86400000000LL*134774;
#else
	struct timeval tv;
	gettimeofday(&tv,0);
	return tv.tv_sec * 1000000LL + tv.tv_usec;
#endif
}

namespace Stel {

TelescopeTcp::TelescopeTcp(const QString &name,const QString &params) : Telescope(name), tcpSocket(new QTcpSocket()),
		end_position(positions+(sizeof(positions)/sizeof(positions[0])))
{
	hangup();

	// Example params:
	// localhost:10000:500000
	// split into:
	// host       = localhost
	// port       = 10000 (int)
	// time_delay = 500000 (int)

	QRegExp paramRx("^([^:]*):(\\d+):(\\d+)$");
	QString host;

	if (paramRx.exactMatch(params))
	{
		// I will not use the ok param to toInt as the
		// QRegExp only matches valid integers.
		host		= paramRx.capturedTexts().at(1).trimmed();
		port		= paramRx.capturedTexts().at(2).toInt();
		time_delay	= paramRx.capturedTexts().at(3).toInt();
	}
	else
	{
		qWarning() << "WARNING - incorrect TelescopeTcp parameters";
		return;
	}

	qDebug() << "TelescopeTcp paramaters host, port, time_delay:" << host << port << time_delay;

	if (port <= 0 || port > 0xFFFF)
	{
		qWarning() << "ERROR creating TelescopeTcp - port not valid (should be less than 32767)";
		return;
	}


	if (time_delay <= 0 || time_delay > 10000000)
	{
		qWarning() << "ERROR creating TelescopeTcp - time_delay not valid (should be less than 10000000)";
		return;
	}

	//BM: TODO: This may cause some delay when there are more telescopes
	QHostInfo info = QHostInfo::fromName(host);
	if (info.error())
	{
		qDebug() << "ERROR creating TelescopeTcp: error looking up host " << host << ":\n" << info.errorString();
		return;
	}
	//BM: is info.addresses().isEmpty() if there's no error?
	address = info.addresses().first();

	end_of_timeout = -0x8000000000000000LL;

	resetPositions();

	connect(tcpSocket, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(socketFailed(QAbstractSocket::SocketError)));
}

void TelescopeTcp::hangup(void)
{
	if (tcpSocket->isValid())
	{
		tcpSocket->abort();// Or maybe tcpSocket->close()?
	}

	readBufferEnd = readBuffer;
	writeBufferEnd = writeBuffer;
	wait_for_connection_establishment = false;

	resetPositions();
}

//! resets/initializes the array of positions kept for position interpolation
void TelescopeTcp::resetPositions()
{
	for (position_pointer = positions; position_pointer < end_position; position_pointer++)
	{
		position_pointer->server_micros = 0x7FFFFFFFFFFFFFFFLL;
		position_pointer->client_micros = 0x7FFFFFFFFFFFFFFFLL;
		position_pointer->pos[0] = 0.0;
		position_pointer->pos[1] = 0.0;
		position_pointer->pos[2] = 0.0;
		position_pointer->status = 0;
	}
	position_pointer = positions;
}

//! queues a GOTO command with the specified position to the write buffer.
//! For the data format of the command see the
//! "Stellarium telescope control protocol" text file
void TelescopeTcp::telescopeGoto(const Vec3d &j2000Pos)
{
	if (isConnected())
	{
		if (writeBufferEnd - writeBuffer + 20 < (int)sizeof(writeBuffer))
		{
			const double ra_signed = atan2(j2000Pos[1], j2000Pos[0]);
			//Workaround for the discrepancy in precision between Windows/Linux/PPC Macs and Intel Macs:
			const double ra = (ra_signed >= 0) ? ra_signed : (ra_signed + 2.0 * M_PI);
			const double dec = atan2(j2000Pos[2], sqrt(j2000Pos[0]*j2000Pos[0]+j2000Pos[1]*j2000Pos[1]));
			unsigned int ra_int = (unsigned int)floor(0.5 + ra*(((unsigned int)0x80000000)/M_PI));
			int dec_int = (int)floor(0.5 + dec*(((unsigned int)0x80000000)/M_PI));
			// length of packet:
			*writeBufferEnd++ = 20;
			*writeBufferEnd++ = 0;
			// type of packet:
			*writeBufferEnd++ = 0;
			*writeBufferEnd++ = 0;
			// client_micros:
			qint64 now = getNow();
			*writeBufferEnd++ = now;
			now>>=8;
			*writeBufferEnd++ = now;
			now>>=8;
			*writeBufferEnd++ = now;
			now>>=8;
			*writeBufferEnd++ = now;
			now>>=8;
			*writeBufferEnd++ = now;
			now>>=8;
			*writeBufferEnd++ = now;
			now>>=8;
			*writeBufferEnd++ = now;
			now>>=8;
			*writeBufferEnd++ = now;
			// ra:
			*writeBufferEnd++ = ra_int;
			ra_int>>=8;
			*writeBufferEnd++ = ra_int;
			ra_int>>=8;
			*writeBufferEnd++ = ra_int;
			ra_int>>=8;
			*writeBufferEnd++ = ra_int;
			// dec:
			*writeBufferEnd++ = dec_int;
			dec_int>>=8;
			*writeBufferEnd++ = dec_int;
			dec_int>>=8;
			*writeBufferEnd++ = dec_int;
			dec_int>>=8;
			*writeBufferEnd++ = dec_int;
		}
		else
		{
			qDebug() << "TelescopeTcp(" << name << ")::telescopeGoto: "<< "communication is too slow, I will ignore this command";
		}
	}
}

void TelescopeTcp::performWriting(void)
{
	const int to_write = writeBufferEnd - writeBuffer;
	const int rc = tcpSocket->write(writeBuffer, to_write);
	if (rc < 0)
	{
		//TODO: Better error message. See the Qt documentation.
		qDebug() << "TelescopeTcp(" << name << ")::performWriting: "
			<< "write failed: " << tcpSocket->errorString();
		hangup();
	}
	else if (rc > 0)
	{
		if (rc >= to_write)
		{
			// everything written
			writeBufferEnd = writeBuffer;
		}
		else
		{
			// partly written
			memmove(writeBuffer, writeBuffer + rc, to_write - rc);
			writeBufferEnd -= rc;
		}
	}
}

//! try to read some data from the telescope server
void TelescopeTcp::performReading(void)
{
	const int to_read = readBuffer + sizeof(readBuffer) - readBufferEnd;
	const int rc = tcpSocket->read(readBufferEnd, to_read);
	if (rc < 0)
	{
		//TODO: Better error warning. See the Qt documentation.
		qDebug() << "TelescopeTcp(" << name << ")::performReading: " << "read failed: " << tcpSocket->errorString();
		hangup();
	}
	else if (rc == 0)
	{
		qDebug() << "TelescopeTcp(" << name << ")::performReading: " << "server has closed the connection";
		hangup();
	}
	else
	{
		readBufferEnd += rc;
		char *p = readBuffer;
		// parse the data in the read buffer:
		while (readBufferEnd - p >= 2)
		{
			const int size = (int)(((unsigned char)(p[0])) | (((unsigned int)(unsigned char)(p[1])) << 8));
			if (size > (int)sizeof(readBuffer) || size < 4)
			{
				qDebug() << "TelescopeTcp(" << name << ")::performReading: " << "bad packet size: " << size;
				hangup();
				return;
			}
			if (size > readBufferEnd - p)
			{
				// wait for complete packet
				break;
			}
			const int type = (int)(((unsigned char)(p[2])) | (((unsigned int)(unsigned char)(p[3])) << 8));
			// dispatch:
			switch (type)
			{
				case 0:
				{
				// We have received position information.
				// For the data format of the message see the
				// "Stellarium telescope control protocol"
					if (size < 24)
					{
						qDebug() << "TelescopeTcp(" << name << ")::performReading: " << "type 0: bad packet size: " << size;
						hangup();
						return;
					}
					const qint64 server_micros = (qint64)
						(((quint64)(unsigned char)(p[ 4])) |
						(((quint64)(unsigned char)(p[ 5])) <<  8) |
						(((quint64)(unsigned char)(p[ 6])) << 16) |
						(((quint64)(unsigned char)(p[ 7])) << 24) |
						(((quint64)(unsigned char)(p[ 8])) << 32) |
						(((quint64)(unsigned char)(p[ 9])) << 40) |
						(((quint64)(unsigned char)(p[10])) << 48) |
						(((quint64)(unsigned char)(p[11])) << 56));
					const unsigned int ra_int =
						((unsigned int)(unsigned char)(p[12])) |
						(((unsigned int)(unsigned char)(p[13])) <<  8) |
						(((unsigned int)(unsigned char)(p[14])) << 16) |
						(((unsigned int)(unsigned char)(p[15])) << 24);
					const int dec_int =
						(int)(((unsigned int)(unsigned char)(p[16])) |
							 (((unsigned int)(unsigned char)(p[17])) <<  8) |
							 (((unsigned int)(unsigned char)(p[18])) << 16) |
							 (((unsigned int)(unsigned char)(p[19])) << 24));
					const int status =
						(int)(((unsigned int)(unsigned char)(p[20])) |
							 (((unsigned int)(unsigned char)(p[21])) <<  8) |
							 (((unsigned int)(unsigned char)(p[22])) << 16) |
							 (((unsigned int)(unsigned char)(p[23])) << 24));

					// remember the time and received position so that later we
					// will know where the telescope is pointing to:
					position_pointer++;
					if (position_pointer >= end_position)
						position_pointer = positions;
					position_pointer->server_micros = server_micros;
					position_pointer->client_micros = getNow();
					const double ra  =  ra_int * (M_PI/(unsigned int)0x80000000);
					const double dec = dec_int * (M_PI/(unsigned int)0x80000000);
					const double cdec = cos(dec);
					position_pointer->pos[0] = cos(ra)*cdec;
					position_pointer->pos[1] = sin(ra)*cdec;
					position_pointer->pos[2] = sin(dec);
					position_pointer->status = status;
				}
				break;
				default:
					qDebug() << "TelescopeTcp(" << name << ")::performReading: " << "ignoring unknown packet, type: " << type;
				break;
			}
			p += size;
		}
		if (p >= readBufferEnd)
		{
			// everything handled
			readBufferEnd = readBuffer;
		}
		else
		{
			// partly handled
			memmove(readBuffer, p, readBufferEnd - p);
			readBufferEnd -= (p - readBuffer);
		}
	}
}

//! estimates where the telescope is by interpolation in the stored
//! telescope positions:
Vec3d TelescopeTcp::getJ2000EquatorialPos(const StelNavigator*) const
{
	if (position_pointer->client_micros == 0x7FFFFFFFFFFFFFFFLL)
	{
		return Vec3d(0,0,0);
	}
	const qint64 now = getNow() - time_delay;
	const Position *p = position_pointer;
	do
	{
		const Position *pp = p;
		if (pp == positions) pp = end_position;
		pp--;
		if (pp->client_micros == 0x7FFFFFFFFFFFFFFFLL) break;
		if (pp->client_micros <= now && now <= p->client_micros)
		{
			if (pp->client_micros != p->client_micros)
			{
				Vec3d rval = p->pos * (now - pp->client_micros) + pp->pos * (p->client_micros - now);
				double f = rval.lengthSquared();
				if (f > 0.0)
				{
					return (1.0/sqrt(f))*rval;
				}
			}
			break;
		}
		p = pp;
	}
	while (p != position_pointer);//BM: WTF?
	return p->pos;
}

//! checks if the socket is connected, tries to connect if it is not
//@return true if the socket is connected
bool TelescopeTcp::prepareCommunication()
{
	if(tcpSocket->state() == QAbstractSocket::ConnectedState)
	{
		if(wait_for_connection_establishment)
		{
			wait_for_connection_establishment = false;
			qDebug() << "TelescopeTcp(" << name << ")::prepareCommunication: " << "connection established";
		}
		return true;
	}
	else if(wait_for_connection_establishment)
	{
		const qint64 now = getNow();
		if (now > end_of_timeout)
		{
			end_of_timeout = now + 1000000;
			qDebug() << "TelescopeTcp(" << name << ")::prepareCommunication: " << "connect timeout";
			hangup();
		}
	}
	else
	{
		const qint64 now = getNow();
		if (now < end_of_timeout)
			return false; //Don't try to reconnect for some time
		end_of_timeout = now + 5000000;
		tcpSocket->connectToHost(address, port);
		wait_for_connection_establishment = true;
	}
	return false;
}

void TelescopeTcp::performCommunication()
{
	if (tcpSocket->state() == QAbstractSocket::ConnectedState)
	{
		performWriting();

		if (tcpSocket->bytesAvailable() > 0)
		{
			//If performReading() is called when there are no bytes to read,
			//it closes the connection
			performReading();
		}
	}
}

//TODO: More informative error messages?
void TelescopeTcp::socketFailed(QAbstractSocket::SocketError socketError)
{
	qDebug() << "TelescopeTcp(" << name << "): TCP socket error:\n" << tcpSocket->errorString();
}

} //namespace Stel
