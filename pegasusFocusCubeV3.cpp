//
//  nexdome.cpp
//  NexDome X2 plugin
//
//  Created by Rodolphe Pineau on 6/11/2016.

#include "pegasusFocusCubeV3.h"


CPegasusFocusCobeV3::CPegasusFocusCobeV3()
{
	std::memset(&m_globalStatus,0,sizeof(m_globalStatus));

#ifdef PLUGIN_DEBUG
#if defined(WIN32)
	m_sLogfilePath = getenv("HOMEDRIVE");
	m_sLogfilePath += getenv("HOMEPATH");
	m_sLogfilePath += "\\PegasusFCv3Log.txt";
#else
	m_sLogfilePath = getenv("HOME");
	m_sLogfilePath += "/PegasusFCv3Log.txt";
#endif
	m_sLogFile.open(m_sLogfilePath, std::ios::out |std::ios::trunc);
#endif

#if defined PLUGIN_DEBUG && PLUGIN_DEBUG >= 2
	m_sLogFile << "["<<getTimeStamp()<<"]"<< " [" << __func__ << "] Version " << std::fixed << std::setprecision(2) << PLUGIN_VERSION << " build " << __DATE__ << " " << __TIME__ << std::endl;
	m_sLogFile << "["<<getTimeStamp()<<"]"<< " [" << __func__ << "] Constructor Called." << std::endl;
	m_sLogFile.flush();
#endif

}

CPegasusFocusCobeV3::~CPegasusFocusCobeV3()
{
#if defined PLUGIN_DEBUG && PLUGIN_DEBUG >= 2
	m_sLogFile << "["<<getTimeStamp()<<"]"<< " [" << __func__ << "] Called." << std::endl;
	m_sLogFile.flush();
	// Close LogFile
	if(m_sLogFile.is_open())
		m_sLogFile.close();
#endif
}

int CPegasusFocusCobeV3::Connect(const std::string sPortName)
{
    int nErr = PLUGIN_OK;
    int nDevice;

    if(!m_pSerx)
        return NOT_CONNECTED;

#if defined PLUGIN_DEBUG && PLUGIN_DEBUG >= 2
	m_sLogFile << "["<<getTimeStamp()<<"]"<< " [" << __func__ << "] Connect Called." << std::endl;
	m_sLogFile << "["<<getTimeStamp()<<"]"<< " [" << __func__ << "] Trying to connect to port " << sPortName << std::endl;
	m_sLogFile.flush();
#endif

    // 19200 8N1
	nErr = m_pSerx->open(sPortName.c_str(), 115200, SerXInterface::B_NOPARITY, "-DTR_CONTROL 1");
    if(nErr == 0)
        m_bIsConnected = true;
    else
        m_bIsConnected = false;

    if(!m_bIsConnected)
        return nErr;

	m_Port.assign(sPortName);


#if defined PLUGIN_DEBUG && PLUGIN_DEBUG >= 2
	m_sLogFile << "["<<getTimeStamp()<<"]"<< " [" << __func__ << "] connected to " << m_Port << std::endl;
	m_sLogFile.flush();
#endif


	if(m_Port.size()>=3 && m_Port.find("TCP") != std::string::npos)  {
		m_bNetworkConnected = true;
	}
	else
		m_bNetworkConnected = false;

	nErr = getDeviceType(nDevice);
    if(nErr) {
		m_bIsConnected = false;
        return nErr;
    }
    // m_globalStatus.deviceType now contains the device type
    return nErr;
}

void CPegasusFocusCobeV3::Disconnect()
{
#if defined PLUGIN_DEBUG && PLUGIN_DEBUG >= 2
	m_sLogFile << "["<<getTimeStamp()<<"]"<< " [" << __func__ << "] Called." << std::endl;
	m_sLogFile.flush();
#endif
    if(m_bIsConnected && m_pSerx)
        m_pSerx->close();
 
	m_bIsConnected = false;
}

