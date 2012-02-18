#include "winserialportio.h"

extern "C" {
#include "assert.h"
#include <sys/time.h>
#include <stdint.h>
};

std::list<HANDLE> WinSerialPortIO::validFDs;

WinSerialPortIO::WinSerialPortIO(uint32_t flags) :
        hDevice(INVALID_HANDLE_VALUE), portFlags(flags)
{
    init_dcb();

    setSpeed(9600);
    setDataBits(DATA_8);
    setStopBits(STOP_1);
    setParity(PAR_NONE);
    setFlowControl(FLOW_OFF);
    setInterByteTimeout(5000);
    setReadTimeout(200000);

#ifdef LOG_PACKETS
    init_logger("packet.log");
#endif
}

WinSerialPortIO::WinSerialPortIO(const char *pathname, uint32_t flags) :
        hDevice(INVALID_HANDLE_VALUE), portFlags(flags)
{
    init_dcb();

    setSpeed(9600);
    setDataBits(DATA_8);
    setStopBits(STOP_1);
    setParity(PAR_NONE);
    setFlowControl(FLOW_OFF);
    setInterByteTimeout(5000);
    setReadTimeout(200000);

#ifdef LOG_PACKETS
    init_logger("packet.log");
#endif

    open(pathname);
}

WinSerialPortIO::~WinSerialPortIO() {
    if (exist()) {
        CloseHandle(hDevice);
    }

#ifdef LOG_PACKETS
    close_logger();
#endif
}

int WinSerialPortIO::open(const char *pathname) {
    if (!exist()) {

        char intArray[128] = {0};
        strcpy(intArray, "\\\\.\\");
        strncat(intArray, pathname, strlen(pathname));

        hDevice = CreateFileA(intArray, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, NULL, NULL);

        if (hDevice == INVALID_HANDLE_VALUE || hDevice == NULL)
            return -1;

        // Set default settings
        setSettings();
        validFDs.push_back(hDevice);
    }

    return 0;
}

int WinSerialPortIO::close() {
    CloseHandle(hDevice);

    // Удаляем из списка дескрипторов
    validFDs.remove(hDevice);
    hDevice = INVALID_HANDLE_VALUE;

    return -1;
}

void WinSerialPortIO::setDataBits(DataBitsType DataBits) {
    switch(DataBits)
    {
    case DATA_5:
        dcb_struct.ByteSize = 5;
        break;
    case DATA_6:
        dcb_struct.ByteSize = 6;
        break;
    case DATA_7:
        dcb_struct.ByteSize = 7;
        break;
    case DATA_8:
        dcb_struct.ByteSize = 8;
        break;
    }
}

void WinSerialPortIO::setStopBits(StopBitsType StopBits) {
    switch(StopBits)
    {
    case STOP_1:
        dcb_struct.StopBits = ONESTOPBIT;
        break;
    case STOP_2:
        dcb_struct.StopBits = TWOSTOPBITS;
        break;
    }
}

void WinSerialPortIO::setParity(ParityType Parity) {
    dcb_struct.Parity = Parity;
    dcb_struct.fParity = Parity != PAR_NONE ? TRUE : FALSE;
}

