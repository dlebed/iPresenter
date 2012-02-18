#include "unixserialportio.h"

extern "C" {
#include "assert.h"
#include <sys/time.h>
#include <sys/select.h>
#include <linux/serial.h>
#include <sys/ioctl.h>
#include <string.h>
};

#ifdef PORT_MEASURE_TIMINGS
#define TV_MULTIPLER 1000000
#define TV_USEC_DIFF(start, end) ((end.tv_sec - start.tv_sec) * TV_MULTIPLER + (end.tv_usec - start.tv_usec))
#endif

std::list<int> UnixSerialPortIO::validFDs;

pthread_mutex_t UnixSerialPortIO::fdMutex;

uint32_t UnixSerialPortIO::instanceCount;

UnixSerialPortIO::UnixSerialPortIO(uint32_t flags):
	fd(-1), last_errno(0), portFlags(flags), speed(B9600), DataBits(DATA_8), StopBits(STOP_1),
	Parity(PAR_NONE), FlowControl(FLOW_OFF), interbyteTimeout(5000), readTimeout(25000) {
    
    if (instanceCount++ == 0) 
        pthread_mutex_init(&fdMutex, NULL);
    
	bzero(&newtio, sizeof(newtio));
    
#ifdef LOG_PACKETS
	init_logger("packet.log");
#endif
    
}

UnixSerialPortIO::UnixSerialPortIO(const char *pathname, uint32_t flags): portFlags(flags), speed(B9600),
	DataBits(DATA_8), StopBits(STOP_1), Parity(PAR_NONE), FlowControl(FLOW_OFF),
	interbyteTimeout(5000), readTimeout(25000) {
    
    if (--instanceCount == 0)
        pthread_mutex_destroy(&fdMutex);
    
	bzero(&newtio, sizeof(newtio));
    
#ifdef LOG_PACKETS
	init_logger("packet.log");
#endif
    
	open(pathname);
}

int UnixSerialPortIO::open(const char *pathname) {
	if (fd < 0) {
		fd = ::open(pathname, O_RDWR | O_NOCTTY);
        
        if (fd < 0) {
            last_errno = errno;
            return fd;
        }
        
		tcgetattr(fd, &oldtio);
		tcflush(fd, TCIOFLUSH);
		// Set default settings
		setSettings();
        pthread_mutex_lock(&fdMutex);
		validFDs.push_back(fd);
        pthread_mutex_unlock(&fdMutex);

        // Set low latency flag
        if ((portFlags & ISerialPortIO::LOW_LATENCY) != 0) {
            struct serial_struct serial;
            if (ioctl(fd, TIOCGSERIAL, &serial) != -1) {
                serial.flags |= ASYNC_LOW_LATENCY;
                if (ioctl(fd, TIOCSSERIAL, &serial) == -1) {
                    std::cerr << "SerialPortIO " << pathname << " : error setting ioctl ASYNC_LOW_LATENCY flag" << std::endl;
                }
            } else {
                std::cerr << "SerialPortIO " << pathname << " : error getting ioctl TIOCGSERIAL settings" << std::endl;
            }
        }
	}

	return fd;
}

int UnixSerialPortIO::close() {
	int ret;

	tcsetattr(fd, TCSANOW, &oldtio);
	ret =::close(fd);
    
	// Удаляем из списка дескрипторов
    pthread_mutex_lock(&fdMutex);
	validFDs.remove(fd);
	pthread_mutex_unlock(&fdMutex);
    
	fd = -1;

	return ret;
}

UnixSerialPortIO::~UnixSerialPortIO() {
	if (fd >= 0) {
		tcsetattr(fd, TCSANOW, &oldtio);
		::close(fd);
	}

#ifdef LOG_PACKETS
	close_logger();
#endif
}

int UnixSerialPortIO::setReadTimeout(uint32_t timeout) {
	readTimeout = timeout;
    return 0;
}

int UnixSerialPortIO::setReadCount(uint32_t count) {
	if (unlikely(fd < 0))
		return -1;

	newtio.c_cc[VMIN] = count;

	return tcsetattr(fd, TCSANOW, &newtio);
}

