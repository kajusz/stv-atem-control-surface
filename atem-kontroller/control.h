#pragma once

#include <QObject>

#include "kdevice.h"

#include "libatem.h"

class control : public QObject
{
	Q_OBJECT
public:
	control(std::shared_ptr<kDevice> devc);
	~control(void);

	void registerAtem(atemPtr atem); // add atem to the vector

	void doTasks(void); // initialisation

signals:
	void mwwManuallyConnect(void); // device was not found so ask the user
	void mwwProgress(unsigned int value); // update the progress bar on the loading page
	void mwwReady(void); // initialisation & connecting is done, go to home page
	void mwwStop(QString reason); // fatal error, need to go to loading page

public slots:
	void mwwConnect(void); // connect to the kontroller

	// slots for the kdevice class
	void devConnected(void); // event, kontroller was connected
	void devDisconnected(void); // event, kontroller was disconnected
	void devBtnCommandKeyUp(uint8_t keyNum);
	void devBtnGroupKeyUp(uint8_t gpId, uint8_t keyNum);
	void devTBarMove(uint16_t pos);
	void devJoystickMove(joystick_t data);

	// slots for the qatemcontrol class
	void atmConnected(atemId id); // event, atem was connected
	void atmDisconnected(atemId id); // event, atem was connected
	void atmProgramInputChanged(atemId id, quint8 me, quint16 oldIndex, quint16 newIndex);
	void atmPreviewInputChanged(atemId id, quint8 me, quint16 oldIndex, quint16 newIndex);
	void atmTransitionPreviewChanged(atemId id, quint8 me, bool state);
	void atmFadeToBlackChanged(atemId id, quint8 me, bool fading, bool enabled);
	void atmCurrentTransitionStyleChanged(atemId id, quint8 me, quint8 style);
	void atmKeyersOnCurrentTransitionChanged(atemId id, quint8 me, quint8 keyers);
	void atmUpstreamKeyOnAirChanged(atemId id, quint8 me, quint8 keyer, bool state);

private:
	// initialisation progress counter
	void incrementalProgress(void);
	unsigned int totalTasks = 0;
	unsigned int tasks = 0;

	// storage container for atem objects
	atemVec bmd;
	// reference to the kontroller
	std::shared_ptr<kDevice> dev;
};
