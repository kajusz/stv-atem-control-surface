#pragma once

#include <memory>
#include "kdevice.h"
#include "qatemconnection.h"

#include <QSettings>

class control : public QObject
{
	Q_OBJECT
public:
	control(std::shared_ptr<kDevice> devc);
	~control(void);

	void registerAtem(std::shared_ptr<QAtemConnection> atem);

	void doTasks(void);

	typedef unsigned int atemId;

signals:
	void mwwManuallyConnect(void);
	void mwwProgress(unsigned int value);
	void mwwReady(void);
	void mwwStop(QString reason);

public slots:
	void mwwConnect(void);

	void devConnected(void);
	void devDisconnected(void);
	void devBtnCommandKeyDown(uint8_t keyNum);
	void devBtnCommandKeyUp(uint8_t keyNum);
	void devBtnGroupKeyDown(uint8_t gpId, uint8_t keyNum);
	void devBtnGroupKeyUp(uint8_t gpId, uint8_t keyNum);
	void devTBarMove(uint16_t pos);
	void devEncMove(uint8_t encId, int8_t data);
	void devJoystickMove(joystick_t data);

	void atmConnected(atemId id);
	void atmDisconnected(atemId id);
	void atmProgramInputChanged(atemId id, quint8 me, quint16 oldIndex, quint16 newIndex);
	void atmPreviewInputChanged(atemId id, quint8 me, quint16 oldIndex, quint16 newIndex);

private:
	void iconnect(void);
	void incrementalProgress(void);

	unsigned int totalTasks = 0;
	unsigned int tasks = 0;

	std::vector<std::shared_ptr<QAtemConnection>> bmd;

	std::shared_ptr<kDevice> dev;
};
