#ifndef SERIALPORTIO_H
#define SERIALPORTIO_H

#include <iostream>
#include <list>
#include <ctime>
#include <cstdio>

extern "C" {
#include <termios.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <asm/types.h>
#include <string.h>
#include <stdint.h>
#include <pthread.h>
}

#include <defines.h>

#include <loggingserialportio.h>

//! Класс для работы с последовательным портом
/*!
  Обеспечивает работу с последовательным портом в ОС Linux.
  Имеет возможность установки межсимвольных таймаутов.
  \note Параметры устанавливаются при помощи методов setSpeed(), setDataBits(),
  setStopBits(), setParity(), setFlowControl().
  Применяются же они к устройству только после вызова метода setSettings().
  */

class UnixSerialPortIO : public LoggingSerialPortIO {
public:
	UnixSerialPortIO(uint32_t flags = 0);
	UnixSerialPortIO(const char *pathname, uint32_t flags = 0);
	~UnixSerialPortIO();

	int open(const char *pathname);

    //! Получить описание последней ошибки
    //char * lastErrorStr() const { return strerror(last_errno); }
    
	int close();

	bool isOpen() const {
		return fd >= 0;
	}

	int setSpeed(uint32_t speed) {
		return convertSpeed(this->speed, speed);
	}

	void setDataBits(DataBitsType DataBits) {
		this->DataBits = DataBits;
	}

	void setStopBits(StopBitsType StopBits) {
		this->StopBits = StopBits;
	}

	void setParity(ParityType Parity) {
		this->Parity = Parity;
	}

	void setFlowControl(FlowControlType FlowControl) {
		this->FlowControl = FlowControl;
	}

	int setReadTimeout(uint32_t timeout);
	int setReadCount(uint32_t count);
	int setInterByteTimeout(uint32_t timeout);

    void setWriteTimeout(uint32_t ) { }

	int setSettings();

	int flush(FlushType type = FLUSH_INPUT);
    int drain();

	void closeAllFDs();

	ssize_t write(uint8_t *data, size_t size);

	ssize_t read(uint8_t *data, size_t size);
	ssize_t read(uint8_t *data, size_t size, uint32_t timeout);

protected:
	//! Функция чтения из порта с произвольным значением таймаута
	/*!
	 * \param data указатель на буфер для принятых данных
	 * \param size размер ожидаемых данных в байтах
	 * \param timeout значение таймаута в микросекундах
	 */
	ssize_t read_select(uint8_t *data, size_t size, uint32_t timeout);

    //! Преобразование значения FlushType в unix-специфичное значение
    static int convertFlushType(FlushType type);
    
    //! Преобразование значения скорости в speed_t
    /*!
     * \param tc_speed переменная, в которую будет записано преобразованное значение
     * \param speed численное значение скорости для преобразовнаия
     * \return ноль - в случае успеха, значение меньше нуля - в случае невозможности преобразования
      */
    static int convertSpeed(speed_t &tc_speed, uint32_t speed);
    
    //! Преобразование количества бит данных во внутренний тип
    static DataBitsType convertDataBits(uint8_t dataBits);
    
    //! Преобразование количества стоп-бит во внутреннее представление
    static StopBitsType convertStopBits(uint8_t stopBits);
    
private:
	int fd, last_errno;
    uint32_t portFlags;
	speed_t speed;
	DataBitsType DataBits;
	StopBitsType StopBits;
	ParityType Parity;
	FlowControlType FlowControl;
	struct termios oldtio, newtio;
	uint32_t interbyteTimeout, readTimeout;
	// для read_select
	fd_set input;
	struct timeval timeout_struct;
    
    static pthread_mutex_t fdMutex;
    static uint32_t instanceCount;
	static std::list <int> validFDs;
};


#endif                          // UnixSerialPortIO_H