#pragma mark move commands
int CPegasusFocusCobeV3::haltFocuser()
{
    int nErr;
    std::string sResp;

	if(!m_bIsConnected)
		return NOT_CONNECTED;

#if defined PLUGIN_DEBUG && PLUGIN_DEBUG >= 2
	m_sLogFile << "["<<getTimeStamp()<<"]"<< " [" << __func__ << "] Called." << std::endl;
	m_sLogFile.flush();
#endif

    nErr = deviceCommand("FH\n", sResp, SERIAL_BUFFER_SIZE);
	m_bAbborted = true;
	
	return nErr;
}

int CPegasusFocusCobeV3::gotoPosition(int nPos)
{
    int nErr;
    std::string sCmd;
    std::string sResp;
	std::stringstream ssCmd;

	if(!m_bIsConnected)
		return NOT_CONNECTED;

#if defined PLUGIN_DEBUG && PLUGIN_DEBUG >= 2
	m_sLogFile << "["<<getTimeStamp()<<"]"<< " [" << __func__ << "] Called." << std::endl;
	m_sLogFile.flush();
#endif

    if (m_bPosLimitEnabled && nPos>m_nPosLimit)
        return ERR_LIMITSEXCEEDED;

#if defined PLUGIN_DEBUG && PLUGIN_DEBUG >= 2
		m_sLogFile << "["<<getTimeStamp()<<"]"<< " [" << __func__ << "] moving to : " << nPos << std::endl;
		m_sLogFile.flush();
#endif

	ssCmd << "FM:" << nPos << std::endl;
	nErr = deviceCommand(ssCmd.str(), sResp, SERIAL_BUFFER_SIZE);
    m_nTargetPos = nPos;

    return nErr;
}

int CPegasusFocusCobeV3::moveRelativeToPosision(int nSteps)
{
    int nErr;

	if(!m_bIsConnected)
		return NOT_CONNECTED;
#if defined PLUGIN_DEBUG && PLUGIN_DEBUG >= 2
	m_sLogFile << "["<<getTimeStamp()<<"]"<< " [" << __func__ << "] Called." << std::endl;
	m_sLogFile << "["<<getTimeStamp()<<"]"<< " [" << __func__ << "] moving by : " << nSteps << std::endl;
	m_sLogFile.flush();
#endif

    m_nTargetPos = m_globalStatus.nCurPos + nSteps;
    nErr = gotoPosition(m_nTargetPos);
    return nErr;
}

#pragma mark command complete functions

int CPegasusFocusCobeV3::isGoToComplete(bool &bComplete)
{
    int nErr = PLUGIN_OK;
	
	if(!m_bIsConnected)
		return NOT_CONNECTED;
#if defined PLUGIN_DEBUG && PLUGIN_DEBUG >= 2
	m_sLogFile << "["<<getTimeStamp()<<"]"<< " [" << __func__ << "] Called." << std::endl;
	m_sLogFile.flush();
#endif

    getPosition(m_globalStatus.nCurPos);
#if defined PLUGIN_DEBUG && PLUGIN_DEBUG >= 2
	m_sLogFile << "["<<getTimeStamp()<<"]"<< " [" << __func__ << "] m_globalStatus.nCurPos = " << m_globalStatus.nCurPos << std::endl;
	m_sLogFile << "["<<getTimeStamp()<<"]"<< " [" << __func__ << "] m_nTargetPos = " << m_nTargetPos << std::endl;
	m_sLogFile.flush();
#endif

    if(m_bAbborted) {
		bComplete = true;
		m_nTargetPos = m_globalStatus.nCurPos;
		m_bAbborted = false;
	}
    else if(m_globalStatus.nCurPos == m_nTargetPos)
        bComplete = true;
    else
        bComplete = false;
    return nErr;
}

