#include "lg_tv_controller.h"

#include <QByteArray>
#include <QtPlugin>

#include <qlogger.h>

#include <serialportiofabric.h>

LGTVController::LGTVController() :
    portIO(NULL)
{
	QLogger(QLogger::INFO_SYSTEM, QLogger::LEVEL_INFO) <<
			"LG TV Controller loaded";
    
    portIO = SerialPortIOFabric::getInstance();
    Q_ASSERT(portIO != NULL);
    portIO->setSpeed(9600);
    portIO->setParity(ISerialPortIO::PAR_NONE);
    portIO->setFlowControl(ISerialPortIO::FLOW_OFF);

    tvSerialPort = settings.value("control/lg_tv/serial", "/dev/ttyS0").toString();

}

LGTVController::~LGTVController() {
    if (portIO != NULL) {
        portIO->close();
        delete portIO;
    }
}

QStringList LGTVController::getIDList() const {
    QStringList IDs;

    IDs.append("lg_tv_controller");
    IDs.append("screen_control");

    return IDs;
}

QString LGTVController::description() const {
	return "LG TV controller";
}

bool LGTVController::executeCommand(const QString &cmdID, const QString &argument) {
    QVariant parameter(argument);

    if (cmdID == "setPower") {
        if (open(tvSerialPort) != E_OK) {
            return false;
        }

        setPower(parameter.toBool());

        close();
    }

    if (cmdID == "remoteLock") {
        if (open(tvSerialPort) != E_OK) {
            return false;
        }

        setRemoteControlLock(parameter.toBool());

        close();
    }

    if (cmdID == "setVolumeLevel") {
        if (open(tvSerialPort) != E_OK) {
            return false;
        }

        setParamLevel(LEVEL_VOLUME, parameter.toUInt());

        close();
    }

    if (cmdID == "setBacklightLevel") {
        if (open(tvSerialPort) != E_OK) {
            return false;
        }

        setParamLevel(LEVEL_BACKLIGHT, parameter.toUInt());

        close();
    }


    return false;
}

quint8 LGTVController::open(const QString &device) {
    QLogger(QLogger::INFO_DEVICE, QLogger::LEVEL_TRACE) << "Попытка открыть порт управления телевизором LG:" << device;

    int res = portIO->open(device.toLocal8Bit().data());
    portIO->setInterByteTimeout(INTERBYTE_READ_TIMEOUT);

    if (res >= 0) {
        portIO->setSettings();
        QLogger(QLogger::INFO_DEVICE, QLogger::LEVEL_INFO) << "Порт управления телевизором LG" << device << "успешно открыт.";
        return E_OK;
    }

    return E_PORT_OPEN_ERROR;
}


quint8 LGTVController::setPower(bool isOn, COMMAND_TYPE cmdType, quint8 tvID) {
    tvID = (cmdType == COMMAND_BROADCAST) ? 0 : tvID;

    if (isOn) {
        QLogger(QLogger::INFO_DEVICE, QLogger::LEVEL_INFO) << "Включаем TV с ID:" << tvID;
        return sendCmd(tvID, 'k', 'a', "01");
    } else {
        QLogger(QLogger::INFO_DEVICE, QLogger::LEVEL_INFO) << "Выключаем TV с ID:" << tvID;
        return sendCmd(tvID, 'k', 'a', "00");
    }
}

quint8 LGTVController::setSignalSource(SIGNAL_SOURCE signalSource, COMMAND_TYPE cmdType, quint8 tvID) {
    QByteArray signalSourceCode;

    tvID = (cmdType == COMMAND_BROADCAST) ? 0 : tvID;

    switch (signalSource) {
    case INPUT_DVB_ANTENNA:
        signalSourceCode = "00";
        break;
    case INPUT_DVB_CABLE:
        signalSourceCode = "01";
        break;
    case INPUT_ANALOG_ANTENNA:
        signalSourceCode = "10";
        break;
    case INPUT_ANALOG_CABLE:
        signalSourceCode = "11";
        break;
    case INPUT_AV1:
        signalSourceCode = "20";
        break;
    case INPUT_AV2:
        signalSourceCode = "21";
        break;
    case INPUT_COMPONENT1:
        signalSourceCode = "40";
        break;
    case INPUT_COMPONENT2:
        signalSourceCode = "41";
        break;
    case INPUT_RGB_PC:
        signalSourceCode = "60";
        break;
    case INPUT_HDMI1:
        signalSourceCode = "90";
        break;
    case INPUT_HDMI2:
        signalSourceCode = "91";
        break;
    case INPUT_HDMI3:
        signalSourceCode = "92";
        break;
    case INPUT_HDMI4:
        signalSourceCode = "93";
        break;
    }

    QLogger(QLogger::INFO_DEVICE, QLogger::LEVEL_INFO) << "Устанавливаем входной сигнал для TV с ID:" << tvID << QString(signalSourceCode);
    return sendCmd(tvID, 'x', 'b', signalSourceCode);
}

