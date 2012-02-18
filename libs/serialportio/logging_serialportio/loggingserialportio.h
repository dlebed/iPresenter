#ifndef LOGGINGSERIALPORTIO_H
#define LOGGINGSERIALPORTIO_H

#include <cstdio>

#include <iserialportio.h>

#define LOG_DATETIME_FORMAT "%H:%M:%S" // "%d.%m.%y %H:%M:%S"

class LoggingSerialPortIO : public ISerialPortIO {
public:
    LoggingSerialPortIO();
    
    virtual ~LoggingSerialPortIO();
    
protected:
    
	void init_logger(char *filename);

	void close_logger();

	void log_packet(uint8_t *data, bool dirOut, size_t size);
    
private:
    // Дескриптор файла для логгирования
    FILE *log_fd;

};

#endif // LOGGINGSERIALPORTIO_H