int CPegasusFocusCobeV3::isMotorMoving(bool &bMoving)
{
    int nErr = PLUGIN_OK;
    std::string sResp;
	
	if(!m_bIsConnected)
		return NOT_CONNECTED;
#if defined PLUGIN_DEBUG && PLUGIN_DEBUG >= 2
	m_sLogFile << "["<<getTimeStamp()<<"]"<< " [" << __func__ << "] Called." << std::endl;
	m_sLogFile.flush();
#endif

    nErr = deviceCommand("FI\n", sResp, SERIAL_BUFFER_SIZE);
    if(nErr)
        return nErr;

	if(std::stoi(sResp)) {
        bMoving = true;
        m_globalStatus.bMoving = MOVING;
    }
    else {
        bMoving = false;
        m_globalStatus.bMoving = IDLE;
    }

    return nErr;
}

#pragma mark getters and setters
int CPegasusFocusCobeV3::getDeviceType(int &nDevice)
{
	int nErr = PLUGIN_OK;
    std::string sResp;
	
	if(!m_bIsConnected)
		return NOT_CONNECTED;

#if defined PLUGIN_DEBUG && PLUGIN_DEBUG >= 2
	m_sLogFile << "["<<getTimeStamp()<<"]"<< " [" << __func__ << "] Called." << std::endl;
	m_sLogFile.flush();
#endif

	nErr = deviceCommand("F#\n", sResp, SERIAL_BUFFER_SIZE);
    if(nErr)
        return nErr;

	if(sResp.find("FC3_") != std::string::npos) {
			getConsolidatedStatus();
    }
    else {
        nErr = COMMAND_FAILED;
    }

    return nErr;
}

int CPegasusFocusCobeV3::getConsolidatedStatus()
{
    int nErr;
    std::string sResp;
	
	if(!m_bIsConnected)
		return NOT_CONNECTED;

#if defined PLUGIN_DEBUG && PLUGIN_DEBUG >= 2
	m_sLogFile << "["<<getTimeStamp()<<"]"<< " [" << __func__ << "] Called." << std::endl;
	m_sLogFile.flush();
#endif

    nErr = deviceCommand("FA\n", sResp, SERIAL_BUFFER_SIZE);
    if(nErr)
        return nErr;


    // parse response
    nErr = parseResp(sResp, m_svParsedRespForA);
    if(m_svParsedRespForA.empty()) {
#if defined PLUGIN_DEBUG && PLUGIN_DEBUG >= 2
		m_sLogFile << "["<<getTimeStamp()<<"]"<< " [" << __func__ << "] parsing returned an empty vector." << std::endl;
		m_sLogFile.flush();
#endif
        return BAD_CMD_RESPONSE;
    }

#if defined PLUGIN_DEBUG && PLUGIN_DEBUG >= 2
	m_sLogFile << "["<<getTimeStamp()<<"]"<< " [" << __func__ << "] Status = " << m_svParsedRespForA[fSTATUS] << std::endl;
	m_sLogFile.flush();
#endif


	if(m_svParsedRespForA[fSTATUS].find("FC#") != std::string::npos) {
        m_globalStatus.bReady = true;
    }
    else {
        m_globalStatus.bReady = false;
    }
    
    m_globalStatus.nCurPos = std::stoi(m_svParsedRespForA[fPOS]);
	m_globalStatus.bMoving = (std::stoi(m_svParsedRespForA[fMoving]) == 1)? true:false;
	m_globalStatus.dTemperature = std::stod(m_svParsedRespForA[fTemp]);
	m_globalStatus.bReverse = (std::stod(m_svParsedRespForA[fDir]) == 1)? true:false;
	m_globalStatus.nBacklash = std::stoi(m_svParsedRespForA[fBacklash]);

#if defined PLUGIN_DEBUG && PLUGIN_DEBUG >= 2
	m_sLogFile << "["<<getTimeStamp()<<"]"<< " [" << __func__ << "] m_globalStatus.nCurPos = " << m_globalStatus.nCurPos << std::endl;
	m_sLogFile << "["<<getTimeStamp()<<"]"<< " [" << __func__ << "] m_globalStatus.bMoving = " << (m_globalStatus.bMoving?"Yes":"No") << std::endl;
	m_sLogFile << "["<<getTimeStamp()<<"]"<< " [" << __func__ << "] m_globalStatus.dTemperature = " << m_globalStatus.dTemperature << std::endl;
	m_sLogFile << "["<<getTimeStamp()<<"]"<< " [" << __func__ << "] m_globalStatus.bReverse = " << (m_globalStatus.bReverse?"Yes":"No") << std::endl;
	m_sLogFile << "["<<getTimeStamp()<<"]"<< " [" << __func__ << "] m_globalStatus.nBacklash = " << m_globalStatus.nBacklash << std::endl;
	m_sLogFile.flush();
#endif

    return nErr;
}