quint8 LGTVController::setAspectRatio(ASPECT_RATIO aspectRatio, COMMAND_TYPE cmdType, quint8 tvID) {
    QByteArray aspectRatioCode;

    tvID = (cmdType == COMMAND_BROADCAST) ? 0 : tvID;

    switch (aspectRatio) {
    case ASPECT_4_3:
        aspectRatioCode = "01";
        break;
    case ASPECT_16_9:
        aspectRatioCode = "02";
        break;
    case ASPECT_AUTO:
        aspectRatioCode = "09";
        break;
    }

    QLogger(QLogger::INFO_DEVICE, QLogger::LEVEL_INFO) << "Устанавливаем соотношение сторон для TV с ID:" << tvID << QString(aspectRatioCode);
    return sendCmd(tvID, 'k', 'c', aspectRatioCode);
}

quint8 LGTVController::setScreenMute(bool isMuted, COMMAND_TYPE cmdType, quint8 tvID) {
    tvID = (cmdType == COMMAND_BROADCAST) ? 0 : tvID;

    if (!isMuted) {
        QLogger(QLogger::INFO_DEVICE, QLogger::LEVEL_INFO) << "Включаем экран TV с ID:" << tvID;
        return sendCmd(tvID, 'k', 'd', "01");
    } else {
        QLogger(QLogger::INFO_DEVICE, QLogger::LEVEL_INFO) << "Выключаем экран TV с ID:" << tvID;
        return sendCmd(tvID, 'k', 'd', "00");
    }
}

quint8 LGTVController::setVolumeMute(bool isMuted, COMMAND_TYPE cmdType, quint8 tvID) {
    tvID = (cmdType == COMMAND_BROADCAST) ? 0 : tvID;

    if (!isMuted) {
        QLogger(QLogger::INFO_DEVICE, QLogger::LEVEL_INFO) << "Включаем звук TV с ID:" << tvID;
        return sendCmd(tvID, 'k', 'e', "01");
    } else {
        QLogger(QLogger::INFO_DEVICE, QLogger::LEVEL_INFO) << "Выключаем звук TV с ID:" << tvID;
        return sendCmd(tvID, 'k', 'e', "00");
    }
}

quint8 LGTVController::setParamLevel(LEVEL_TYPE levelType, quint8 level, COMMAND_TYPE cmdType, quint8 tvID) {
    QString levelStr = QString::number(level >> 2, 16);

    if (levelStr.size() < 2)
        levelStr.prepend('0');

    tvID = (cmdType == COMMAND_BROADCAST) ? 0 : tvID;

    QLogger(QLogger::INFO_DEVICE, QLogger::LEVEL_INFO) << "Устанавливаем параметр для TV с ID:" << tvID << level << levelType;

    switch (levelType) {
    case LEVEL_VOLUME:
        return sendCmd(tvID, 'k', 'f', levelStr.toAscii());
    case LEVEL_CONTRAST:
        return sendCmd(tvID, 'k', 'g', levelStr.toAscii());
    case LEVEL_BRIGHTNESS:
        return sendCmd(tvID, 'k', 'h', levelStr.toAscii());
    case LEVEL_COLOR:
        return sendCmd(tvID, 'k', 'i', levelStr.toAscii());
    case LEVEL_TINT:
        return sendCmd(tvID, 'k', 'j', levelStr.toAscii());
    case LEVEL_SHARPNESS:
        return sendCmd(tvID, 'k', 'k', levelStr.toAscii());
    case LEVEL_TREBLE:
        return sendCmd(tvID, 'k', 'r', levelStr.toAscii());
    case LEVEL_BASS:
        return sendCmd(tvID, 'k', 's', levelStr.toAscii());
    case LEVEL_BALANCE:
        return sendCmd(tvID, 'k', 't', levelStr.toAscii());
    case LEVEL_COLOR_TEMP:
        return sendCmd(tvID, 'k', 'u', levelStr.toAscii());
    case LEVEL_BACKLIGHT:
        return sendCmd(tvID, 'm', 'g', levelStr.toAscii());
    }

    return E_PARAM_ERROR;
}