int UnixSerialPortIO::setInterByteTimeout(uint32_t timeout) {
    interbyteTimeout = timeout;
    return 0;
}

int UnixSerialPortIO::setSettings() {
	if (unlikely(fd < 0))
		return -1;

	newtio.c_cflag = speed | CLOCAL | CREAD;

	switch (DataBits) {
		case DATA_5:
			newtio.c_cflag |= CS5;
			break;
		case DATA_6:
			newtio.c_cflag |= CS6;
			break;
		case DATA_7:
			newtio.c_cflag |= CS7;
			break;
		case DATA_8:
			newtio.c_cflag |= CS8;
			break;
		default:
			newtio.c_cflag |= CS8;
			break;
	}

	if (StopBits == STOP_2)
		newtio.c_cflag |= CSTOPB;

	switch (Parity) {
		case PAR_NONE:
			newtio.c_iflag = IGNPAR;
			break;
		case PAR_ODD:
			newtio.c_cflag |= PARODD | PARENB | INPCK;
			break;
		case PAR_EVEN:
			newtio.c_cflag |= PARENB | INPCK;
			// Action not needed
			break;

		default:
			newtio.c_iflag = IGNPAR;
			break;
	}

	switch (FlowControl) {
		case FLOW_OFF:
			// Action not needed
			break;
		case FLOW_HARDWARE:
			newtio.c_cflag |= CRTSCTS;
			break;
		case FLOW_XONXOFF:
			newtio.c_iflag = IXON | IXOFF;
			break;

		default:
			// Action not needed
			break;
	}

	tcflush(fd, TCIFLUSH);
	return tcsetattr(fd, TCSANOW, &newtio);
}

int UnixSerialPortIO::flush(FlushType type) {
	if (unlikely(fd < 0))
		return -1;

	return ::tcflush(fd, convertFlushType(type));
}

int UnixSerialPortIO::drain() {
	if (unlikely(fd < 0))
		return -1;

	return ::tcdrain(fd);
}

void UnixSerialPortIO::closeAllFDs() {
    pthread_mutex_lock(&fdMutex);
	for (std::list < int >::iterator it = validFDs.begin(), end = validFDs.end(); it != end; ++it) {
		::close(*it);
        validFDs.erase(it);
    }
    pthread_mutex_unlock(&fdMutex);
}

ssize_t UnixSerialPortIO::write(uint8_t *data, size_t size) {
	if (unlikely(fd < 0))
		return -1;

#ifdef LOG_PACKETS
	log_packet(data, true, size);
#endif

	return::write(fd, data, size);
}

ssize_t UnixSerialPortIO::read(uint8_t *data, size_t size) {
	int res = read_select(data, size, readTimeout);

#ifdef LOG_PACKETS
	if (res > 0)
		log_packet(data, false, res);
#endif

	return res;
}