int CPegasusFocusCobeV3::getMotoMaxSpeed(int &nSpeed)
{
    int nErr;
    std::string sResp;
    std::vector<std::string> svParsedResp;
	
	if(!m_bIsConnected)
		return NOT_CONNECTED;

#if defined PLUGIN_DEBUG && PLUGIN_DEBUG >= 2
	m_sLogFile << "["<<getTimeStamp()<<"]"<< " [" << __func__ << "] Called." << std::endl;
	m_sLogFile.flush();
#endif

    nErr = deviceCommand("SP\n", sResp, SERIAL_BUFFER_SIZE);
    if(nErr)
        return nErr;

    // parse response
    svParsedResp.clear();
    nErr = parseResp(sResp, svParsedResp);
	nSpeed = std::stoi(svParsedResp[1]);
#if defined PLUGIN_DEBUG && PLUGIN_DEBUG >= 2
	m_sLogFile << "["<<getTimeStamp()<<"]"<< " [" << __func__ << "] nSpeed = " << nSpeed << std::endl;
	m_sLogFile.flush();
#endif


    return nErr;
}

int CPegasusFocusCobeV3::setMotoMaxSpeed(int nSpeed)
{
    int nErr;
    std::string sCmd;
    std::string sResp;
	std::stringstream ssCmd;

	if(!m_bIsConnected)
		return NOT_CONNECTED;

#if defined PLUGIN_DEBUG && PLUGIN_DEBUG >= 2
	m_sLogFile << "["<<getTimeStamp()<<"]"<< " [" << __func__ << "] Called." << std::endl;
	m_sLogFile.flush();
#endif

	ssCmd << "FS:" << nSpeed << std::endl;
    nErr = deviceCommand(ssCmd.str(), sResp, SERIAL_BUFFER_SIZE);

    return nErr;
}

int CPegasusFocusCobeV3::getBacklashComp(int &nSteps)
{
    int nErr;
	
	if(!m_bIsConnected)
		return NOT_CONNECTED;

#if defined PLUGIN_DEBUG && PLUGIN_DEBUG >= 2
	m_sLogFile << "["<<getTimeStamp()<<"]"<< " [" << __func__ << "] Called." << std::endl;
	m_sLogFile.flush();
#endif

    nErr = getConsolidatedStatus();
    nSteps = m_globalStatus.nBacklash;

    return nErr;
}

int CPegasusFocusCobeV3::setBacklashComp(int nSteps)
{
    int nErr = PLUGIN_OK;
    std::string sCmd;
    std::string sResp;
	std::stringstream ssCmd;

	if(!m_bIsConnected)
		return NOT_CONNECTED;

#if defined PLUGIN_DEBUG && PLUGIN_DEBUG >= 2
	m_sLogFile << "["<<getTimeStamp()<<"]"<< " [" << __func__ << "] Called." << std::endl;
	m_sLogFile.flush();
#endif


	ssCmd << "BL:" << nSteps << std::endl;
    nErr = deviceCommand(ssCmd.str(), sResp, SERIAL_BUFFER_SIZE);
    if(!nErr)
        m_globalStatus.nBacklash = nSteps;

    return nErr;
}