quint8 LGTVController::setOSD(bool isEnabled, COMMAND_TYPE cmdType, quint8 tvID) {
    tvID = (cmdType == COMMAND_BROADCAST) ? 0 : tvID;

    if (isEnabled) {
        QLogger(QLogger::INFO_DEVICE, QLogger::LEVEL_INFO) << "Включаем OSD TV с ID:" << tvID;
        return sendCmd(tvID, 'k', 'l', "01");
    } else {
        QLogger(QLogger::INFO_DEVICE, QLogger::LEVEL_INFO) << "Выключаем OSD TV с ID:" << tvID;
        return sendCmd(tvID, 'k', 'l', "00");
    }
}

quint8 LGTVController::setRemoteControlLock(quint8 isLocked, COMMAND_TYPE cmdType, quint8 tvID) {
    tvID = (cmdType == COMMAND_BROADCAST) ? 0 : tvID;

    if (isLocked) {
        QLogger(QLogger::INFO_DEVICE, QLogger::LEVEL_INFO) << "Включаем блокировку пульта TV с ID:" << tvID;
        return sendCmd(tvID, 'k', 'm', "01");
    } else {
        QLogger(QLogger::INFO_DEVICE, QLogger::LEVEL_INFO) << "Выключаем блокировку пульта TV с ID:" << tvID;
        return sendCmd(tvID, 'k', 'm', "00");
    }
}

quint8 LGTVController::makeAutoConfig(COMMAND_TYPE cmdType, quint8 tvID) {
    tvID = (cmdType == COMMAND_BROADCAST) ? 0 : tvID;

    QLogger(QLogger::INFO_DEVICE, QLogger::LEVEL_INFO) << "Выполняем автонастройку TV с ID:" << tvID;
    return sendCmd(tvID, 'j', 'u', "00");
}

quint8 LGTVController::setRemoteKeyCode(quint8 code, COMMAND_TYPE cmdType, quint8 tvID) {
    QString codeStr = QString::number(code, 16);

    if (codeStr.size() < 2)
        codeStr.prepend('0');

    tvID = (cmdType == COMMAND_BROADCAST) ? 0 : tvID;

    QLogger(QLogger::INFO_DEVICE, QLogger::LEVEL_INFO) << "Отправляем код для TV с ID:" << tvID << codeStr;

    return sendCmd(tvID, 'm', 'c', codeStr.toAscii());
}

quint8 LGTVController::sendCmd(quint8 id, char cmd1, char cmd2, const QByteArray &data) {
    if (portIO == NULL || !portIO->isOpen()) {
        QLogger(QLogger::INFO_DEVICE, QLogger::LEVEL_ERROR) << "Попытка отправки команды, однако порт не открыт";
        return E_PORT_NOT_OPEN;
    }

    QByteArray writeData;
    writeData.append(cmd1);
    writeData.append(cmd2);
    writeData.append(0x20);
    writeData.append(QString::number(id, 16).toAscii());
    writeData.append(0x20);
    writeData.append(data);
    writeData.append(0x0D);

    QLogger(QLogger::INFO_DEVICE, QLogger::LEVEL_TRACE) << "Записываем команду для TV:" << QString(writeData).replace('\r',' ') << "(" << QString(writeData.toHex()) << ")";

    ssize_t writedBytes = portIO->write((uint8_t *)writeData.data(), writeData.size());

    portIO->drain();

    if (writedBytes != writeData.size()) {
        QLogger(QLogger::INFO_DEVICE, QLogger::LEVEL_ERROR) << "Ошибка записи команды для TV. Записано" << writedBytes <<
                                                               "байт из" << writeData.size();
        return E_PORT_WRITE_ERROR;
    }

    return E_OK;
}

Q_EXPORT_PLUGIN2(lg_tv_controller, LGTVController)