void WinSerialPortIO::setFlowControl(FlowControlType FlowControl) {
    fControl = FlowControl;
    switch(FlowControl) {
    case FLOW_OFF:
        dcb_struct.fDtrControl = DTR_CONTROL_DISABLE;
        dcb_struct.fRtsControl = RTS_CONTROL_DISABLE;
        dcb_struct.fOutxCtsFlow = FALSE;
        dcb_struct.fOutxDsrFlow = FALSE;
        dcb_struct.fDsrSensitivity = FALSE;
        dcb_struct.fTXContinueOnXoff = FALSE;
        dcb_struct.fOutX = FALSE;
        dcb_struct.fInX = FALSE;
        dcb_struct.XoffLim = 0;
        dcb_struct.XonLim = 0;
        break;
    case FLOW_HARDWARE:
        dcb_struct.fDtrControl = DTR_CONTROL_HANDSHAKE;
        dcb_struct.fRtsControl = RTS_CONTROL_DISABLE;
        dcb_struct.fOutxCtsFlow = FALSE;





        dcb_struct.fOutxDsrFlow = FALSE;
        dcb_struct.fDsrSensitivity = FALSE;
        dcb_struct.fTXContinueOnXoff = FALSE;
        dcb_struct.fOutX = FALSE;
        dcb_struct.fInX = FALSE;


        /*dcb_struct.fDtrControl = DTR_CONTROL_HANDSHAKE;
        dcb_struct.fRtsControl = RTS_CONTROL_DISABLE;
        dcb_struct.fOutxCtsFlow = FALSE;
        dcb_struct.fOutxDsrFlow = TRUE;
        dcb_struct.fDsrSensitivity = FALSE;
        dcb_struct.fTXContinueOnXoff = FALSE;
        dcb_struct.fOutX = FALSE;
        dcb_struct.fInX = FALSE;*/


        break;
    case FLOW_XONXOFF:
        dcb_struct.fDtrControl = DTR_CONTROL_DISABLE;
        dcb_struct.fRtsControl = RTS_CONTROL_DISABLE;
        dcb_struct.fOutxCtsFlow = FALSE;
        dcb_struct.fOutxDsrFlow = TRUE;
        dcb_struct.fDsrSensitivity = FALSE;
        dcb_struct.fTXContinueOnXoff = TRUE;
        dcb_struct.fOutX = TRUE;
        dcb_struct.fInX = TRUE;
        break;
    }
}

int WinSerialPortIO::setReadTimeout(uint32_t timeout) {
    if(timeout == -2)
    {
        this->timeout.ReadTotalTimeoutConstant = -2;
        this->timeout.ReadTotalTimeoutMultiplier = -1;
    }
    else if(timeout == -1)
    {
        this->timeout.ReadTotalTimeoutConstant = -1;
        this->timeout.ReadTotalTimeoutMultiplier = -1;
    }
    else
    {
    this->timeout.ReadTotalTimeoutConstant = ROUND_VALUE(timeout);
        this->timeout.ReadTotalTimeoutMultiplier = -1;
    }

    return 0;
}

int WinSerialPortIO::setSettings() {
    if (!exist())
        return -1;

    DCB dcb_tmp;
    COMMTIMEOUTS ctm;

    if(!GetCommState(hDevice, &dcb_tmp))
        return -1;

    update_dcb(&dcb_tmp);

    if(!SetCommState(hDevice, &dcb_tmp))
        return -1;

    if(!GetCommTimeouts(hDevice, &ctm))
        return -1;

    updateTimeout(&ctm);

    if(!SetCommTimeouts(hDevice, &ctm))
        return -1;

    if(!SetupComm(hDevice, 4096, 2048))
        return -1;

    /*if(fControl == FLOW_HARDWARE)
    {
        DCB tmpDcb;
        GetCommState(hDevice, &tmpDcb);
        tmpDcb.fRtsControl = RTS_CONTROL_DISABLE;
        SetCommState(hDevice, &tmpDcb);
        SetCommMask(hDevice, EV_RXCHAR | EV_RXFLAG | EV_CTS | EV_DSR | EV_RLSD | EV_BREAK | EV_ERR | EV_RING);
    }*/

    return 0;
}

int WinSerialPortIO::flush(FlushType) {
    if (!exist())
        return -1;

    if (FlushFileBuffers(hDevice) == FALSE)
        return -1;

    return 0;
}

int WinSerialPortIO::drain() {
    // TODO Убрать заглушку
    return 0;
}

void WinSerialPortIO::closeAllFDs() {
    for (std::list<HANDLE>::const_iterator it = validFDs.begin(), end = validFDs.end(); it != end; ++it)
        CloseHandle(*it);
}

ssize_t WinSerialPortIO::write(uint8_t *data, size_t size) {
    if (!exist())
        return -1;

    /*if(fControl == FLOW_HARDWARE)
    {
        DWORD dwEvtMask, oldMask;

        //GetCommMask(hDevice, &oldMask);
        if (WaitCommEvent(hDevice, &dwEvtMask, NULL))
            {
                if (dwEvtMask &  oldMask)
                {
                     // To do.
                }
            }
    }*/

    DWORD written = 0;

#ifdef LOG_PACKETS
    log_packet(data, true, size);
#endif

    BOOL bres=WriteFile(hDevice, data, size, &written, NULL);

    if (written!=size&&written>0&&bres==FALSE)
        return written;
    if (written==0&&bres==FALSE)
        return -1;
    if (written==0&&bres==TRUE&&size!=0)
        return -1;
    return written;
}

