#include "loggingserialportio.h"

extern "C" {
#include <sys/time.h>
#include <assert.h>
#include <time.h>
}

LoggingSerialPortIO::LoggingSerialPortIO() :
    log_fd(NULL)
{
}

LoggingSerialPortIO::~LoggingSerialPortIO() {
    close_logger();
}

void LoggingSerialPortIO::init_logger(char *filename) {
	log_fd = ::fopen(filename, "a");

	if (log_fd != NULL) {
		fprintf(log_fd, "packet logger init\n");
	}
}

void LoggingSerialPortIO::close_logger() {
	if (log_fd != NULL) {
		fprintf(log_fd, "packet logger close\n");
		::fclose(log_fd);
	}
}

void LoggingSerialPortIO::log_packet(uint8_t *data, bool dirOut, size_t size) {
	if (unlikely(log_fd != NULL)) {
		char timebuf[32];
		struct timeval tv;

		time_t t = time(NULL);
		gettimeofday(&tv, NULL);

		struct tm *tmp = localtime(&t);

		assert(tmp != NULL);

		strftime(timebuf, sizeof(timebuf), LOG_DATETIME_FORMAT, tmp);

		fprintf(log_fd, "%s:%.3u ", timebuf, (unsigned int)tv.tv_usec / 1000);

		if (dirOut)
			fprintf(log_fd, ">>> ");
		else
			fprintf(log_fd, "<<< ");

		for (size_t i = 0; i < size; i++)
			fprintf(log_fd, "%.2X ", data[i]);

		fprintf(log_fd, "\n");
	}
}
