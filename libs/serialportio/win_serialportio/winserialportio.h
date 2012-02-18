#ifndef WINSERIALPORTIO_H
#define WINSERIALPORTIO_H

#include <iostream>
#include <list>
#include <ctime>
#include <cstdio>
#include <windows.h>

extern "C" {
#include <string.h>
#include <stdint.h>
}

#include <defines.h>

#include <loggingserialportio.h>
#include <iserialportio.h>

typedef long int ssize_t;

//! Пересчитывает микросекунды в милисеукнды с округлением до ближайшего
#define ROUND_VALUE(val) (val+500)/1000

//! Класс для работы с последовательным портом
/*!
  Обеспечивает работу с последовательным портом в ОС Windows.
  Имеет возможность установки межсимвольных таймаутов.
  \note Параметры устанавливаются при помощи методов setSpeed(), setDataBits(),
  setStopBits(), setParity(), setFlowControl(), setReadTimeout(), setInterByteTimeout().
  Применяются же они к устройству только после вызова метода setSettings().
  */

class WinSerialPortIO : public LoggingSerialPortIO {
public:
        //! Конструктор без открытия порта
        WinSerialPortIO(uint32_t flags = 0);

        //! Конструктор с открытием порта
        WinSerialPortIO(const char *pathname, uint32_t flags = 0);

        ~WinSerialPortIO();

        //! Открытие устройства и применение настроек по умолчанию
        int open(const char *pathname);

        //! Закрытие устройстваs
        int close();

        //! Проверка, открыт ли порт
        bool isOpen() const {
            return (hDevice != INVALID_HANDLE_VALUE && hDevice != NULL);
        }

        //! Установка скороти порта
        int setSpeed(uint32_t speed) {
            return dcb_struct.BaudRate = speed;
        }

        //! Установка количества байт данных для порта
        void setDataBits(ISerialPortIO::DataBitsType DataBits);

        //! Установка количества стоп-бит для порта
        void setStopBits(ISerialPortIO::StopBitsType StopBits);

        //! Установка типа четности для порта
        void setParity(ISerialPortIO::ParityType Parity);

        //! Установка типа управления потоком
        void setFlowControl(ISerialPortIO::FlowControlType FlowControl);

        //! Установка значения таймаута для операции чтения
        /*!
         * \param timeout значение таймаута в микросекундах
         * \return результат выполнения операции, 0 в случае успеха, -1 в случае ошибки.
         */
        int setReadTimeout(uint32_t timeout);

        //! Установка значения мультпликативного таймаута для операции чтения
        /*!
        * \param timeout значение таймаута в микросекундах
        */
        void setReadMultipleTimeout(uint32_t _timeout){
            this->timeout.ReadTotalTimeoutMultiplier = ROUND_VALUE(_timeout);
        }

        //! Установка необходимого таймаута для операции записи
        /*!
        * \param count необходимое количество байт
        */
        void setWriteTimeout(uint32_t _timeout){
            this->timeout.WriteTotalTimeoutConstant = ROUND_VALUE(_timeout);
            this->timeout.WriteTotalTimeoutMultiplier=0;
        }

        //! Установка значения мультпликативного таймаута для операции записи
        /*!
        * \param timeout значение таймаута в микросекундах
        */
        void setWriteMultipleTimeout(uint32_t _timeout){
            this->timeout.WriteTotalTimeoutMultiplier = ROUND_VALUE(_timeout);
        }

        //! Установка значения межбайтного таймаута для операции чтения
        /*!
	 * \param timeout таймаут в миллисикундах
	 */
        int setInterByteTimeout(uint32_t _timeout) {
            if(_timeout == -1)
                return this->timeout.ReadIntervalTimeout = -1;
            else
            return this->timeout.ReadIntervalTimeout = ROUND_VALUE(_timeout);
        }

        //! Установка необходимого количества байт для чтения
        /*!
         * \param count необходимое количество байт
         */
        int setReadCount(uint32_t count) {
            data_count = count;
            return 0;
        }

        //! Применение настроек
        /*!
	 * \return 0 в случае успеха, < 0 в случае неудачи
	 */
        int setSettings();

        //! Удаляет незаписанные, или несчитанные данные из буфера порта
        /*!
	 * \param type TCIFLUSH - для очистки принятых, но не прочитанных данных,\n
	 * TCOFLUSH - для очистки записанных в буфер порта данных, но не переданных\n
	 * TCIOFLUSH - для очистки обеих буферов, приёма и передачи
	 */
        int flush(FlushType type = FLUSH_INPUT);

        //! Ожидане окончания передачи записанных данных
        int drain();
        
        //! Закрыть все открытиые при помощи данного класса файловые дескрипторы
        void closeAllFDs();

        //! Прочитать из порта
        /*!
	 * Функция записи в порта без работы с таймаутами
	 * \param data указатель на буфер с данными
	 * \param size размер записываемых данных в байтах
	 */
        ssize_t write(uint8_t *data, size_t size);

        //! Записать в порт
        /*!
	 * Функция чтения из порта без работы с таймаутами
	 * \param data указатель на буфер для принятых данных
	 * \param size размер ожидаемых данных в байтах
	 */
        ssize_t read(uint8_t *data, size_t size);

        //! Прочитать из порта с установкой таймаута
        /*!
	 * Функция чтения из порта с таймаутом.
	 * Параметр timeout задаёт максимальное время ожидания появления данных в приёмном буфере порта.
	 * Межбайтный таймаут для операции чтения задаётся при помощи функции setInterByteTimeout.
	 * \param data указатель на буфер для принятых данных
	 * \param size размер ожидаемых данных в байтах
	 * \param timeout значение нового таймаута начала передачи для операции чтения в микросекундах
	 * \return размер принятых данных в байтах
	 * \sa setInterByteTimeout read
	 */
        ssize_t read(uint8_t *data, size_t size, uint32_t timeout);

protected:
        void init_dcb();

        void update_dcb(DCB* _dcb);

        void updateTimeout(COMMTIMEOUTS* _timeout);

        bool exist() {
            return hDevice != INVALID_HANDLE_VALUE && hDevice != NULL ? true : false;
        }

private:
        HANDLE hDevice;
        DCB dcb_struct;
        COMMTIMEOUTS timeout;
        uint32_t portFlags;
        uint32_t data_count;
        static std::list <HANDLE> validFDs;
        FlowControlType fControl;
};


#endif                          // WINSERIALPORTIO_H