int CPegasusFocusCobeV3::getFirmwareVersion(std::string sFirmareVersion)
{
    int nErr = PLUGIN_OK;
    std::string sResp;

    if(!m_bIsConnected)
        return NOT_CONNECTED;

#if defined PLUGIN_DEBUG && PLUGIN_DEBUG >= 2
	m_sLogFile << "["<<getTimeStamp()<<"]"<< " [" << __func__ << "] Called." << std::endl;
	m_sLogFile.flush();
#endif

    nErr = deviceCommand("FV\n", sResp, SERIAL_BUFFER_SIZE);
    if(nErr)
        return nErr;

	sFirmareVersion.assign(sResp);
    return nErr;
}

int CPegasusFocusCobeV3::getTemperature(double &dTemperature)
{
    int nErr = PLUGIN_OK;
    std::string sResp;
	
	if(!m_bIsConnected)
		return NOT_CONNECTED;

#if defined PLUGIN_DEBUG && PLUGIN_DEBUG >= 2
	m_sLogFile << "["<<getTimeStamp()<<"]"<< " [" << __func__ << "] Called." << std::endl;
	m_sLogFile.flush();
#endif

    nErr = deviceCommand("FT\n", sResp, SERIAL_BUFFER_SIZE);
    if(nErr)
        return nErr;

    // convert response
    dTemperature = std::stod(sResp);

    return nErr;
}

int CPegasusFocusCobeV3::getPosition(int &nPosition)
{
    int nErr = PLUGIN_OK;
    std::string sResp;
	
	if(!m_bIsConnected)
		return NOT_CONNECTED;

#if defined PLUGIN_DEBUG && PLUGIN_DEBUG >= 2
	m_sLogFile << "["<<getTimeStamp()<<"]"<< " [" << __func__ << "] Called." << std::endl;
	m_sLogFile.flush();
#endif


	nErr = getConsolidatedStatus();
    if(nErr)
        return nErr;

    // convert response
	nPosition = m_globalStatus.nCurPos;
	return nErr;
}


int CPegasusFocusCobeV3::syncMotorPosition(int nPos)
{
    int nErr = PLUGIN_OK;
    std::string sCmd;
	std::string sResp;
	std::stringstream ssCmd;

	if(!m_bIsConnected)
		return NOT_CONNECTED;

#if defined PLUGIN_DEBUG && PLUGIN_DEBUG >= 2
	m_sLogFile << "["<<getTimeStamp()<<"]"<< " [" << __func__ << "] Called." << std::endl;
	m_sLogFile.flush();
#endif

	ssCmd << "FN:" << nPos << std::endl;
    nErr = deviceCommand(ssCmd.str(), sResp);
    nErr |= getConsolidatedStatus();
    return nErr;
}

int CPegasusFocusCobeV3::getPosLimit()
{
    return m_nPosLimit;
}

void CPegasusFocusCobeV3::setPosLimit(int nLimit)
{
    m_nPosLimit = nLimit;
}

bool CPegasusFocusCobeV3::isPosLimitEnabled()
{
    return m_bPosLimitEnabled;
}

void CPegasusFocusCobeV3::enablePosLimit(bool bEnable)
{
    m_bPosLimitEnabled = bEnable;
}


int CPegasusFocusCobeV3::setReverseEnable(bool bEnabled)
{
    int nErr = PLUGIN_OK;
    std::string sResp;
    std::string sCmd;
	std::stringstream ssCmd;

	if(!m_bIsConnected)
		return NOT_CONNECTED;

#if defined PLUGIN_DEBUG && PLUGIN_DEBUG >= 2
	m_sLogFile << "["<<getTimeStamp()<<"]"<< " [" << __func__ << "] Called." << std::endl;
	m_sLogFile.flush();
#endif

	ssCmd << "FD:" << (bEnabled?1:0) << std::endl;

    nErr = deviceCommand(ssCmd.str(), sResp);

#ifdef PLUGIN_DEBUG
    if(nErr) {
		m_sLogFile << "["<<getTimeStamp()<<"]"<< " [" << __func__ << "] Error setting direction." << std::endl;
		m_sLogFile.flush();
    }
#endif

    return nErr;
}