ssize_t WinSerialPortIO::read(uint8_t *data, size_t size) {
    if (!exist())
        return -1;

    DWORD read = 0;

    ReadFile(hDevice, data, size, &read, NULL);

#ifdef LOG_PACKETS
    if (read > 0)
        log_packet(data, false, read);
#endif

    return read;
}

// Read with timeout in microseconds
ssize_t WinSerialPortIO::read(uint8_t *data, size_t size, uint32_t timeout) {
    if (!exist())
        return -1;

    size_t count = 0;
    COMMTIMEOUTS ctm;

    if (!GetCommTimeouts(hDevice, &ctm))
        return -1;

    if (this->timeout.ReadIntervalTimeout != ROUND_VALUE(timeout)) {
        setInterByteTimeout(timeout);
        updateTimeout(&ctm);

        if (!SetCommTimeouts(hDevice, &ctm))
            return -1;
    }

    // Reading avalible bytes
    DWORD read = 0;
    ReadFile(hDevice, data, size, &read, NULL);

    if (read <= 0)
        return 0;

    count += read;

    if (count < size) {
        //std::cerr << "PortIO: read() interbyte timeout: " << size << " > " << count << std::endl;

        // Reading remaining bytes
        while (count < size) {
            ReadFile(hDevice, data + count, size - count, &read, NULL);

            if (read <= 0) {
#ifdef LOG_PACKETS
                if (count > 0)
                    log_packet(data, false, count);
#endif
                return count;
            }

            count += read;
        }
    }

#ifdef LOG_PACKETS
    if (count > 0)
        log_packet(data, false, count);
#endif

    return count;
}

void WinSerialPortIO::init_dcb() {
    dcb_struct.DCBlength = sizeof(DCB);
    dcb_struct.fBinary = TRUE;
    dcb_struct.fErrorChar = dcb_struct.fParity == TRUE ? TRUE : FALSE;
    dcb_struct.fNull = FALSE;
    dcb_struct.fAbortOnError = FALSE;
    dcb_struct.wReserved = 0;
}

void WinSerialPortIO::update_dcb(DCB* _dcb)
{
    _dcb->BaudRate = dcb_struct.BaudRate;
    _dcb->ByteSize = dcb_struct.ByteSize;
    _dcb->fDsrSensitivity = dcb_struct.fDsrSensitivity;
    _dcb->fDtrControl = dcb_struct.fDtrControl;
    _dcb->fRtsControl = dcb_struct.fRtsControl;
    _dcb->fOutX = dcb_struct.fOutX;
    _dcb->fOutX = dcb_struct.fInX;
    _dcb->fOutxCtsFlow = dcb_struct.fOutxCtsFlow;
    _dcb->fOutxDsrFlow = dcb_struct.fOutxDsrFlow;
    _dcb->fTXContinueOnXoff = dcb_struct.fTXContinueOnXoff;
    _dcb->fParity = dcb_struct.fParity;
    _dcb->Parity = dcb_struct.Parity;
    _dcb->StopBits = dcb_struct.StopBits;
}

void WinSerialPortIO::updateTimeout(COMMTIMEOUTS* _timeout)
{
    _timeout->ReadIntervalTimeout = timeout.ReadIntervalTimeout >= 0 ? timeout.ReadIntervalTimeout : 100;
    _timeout->ReadTotalTimeoutConstant = timeout.ReadTotalTimeoutConstant >= 0 ? timeout.ReadTotalTimeoutConstant : MAXDWORD;
    _timeout->ReadTotalTimeoutMultiplier=timeout.ReadTotalTimeoutMultiplier >=0 ? timeout.ReadTotalTimeoutMultiplier : 100;
    _timeout->WriteTotalTimeoutConstant = timeout.WriteTotalTimeoutConstant >=0 ? timeout.WriteTotalTimeoutConstant : 200;
    _timeout->WriteTotalTimeoutMultiplier=timeout.WriteTotalTimeoutMultiplier >=0 ? timeout.WriteTotalTimeoutMultiplier : 0;
}
