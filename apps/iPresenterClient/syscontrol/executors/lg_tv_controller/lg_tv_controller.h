#ifndef LGTVCONTROLLER_H
#define LGTVCONTROLLER_H

#include <QObject>
#include <QSettings>

#include <icommandhandler.h>

#include <iserialportio.h>

#define INTERBYTE_READ_TIMEOUT   10000

class LGTVController : public ICommandHandler {
	Q_OBJECT
	Q_INTERFACES(ICommandHandler)

    enum TVCONTROLLER_ERRORS {
        E_OK                =   0x00,
        E_PORT_OPEN_ERROR   =   0x01,
        E_PORT_CLOSE_ERROR  =   0x02,
        E_PORT_NOT_OPEN     =   0x03,
        E_PORT_WRITE_ERROR  =   0x04,
        E_PARAM_ERROR       =   0x05
    };

    enum COMMAND_TYPE {
        COMMAND_BROADCAST       =   0x00,
        COMMAND_UNICAST         =   0x01
    };

    enum SIGNAL_SOURCE {
        INPUT_DVB_ANTENNA           =   0x00,
        INPUT_DVB_CABLE             =   0x01,
        INPUT_ANALOG_ANTENNA        =   0x02,
        INPUT_ANALOG_CABLE          =   0x03,
        INPUT_AV1                   =   0x04,
        INPUT_AV2                   =   0x05,
        INPUT_COMPONENT1            =   0x06,
        INPUT_COMPONENT2            =   0x07,
        INPUT_RGB_PC                =   0x08,
        INPUT_HDMI1                 =   0x09,
        INPUT_HDMI2                 =   0x0A,
        INPUT_HDMI3                 =   0x0B,
        INPUT_HDMI4                 =   0x0C
    };

    enum ASPECT_RATIO {
        ASPECT_4_3                  =   0x00,
        ASPECT_16_9                 =   0x01,
        ASPECT_AUTO                 =   0x02
    };

    enum LEVEL_TYPE {
        LEVEL_VOLUME                =   0x00,
        LEVEL_CONTRAST              =   0x01,
        LEVEL_BRIGHTNESS            =   0x02,
        LEVEL_COLOR                 =   0x03,
        LEVEL_TINT                  =   0x04,
        LEVEL_SHARPNESS             =   0x05,
        LEVEL_TREBLE                =   0x06,
        LEVEL_BASS                  =   0x07,
        LEVEL_BALANCE               =   0x08,
        LEVEL_COLOR_TEMP            =   0x09,
        LEVEL_BACKLIGHT             =   0x0A
    };


public:

    LGTVController();

    ~LGTVController();

    QStringList getIDList() const;
    
    QString description() const;

public slots:
	
    bool executeCommand(const QString &cmdID, const QString &argument);

protected:

    quint8 open(const QString &device);
    bool isOpen() const { return portIO->isOpen(); }
    quint8 close() { return portIO->close(); }

    quint8 setPower(bool isOn = true, COMMAND_TYPE cmdType = COMMAND_BROADCAST, quint8 tvID = 0);
    quint8 setSignalSource(SIGNAL_SOURCE signalSource, COMMAND_TYPE cmdType = COMMAND_BROADCAST, quint8 tvID = 0);
    quint8 setAspectRatio(ASPECT_RATIO aspectRatio, COMMAND_TYPE cmdType = COMMAND_BROADCAST, quint8 tvID = 0);
    quint8 setScreenMute(bool isMuted = true, COMMAND_TYPE cmdType = COMMAND_BROADCAST, quint8 tvID = 0);
    quint8 setVolumeMute(bool isMuted = true, COMMAND_TYPE cmdType = COMMAND_BROADCAST, quint8 tvID = 0);
    quint8 setParamLevel(LEVEL_TYPE levelType, quint8 level, COMMAND_TYPE cmdType = COMMAND_BROADCAST, quint8 tvID = 0);
    quint8 setOSD(bool isEnabled = true, COMMAND_TYPE cmdType = COMMAND_BROADCAST, quint8 tvID = 0);
    quint8 setRemoteControlLock(quint8 isLocked = true, COMMAND_TYPE cmdType = COMMAND_BROADCAST, quint8 tvID = 0);
    quint8 makeAutoConfig(COMMAND_TYPE cmdType = COMMAND_BROADCAST, quint8 tvID = 0);
    quint8 setRemoteKeyCode(quint8 code, COMMAND_TYPE cmdType = COMMAND_BROADCAST, quint8 tvID = 0);

    quint8 sendCmd(quint8 id, char cmd1, char cmd2, const QByteArray &data);



private:
    QSettings settings;
    ISerialPortIO * portIO;
    QString tvSerialPort;

};

#endif // LGTVCONTROLLER_H