int CPegasusFocusCobeV3::getReverseEnable(bool &bEnabled)
{
    int nErr;
	
	if(!m_bIsConnected)
		return NOT_CONNECTED;

    nErr = getConsolidatedStatus();
    bEnabled = m_globalStatus.bReverse;

    return nErr;
}

#pragma mark command and response functions

int CPegasusFocusCobeV3::deviceCommand(const std::string sCmd, std::string &sResp, int nTimeout, char cEndOfResponse)
{
	int nErr = PLUGIN_OK;
	unsigned long  ulBytesWrite;
	std::string localResp;

	if(!m_bIsConnected)
		return NOT_CONNECTED;

	m_pSerx->purgeTxRx();
#if defined PLUGIN_DEBUG && PLUGIN_DEBUG >= 2
	m_sLogFile << "["<<getTimeStamp()<<"]"<< " [" << __func__ << "] Sending : '" << sCmd <<  "'" << std::endl;
	m_sLogFile.flush();
#endif
	nErr = m_pSerx->writeFile((void *)(sCmd.c_str()), sCmd.size(), ulBytesWrite);
	m_pSerx->flushTx();

	if(nErr){
#if defined PLUGIN_DEBUG && PLUGIN_DEBUG >= 2
		m_sLogFile << "["<<getTimeStamp()<<"]"<< " [" << __func__ << "] writeFile error : " << nErr << std::endl;
		m_sLogFile.flush();
#endif
		return nErr;
	}

	// read response
	nErr = readResponse(localResp, nTimeout, cEndOfResponse);
	if(nErr)
		return nErr;

	if(!localResp.size())
		return BAD_CMD_RESPONSE;

	sResp.assign(localResp);

#if defined PLUGIN_DEBUG && PLUGIN_DEBUG >= 2
	m_sLogFile << "["<<getTimeStamp()<<"]"<< " [" << __func__ << "] response : " << sResp << std::endl;
	m_sLogFile.flush();
#endif

	return nErr;
}