// Read with timeout in microseconds
ssize_t UnixSerialPortIO::read(uint8_t *data, size_t size, uint32_t timeout) {
	if (unlikely(fd < 0))
		return -1;

	size_t count = 0;
	ssize_t readBytes;
    
#ifdef PORT_MEASURE_TIMINGS
    struct timeval tv_start, tv_end;
    gettimeofday(&tv_start, NULL);
#endif
    
	// Reading avalible bytes
	readBytes = read_select(data, size, timeout);

#ifdef PORT_MEASURE_TIMINGS
    gettimeofday(&tv_end, NULL);
#endif
    
	if (unlikely(readBytes <= 0)) {

#ifdef PORT_LOG_TIMEOUTS
        std::cerr << std::endl << "SerialPortIO: first read() ended without data. Requested " << size << " bytes" <<
#ifdef PORT_MEASURE_TIMINGS
                     "; time offset: " << TV_USEC_DIFF(tv_start, tv_end) << " usec" <<
#endif
                     std::endl;
#endif

		return 0;
    }

	count += readBytes;

#ifdef PORT_LOG_TIMEOUTS
	if (count < size) {
        std::cerr << std::endl << "SerialPortIO: first read() ended. Read " << count << " bytes of " << size << 
#ifdef PORT_MEASURE_TIMINGS
                     "; time offset: " << TV_USEC_DIFF(tv_start, tv_end) << " usec" <<
#endif
                     std::endl;
#endif
        
		// Reading remaining bytes
		while (count < size) {
			readBytes = read_select(data + count, size - count, interbyteTimeout);

#ifdef PORT_MEASURE_TIMINGS
            gettimeofday(&tv_end, NULL);
#endif
            
#ifdef PORT_LOG_TIMEOUTS
                std::cerr << "SerialPortIO: interbyte read() ended. Read " << readBytes << " bytes, total read " << count + readBytes <<
#ifdef PORT_MEASURE_TIMINGS
                     "; time offset: " << TV_USEC_DIFF(tv_start, tv_end) << " usec" <<
#endif
                     std::endl;
#endif
            
			if (readBytes <= 0) {
                break;
			}

			count += readBytes;
		}
        
#ifdef PORT_LOG_TIMEOUTS
	}
#endif

#ifdef PORT_MEASURE_TIMINGS
    gettimeofday(&tv_end, NULL);
#endif
    
#ifdef LOG_PACKETS

	if (count > 0)
		log_packet(data, false, count);

#endif

#ifdef PORT_LOG_TIMEOUTS
    std::cerr << "SerialPortIO: full read() ended. Read " << count << " bytes of " << size <<
#ifdef PORT_MEASURE_TIMINGS
                 "; time offset: " << TV_USEC_DIFF(tv_start, tv_end) << " usec" <<
#endif
                 std::endl << std::endl;
#endif
    
	return count;
}

// timeout - таймаут в микросекундах
ssize_t UnixSerialPortIO::read_select(uint8_t *data, size_t size, uint32_t timeout) {
	int n;

	// если таймаут не задан
	if (unlikely(timeout == 0))
		return::read(fd, data, size);

	FD_ZERO(&input);
	FD_SET(fd, &input);

	timeout_struct.tv_sec = timeout / 1000000;
	timeout_struct.tv_usec = timeout % 1000000;

	n = select(fd + 1, &input, NULL, NULL, &timeout_struct);

	if (likely(n >= 0)) {
		int res = ::read(fd, data, size);
		return res;
	} else if (n < 0) {
		std::cerr << "UnixSerialPortIO: select() failed: " << n << std::endl;
		return n;
	}
    
    return 0;
}


int UnixSerialPortIO::convertSpeed(speed_t &tc_speed, uint32_t speed) {
    switch (speed) {
    case 150:
        tc_speed = B150;
        break;
    case 200:
        tc_speed = B200;
        break;
    case 300:
        tc_speed = B300;
        break;
    case 600:
        tc_speed = B600;
        break;
    case 1200:
        tc_speed = B1200;
        break;
    case 1800:
        tc_speed = B1800;
        break;
    case 2400:
        tc_speed = B2400;
        break;
    case 4800:
        tc_speed = B4800;
        break;
    case 9600:
        tc_speed = B9600;
        break;
    case 19200:
        tc_speed = B19200;
        break;
    case 38400:
        tc_speed = B38400;
        break;
    case 57600:
        tc_speed = B57600;
        break;
    case 115200:
        tc_speed = B115200;
        break;
    case 230400:
        tc_speed = B230400;
        break;
    default:
        return -1;
    }

    return 0;
}

UnixSerialPortIO::DataBitsType UnixSerialPortIO::convertDataBits(uint8_t dataBits) {
    switch (dataBits) {
    case 5:
        return DATA_5;
    case 6:
        return DATA_6;
    case 7:
        return DATA_7;
    case 8:
    default:
        return DATA_8;
    }
}

UnixSerialPortIO::StopBitsType UnixSerialPortIO::convertStopBits(uint8_t stopBits) {
    switch (stopBits) {
    case 2:
        return STOP_2;
    case 1:
    default:
        return STOP_1;
    }
}


int UnixSerialPortIO::convertFlushType(FlushType type) {
    switch (type) {
    case FLUSH_INPUT:
        return TCIFLUSH;
    case FLUSH_OUTPUT:
        return TCOFLUSH;
    case FLUSH_INPUT_OUTPUT:
        return TCIOFLUSH;
    default:
        return -1;
    }
}
