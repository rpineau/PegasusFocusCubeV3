// pegasysFocusCubeV3.h
// Pegasus Astro Focus Cube V3 X2 plugin
// Created by Rodolphe Pineau on 2024-11-06
// Copyright © 2024 Rodolphe Pineau. All rights reserved


#ifndef __PEGASUS_C__
#define __PEGASUS_C__
#include <math.h>
#include <string.h>
#include <string>
#include <vector>
#include <sstream>
#include <iostream>
#include <fstream>
#include <exception>
#include <typeinfo>
#include <stdexcept>
#include <chrono>
#include <thread>
#include <iomanip>

#ifndef WIN32
#include <unistd.h>
#endif

#include "../../licensedinterfaces/sberrorx.h"
#include "../../licensedinterfaces/serxinterface.h"
#include "../../licensedinterfaces/loggerinterface.h"
#include "../../licensedinterfaces/sleeperinterface.h"

#define PLUGIN_DEBUG 3   // define this to have log files, 1 = bad stuff only, 2 and up.. full debug

#define PLUGIN_VERSION      1.0

#define SERIAL_BUFFER_SIZE 256
#define MAX_TIMEOUT 500
#define MAX_READ_WAIT_TIMEOUT 25
#define NB_RX_WAIT 10


enum FCV3_Errors    {PLUGIN_OK = 0, NOT_CONNECTED, ND_CANT_CONNECT, BAD_CMD_RESPONSE, COMMAND_FAILED, COMMAND_TIMEOUT};
enum GetLedStatus   {OFF = 0, ON};
enum SetLEdStatus   {SWITCH_OFF = 1, SWITCH_ON};
enum MotorDir       {NORMAL = 0 , REVERSE};
enum MotorStatus    {IDLE = 0, MOVING};


typedef struct {
	bool	bReady;
	int     nCurPos;
	bool    bMoving;
    double  dTemperature;
    bool    bReverse;
    int     nBacklash;
} FCV3Status;

// field indexes in response for A command
#define fSTATUS     0
#define fPOS        1
#define fMoving     2
#define fTemp		3
#define fDir		4
#define fBacklash	5


class CPegasusFocusCubeV3
{
public:
    CPegasusFocusCubeV3();
    ~CPegasusFocusCubeV3();

    int         Connect(const std::string sPortName);
    void        Disconnect(void);
    bool        IsConnected(void) { return m_bIsConnected; };

    void        SetSerxPointer(SerXInterface *p) { m_pSerx = p; };
    // move commands
    int         haltFocuser();
    int         gotoPosition(int nPos);
    int         moveRelativeToPosision(int nSteps);

    // command complete functions
    int         isGoToComplete(bool &bComplete);
    int         isMotorMoving(bool &bMoving);

    // getter and setter
    int         getDeviceType(int &nDevice);
    int         getConsolidatedStatus(void);

    int         getMotoMaxSpeed(int &nSpeed);
    int         setMotoMaxSpeed(int nSpeed);

    int         getBacklashComp(int &nSteps);
    int         setBacklashComp(int nSteps);

    int         getFirmwareVersion(std::string sFirmareVersion);
    int         getTemperature(double &dTemperature);

    int         getPosition(int &nPosition);

    int         syncMotorPosition(int nPos);

    int         getPosLimit(void);
    void        setPosLimit(int nLimit);

    bool        isPosLimitEnabled(void);
    void        enablePosLimit(bool bEnable);

    int         setReverseEnable(bool bEnabled);
    int         getReverseEnable(bool &bEnabled);

#ifdef PLUGIN_DEBUG
	void log(std::string sLogString);
#endif

protected:

	int             deviceCommand(const std::string sCmd, std::string &sResp,  int nTimeout = MAX_TIMEOUT, char cEndOfResponse = '\n');
	int             readResponse(std::string &sResp, int nTimeout = MAX_TIMEOUT, char cEndOfResponse = '\n');
    int             parseResp(std::string sResp, std::vector<std::string>  &sParsedRes);


    SerXInterface   *m_pSerx = nullptr;
	bool			m_bNetworkConnected = false;
	std::string     m_Port;
    bool            m_bIsConnected = false;
    std::string     m_sFirmwareVersion;

    std::vector<std::string>    m_svParsedRespForA;

	FCV3Status      m_globalStatus;
    int             m_nTargetPos = 0;
    int             m_nPosLimit = 0;
    bool            m_bPosLimitEnabled = false;
	bool			m_bAbborted = false;

#ifdef PLUGIN_DEBUG
	// timestamp for logs
	const std::string getTimeStamp();
	std::ofstream m_sLogFile;
	std::string m_sLogfilePath;
#endif

};

#endif //__PEGASUS_C__