int CPegasusFocusCobeV3::readResponse(std::string &sResp, int nTimeout, char cEndOfResponse)
{
	int nErr = PLUGIN_OK;
	char pszBuf[SERIAL_BUFFER_SIZE];
	unsigned long ulBytesRead = 0;
	unsigned long ulTotalBytesRead = 0;
	char *pszBufPtr;
	int nBytesWaiting = 0 ;
	int nbTimeouts = 0;

	sResp.clear();
	memset(pszBuf, 0, SERIAL_BUFFER_SIZE);
	pszBufPtr = pszBuf;

	do {
		nErr = m_pSerx->bytesWaitingRx(nBytesWaiting);
#if defined PLUGIN_DEBUG && PLUGIN_DEBUG >= 3
		m_sLogFile << "["<<getTimeStamp()<<"]"<< " [" << __func__ << "] nBytesWaiting = " << nBytesWaiting << std::endl;
		m_sLogFile << "["<<getTimeStamp()<<"]"<< " [" << __func__ << "] nBytesWaiting nErr = " << nErr << std::endl;
		m_sLogFile.flush();
#endif
		if(!nBytesWaiting) {
			nbTimeouts += MAX_READ_WAIT_TIMEOUT;
			if(nbTimeouts >= nTimeout) {
#if defined PLUGIN_DEBUG && PLUGIN_DEBUG >= 3
				m_sLogFile << "["<<getTimeStamp()<<"]"<< " [" << __func__ << "] bytesWaitingRx timeout, no data for" << nbTimeouts <<" ms" << std::endl;
				m_sLogFile.flush();
#endif
				nErr = COMMAND_TIMEOUT;
				break;
			}
			std::this_thread::sleep_for(std::chrono::milliseconds(MAX_READ_WAIT_TIMEOUT));
			continue;
		}
		nbTimeouts = 0;
		if(ulTotalBytesRead + nBytesWaiting <= SERIAL_BUFFER_SIZE)
			nErr = m_pSerx->readFile(pszBufPtr, nBytesWaiting, ulBytesRead, nTimeout);
		else {
			nErr = ERR_RXTIMEOUT;
			break; // buffer is full.. there is a problem !!
		}
		if(nErr) {
#if defined PLUGIN_DEBUG && PLUGIN_DEBUG >= 2
			m_sLogFile << "["<<getTimeStamp()<<"]"<< " [" << __func__ << "] readFile error." << std::endl;
			m_sLogFile.flush();
#endif
			return nErr;
		}

		if (ulBytesRead != nBytesWaiting) { // timeout
#if defined PLUGIN_DEBUG && PLUGIN_DEBUG >= 2
			m_sLogFile << "["<<getTimeStamp()<<"]"<< " [" << __func__ << "] readFile Timeout Error." << std::endl;
			m_sLogFile << "["<<getTimeStamp()<<"]"<< " [" << __func__ << "] readFile nBytesWaiting = " << nBytesWaiting << std::endl;
			m_sLogFile << "["<<getTimeStamp()<<"]"<< " [" << __func__ << "] readFile ulBytesRead =" << ulBytesRead << std::endl;
			m_sLogFile.flush();
#endif
		}

		ulTotalBytesRead += ulBytesRead;
		pszBufPtr+=ulBytesRead;
	} while (ulTotalBytesRead < SERIAL_BUFFER_SIZE  && *(pszBufPtr-1) != cEndOfResponse);


#if defined PLUGIN_DEBUG && PLUGIN_DEBUG >= 2
	m_sLogFile << "["<<getTimeStamp()<<"]"<< " [" << __func__ << "] pszBuf = '" << pszBuf << "'" << std::endl;
	m_sLogFile.flush();
#endif


	if(!ulTotalBytesRead)
		nErr = COMMAND_TIMEOUT; // we didn't get an answer.. so timeout
	else
		*(pszBufPtr-1) = 0; //remove the cEndOfResponse

	sResp.assign(pszBuf);
	return nErr;
}



int CPegasusFocusCobeV3::parseResp(std::string sResp, std::vector<std::string>  &svParsedResp)
{
    std::string sSegment;
    std::vector<std::string> svSeglist;
    std::stringstream ssTmp(sResp);

#if defined PLUGIN_DEBUG && PLUGIN_DEBUG >= 2
	m_sLogFile << "["<<getTimeStamp()<<"]"<< " [" << __func__ << "] parsing '" << sResp << "'" << std::endl;
	m_sLogFile.flush();
#endif

	svParsedResp.clear();
    // split the string into vector elements
    while(std::getline(ssTmp, sSegment, ':'))
    {
        svSeglist.push_back(sSegment);
#if defined PLUGIN_DEBUG && PLUGIN_DEBUG >= 2
	m_sLogFile << "["<<getTimeStamp()<<"]"<< " [" << __func__ << "] sSegment : '" << sSegment << "'" << std::endl;
	m_sLogFile.flush();
#endif
    }

    svParsedResp = svSeglist;

    return PLUGIN_OK;
}


#ifdef PLUGIN_DEBUG

const std::string CPegasusFocusCobeV3::getTimeStamp()
{
	time_t     now = time(0);
	struct tm  tstruct;
	char       buf[80];
	tstruct = *localtime(&now);
	std::strftime(buf, sizeof(buf), "%Y-%m-%d.%X", &tstruct);

	return buf;
}
#endif
