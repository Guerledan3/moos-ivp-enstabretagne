#include <iterator>
#include "MBUtils.h"
#include "ACTable.h"
#include "Sonar.h"
#include "seanetmsg.h"
#include "math.h"

using namespace std;

Sonar::Sonar()
{
	m_iterations = 0;
	m_timewarp   = 1;

  m_bIsAlive = false;
  m_bReplyVersionData = false;
  m_bReplyBBUserData = false;
  m_bHasParams = false;
  m_bSentCfg = false;

	m_bSonarReady = false;
	m_bIsPowered = false;

  m_snrType = SeaNetMsg::MicronDST;

  m_bPollSonar = true;

  m_iParamBins = 0;

	if (!m_serial_thread.Initialise(listen_sonar_messages_thread_func, (void*)this))
	{
	  reportRunWarning("Sonar thread initialization error...\n");
	  MOOSFail("Sonar thread initialization error...\n");
	}
}

Sonar::~Sonar()
{
	MOOSTrace("iSonar: stopping aquisition thread.\n");
  if (m_serial_thread.IsThreadRunning())
    m_serial_thread.Stop();
	MOOSTrace("iSonar: finished.\n");
}

bool Sonar::OnNewMail(MOOSMSG_LIST &NewMail)
{
  AppCastingMOOSApp::OnNewMail(NewMail);
	MOOSMSG_LIST::iterator p;

	for(p = NewMail.begin() ; p != NewMail.end() ; p++)
	{
		CMOOSMsg &msg = *p;
    	string key    = msg.GetKey();

		#if 0 // Keep these around just for template
		string key   = msg.GetKey();
		string comm  = msg.GetCommunity();
		double dval  = msg.GetDouble();
		string sval  = msg.GetString();
		string msrc  = msg.GetSource();
		double mtime = msg.GetTime();
		bool   mdbl  = msg.IsDouble();
		bool   mstr  = msg.IsString();
		#endif

		// Mise à jour des paramètres du sonar
		if ( key == "MICRON_PARAMS" && msg.IsString() && m_snrType == SeaNetMsg::MicronDST)
		{
		  string msg_val = msg.GetString();
		  // Le message est de la forme "Range=25,Gain=45,Continuous=true"
      // uPokeDB SONAR_PARAMS="Range=25,Gain=45,Continuous=true, VoS=1500, invert=1, nBins=200, AngleStep=, Gain=, LeftLimit=, RightLimit="
		  double dVal=0.0; int iVal; bool bVal;
      if (MOOSValFromString(dVal, msg_val, "VoS", true))
        m_msgHeadCommand.setVOS(dVal);
		  if (MOOSValFromString(dVal, msg_val, "Range", true))
		    m_msgHeadCommand.setRange(dVal);
      if (MOOSValFromString(dVal, msg_val, "invert", true))
        m_msgHeadCommand.setInverted(dVal);
		  if (MOOSValFromString(iVal, msg_val, "nBins", true))
      {
        m_iParamBins = iVal;
		    m_msgHeadCommand.setNbins(iVal);
      }
		  if (MOOSValFromString(dVal, msg_val, "AngleStep", true))
	      	m_msgHeadCommand.setAngleStep(dVal);
		  if (MOOSValFromString(bVal, msg_val, "Continuous", true))
	     	m_msgHeadCommand.setContinuous(bVal);
		  if (MOOSValFromString(dVal, msg_val, "Gain", true))
		    m_msgHeadCommand.setGain(dVal);
		  if (MOOSValFromString(dVal, msg_val, "LeftLimit", true)){
		    m_msgHeadCommand.setLeftLimit(dVal);}
		  if (MOOSValFromString(dVal, msg_val, "RightLimit", true)){
		    m_msgHeadCommand.setRightLimit(dVal);}
		  // Envoi de la commande au sondeur
		  // TODO: vérifier que le CMOOSSerialPort est bien thread safe. Sinon, rajouter un mutex
      //SendSonarMessage(m_msgHeadCommand);
      //if we change params, sonar do not have params and had to send it again via listenMEssage Thread
      m_bHasParams = false;
		}
    else if ( key == "MICRON_PARAMS" && msg.IsString() && m_snrType == SeaNetMsg::MiniKingNotDST){/* TO AVOID UNHANDELED MAIL*/}
		else if ( key == "MINIKING_PARAMS" && msg.IsString() && m_snrType == SeaNetMsg::MiniKingNotDST)
    {
      string msg_val = msg.GetString();
      // Le message est de la forme "Range=25,Gain=45,Continuous=true"
      // uPokeDB SONAR_PARAMS="Range=25,Gain=45,Continuous=true, VoS=1500, invert=1, nBins=200, AngleStep=, Gain=, LeftLimit=, RightLimit="
      double dVal=0.0; int iVal; bool bVal;
      if (MOOSValFromString(dVal, msg_val, "VoS", true))
        m_msgHeadCommand.setVOS(dVal);
      if (MOOSValFromString(dVal, msg_val, "Range", true))
        m_msgHeadCommand.setRange(dVal);
      if (MOOSValFromString(dVal, msg_val, "invert", true))
        m_msgHeadCommand.setInverted(dVal);
      if (MOOSValFromString(iVal, msg_val, "nBins", true))
      {
        m_iParamBins = iVal;
        m_msgHeadCommand.setNbins(iVal);
      }
      if (MOOSValFromString(dVal, msg_val, "AngleStep", true))
          m_msgHeadCommand.setAngleStep(dVal);
      if (MOOSValFromString(bVal, msg_val, "Continuous", true))
        m_msgHeadCommand.setContinuous(bVal);
      if (MOOSValFromString(dVal, msg_val, "Gain", true))
        m_msgHeadCommand.setGain(dVal);
      if (MOOSValFromString(dVal, msg_val, "LeftLimit", true)){
        m_msgHeadCommand.setLeftLimit(dVal);}
      if (MOOSValFromString(dVal, msg_val, "RightLimit", true)){
        m_msgHeadCommand.setRightLimit(dVal);}
      // Envoi de la commande au sondeur
      // TODO: vérifier que le CMOOSSerialPort est bien thread safe. Sinon, rajouter un mutex
      //SendSonarMessage(m_msgHeadCommand);
      //if we change params, sonar do not have params and had to send it again via listenMEssage Thread
      m_bHasParams = false;
    }
    else if ( key == "MINIKING_PARAMS" && msg.IsString() && m_snrType == SeaNetMsg::MicronDST){/* TO AVOID UNHANDELED MAIL*/}
    else if ( key == "MICRON_POLL" && m_snrType == SeaNetMsg::MicronDST)
		{
			m_bPollSonar = (msg.GetDouble())?true:false;
			if (m_bSonarReady && m_bSentCfg && m_bHasParams)
			{
				SeaNetMsg_SendData msg_SendData;
				msg_SendData.setTime(MOOSTime());
				SendSonarMessage(msg_SendData);
				retractRunWarning("Sonar not initialized!");
			}
			else
			{
				reportRunWarning("Sonar not initialized!");
			}
		}
    else if ( key == "MICRON_POLL" && msg.IsString() && m_snrType == SeaNetMsg::MiniKingNotDST){/* TO AVOID UNHANDELED MAIL*/}
		else if ( key == "MINIKING_POLL" && m_snrType == SeaNetMsg::MiniKingNotDST)
    {
      m_bPollSonar = (msg.GetDouble())?true:false;
      if (m_bSonarReady && m_bSentCfg && m_bHasParams)
      {
        SeaNetMsg_SendData msg_SendData;
        msg_SendData.setTime(MOOSTime());
        SendSonarMessage(msg_SendData);
        retractRunWarning("Sonar not initialized!");
      }
      else
      {
        reportRunWarning("Sonar not initialized!");
      }
    }
    else if ( key == "MINIKING_POLL" && msg.IsString() && m_snrType == SeaNetMsg::MicronDST){/* TO AVOID UNHANDELED MAIL*/}
    else if ( key == "POWERED_SONAR" && m_snrType == SeaNetMsg::MicronDST)
    {
      m_bSonarReady = false;
      if (m_serial_thread.IsThreadRunning())
        m_serial_thread.Stop();
      if (msg.GetDouble() == 0)
      {
        m_bIsAlive = false;
        m_bReplyVersionData = false;
        m_bReplyBBUserData = false;
        m_bHasParams = false;
        m_bSentCfg = false;
        m_bIsPowered = false;
      }
      else
      {
        m_bIsPowered = true;
        if (!m_serial_thread.IsThreadRunning())
          m_serial_thread.Start();
      }
    }
    else if ( key == "POWERED_SONAR" && msg.IsString() && m_snrType == SeaNetMsg::MiniKingNotDST){/* TO AVOID UNHANDELED MAIL*/}
    else if ( key == "POWERED_MINIKING" && m_snrType == SeaNetMsg::MiniKingNotDST)
    {
      m_bIsAlive = false;
      m_bReplyVersionData = false;
      m_bReplyBBUserData = false;
      m_bHasParams = false;
      m_bSentCfg = false;
      m_bSonarReady = false;

      if (m_serial_thread.IsThreadRunning())
        m_serial_thread.Stop();
      if (msg.GetDouble() == 0)
        m_bIsPowered = false;
      else
      {
        m_bIsPowered = true;
        if (!m_serial_thread.IsThreadRunning())
          m_serial_thread.Start();
      }
    }
    else if ( key == "POWERED_MINIKING" && msg.IsString() && m_snrType == SeaNetMsg::MicronDST){/* TO AVOID UNHANDELED MAIL*/}
    else if(key != "APPCAST_REQ") // handle by AppCastingMOOSApp
      reportRunWarning("Unhandled Mail: " + key);
	}

	return(true);
}

bool Sonar::OnConnectToServer()
{
	// register for variables here
	// possibly look at the mission file?
	// m_MissionReader.GetConfigurationParam("Name", <string>);
	// m_Comms.Register("VARNAME", 0);

	RegisterVariables();
	return(true);
}

bool Sonar::Iterate()
{
  AppCastingMOOSApp::Iterate();
	m_iterations++;
  AppCastingMOOSApp::PostReport();
	return(true);
}

void Sonar::ListenSonarMessages()
{
  const int buf_size = 512;
  char buf[buf_size];
  string sBuf;

  union HeadInf {
    char c;
    struct {
      int InCenter : 1;
      int Centered : 1;
      int Motoring : 1;
      int MotorOn  : 1;
      int Dir : 1;
      int InScan : 1;
      int NoParams : 1;
      int SentCfg : 1;
    } bits;
  } headInf;

  if (m_bPortOpened)
    reportRunWarning("In MtMessageListenning thread::Serial Port not oppenned!");

  while (!m_serial_thread.IsQuitRequested())
  {
    int msg_size = 0;
    int needed_len = SeaNetMsg::numberBytesMissing(sBuf, msg_size);

    if (needed_len == SeaNetMsg::mrNotAMessage)
    {
      // Remove first character if the header cannot be decoded
      sBuf.erase(0,1);
    }
    else if (needed_len > 0)
    {
      // Read more data as needed
      int nb_read = m_Port.ReadNWithTimeOut(buf, needed_len);
      sBuf.append(buf, nb_read);
      // cout<<"ok"<<endl;
    }
    else if (needed_len == 0)
    {
      // Process message
      // cout << "Found message " << SeaNetMsg::detectMessageType(sBuf) << endl;
      SeaNetMsg snmsg(sBuf);
      // cout << "Created message with type " << snmsg.messageType() << endl;
      // snmsg.print_hex();

      if (snmsg.messageType() == SeaNetMsg::mtAlive)
      {
        headInf.c = snmsg.data().at(20);

        m_bHasParams = !headInf.bits.NoParams;
        m_bSentCfg = headInf.bits.SentCfg;
        // Update m_bSonarReady
        m_bSonarReady = m_bHasParams && m_bSentCfg;
        m_bIsAlive = true;

        // if(m_bHasParams && m_snrType == SeaNetMsg::MiniKingNotDST)Notify("MT_MESSAGE_MINIKING", "mtAlive, has params");
        // if(m_bHasParams && m_snrType == SeaNetMsg::MicronDST)Notify("MT_MESSAGE_MICRON", "mtAlive, has params");
        // if(m_bSentCfg && m_snrType == SeaNetMsg::MiniKingNotDST)Notify("MT_MESSAGE_MINIKING", "mtAlive, sent cfg");
        // if(m_bSentCfg && m_snrType == SeaNetMsg::MicronDST)Notify("MT_MESSAGE_MICRON", "mtAlive, sent cfg");
      }
      else if (snmsg.messageType() == SeaNetMsg::mtVersionData)
      {
        m_bReplyVersionData = true;
        // if(m_snrType == SeaNetMsg::MicronDST)Notify("MT_MESSAGE_MICRON", "REPLY mtVersionData");
        // if(m_snrType == SeaNetMsg::MiniKingNotDST)Notify("MT_MESSAGE_MINIKING", "REPLY mtVersionData");
      }
      else if (snmsg.messageType() == SeaNetMsg::mtBBUserData)
      {
        const SeaNetMsg_BBUserData * pBBUserData = reinterpret_cast<SeaNetMsg_BBUserData*> (&snmsg);
        m_snrType = pBBUserData->getSonarType();
        m_bReplyBBUserData = true;
        // if(m_snrType == SeaNetMsg::MicronDST)Notify("MT_MESSAGE_MICRON", "REPLY mtBBUserData");
        // if(m_snrType == SeaNetMsg::MiniKingNotDST)Notify("MT_MESSAGE_MINIKING", "REPLY mtBBUserData");
        // reportRunWarning("Sonar Type : " + (m_snrType == SeaNetMsg::MicronDST)?"MicronSnr":"MinikingSnr");
      }
      else if (snmsg.messageType() == SeaNetMsg::mtHeadData)
      {
        // snmsg.print_hex();
	      const SeaNetMsg_HeadData * pHdta = reinterpret_cast<SeaNetMsg_HeadData*> (&snmsg);

	      // MOOSDB raw data
	      vector<int> vScanline;
	      for (int k=0; k<pHdta->nBins(); ++k)
        	vScanline.push_back( pHdta->scanlineData()[k] );

	      stringstream ss;
	      ss << "bearing=" << pHdta->bearingDeg()*M_PI/180.0 << ",";
	      ss << "ad_interval=" << pHdta->ADInterval_m() << ",";
	      ss << "scanline=";
	      Write(ss, vScanline);
        if (pHdta->nBins() <= (m_iParamBins*1.5))
        {
          if(m_snrType == SeaNetMsg::MicronDST)
            Notify("SONAR_RAW_DATA_MICRON", ss.str());
          else if(m_snrType == SeaNetMsg::MiniKingNotDST)
            Notify("SONAR_RAW_DATA_MINIKING", ss.str());
        }

	      // ***************************************

	      // stringstream ss2;
	      // ss2 << "bearing=" << pHdta->bearing() << ","
	      //     << "distance=" << pHdta->firstObstacleDist(90, 0.5, 100.); // thd, min, max
	      // Notify("SONAR_DISTANCE", ss2.str());
      }
      sBuf.erase(0,msg_size);

      //Take a breath before talking
      MOOSPause(500);

      if (m_bIsAlive && !m_bSonarReady)
      {
        if(!m_bReplyVersionData)
        {
          SendSonarMessage(SeaNetMsg_SendVersion());
          // if(m_snrType == SeaNetMsg::MicronDST)Notify("MT_MESSAGE_MICRON", "SEND mtSendVersion");
          // if(m_snrType == SeaNetMsg::MiniKingNotDST)Notify("MT_MESSAGE_MINIKING", "SEND mtSendVersion");
        }
        else if(m_bReplyVersionData && !m_bReplyBBUserData)
        {
          SendSonarMessage(SeaNetMsg_SendBBUser());
          // if(m_snrType == SeaNetMsg::MicronDST)Notify("MT_MESSAGE_MICRON", "SEND mtSendBBUser");
          // if(m_snrType == SeaNetMsg::MiniKingNotDST)Notify("MT_MESSAGE_MINIKING", "SEND mtSendBBUser");
        }
        else if(m_bReplyVersionData && m_bReplyBBUserData && !m_bHasParams)
        {
          SendSonarMessage(m_msgHeadCommand);
          // if(m_snrType == SeaNetMsg::MicronDST)Notify("MT_MESSAGE_MICRON", "SEND mtHeadCommand");
          // if(m_snrType == SeaNetMsg::MiniKingNotDST)Notify("MT_MESSAGE_MINIKING", "SEND mtHeadCommand");
        }
      }
      if (m_bSonarReady && m_bPollSonar && m_bHasParams && m_bSentCfg) {
        SeaNetMsg_SendData msg_SendData;
        msg_SendData.setTime(MOOSTime());
        SendSonarMessage(msg_SendData);
      }
    }
  }
}

bool Sonar::OnStartUp()
{
  	AppCastingMOOSApp::OnStartUp();
	setlocale(LC_ALL, "C");
	list<string> sParams;
	m_MissionReader.EnableVerbatimQuoting(false);

	if(m_MissionReader.GetConfiguration(GetAppName(), sParams))
	{
    MOOSTrace("iSonar: Reading configuration\n");
    list<string>::iterator p;
    for(p = sParams.begin() ; p != sParams.end() ; p++)
    {
			string original_line = *p;
			string param = stripBlankEnds(toupper(biteString(*p, '=')));
			string value = stripBlankEnds(*p);

			MOOSTrace(original_line);

			if(MOOSStrCmp(param, "RANGE"))
			    m_msgHeadCommand.setRange(atof(value.c_str()));
      if(MOOSStrCmp(param, "VOS"))
          m_msgHeadCommand.setVOS(atof(value.c_str()));
      if(MOOSStrCmp(param, "INVERT"))
          m_msgHeadCommand.setInverted(atoi(value.c_str()));
			if(MOOSStrCmp(param, "NBINS"))
      {
        m_iParamBins = atoi(value.c_str());
        m_msgHeadCommand.setNbins(m_iParamBins);
      }
			if(MOOSStrCmp(param, "ANGLESTEP"))
			    m_msgHeadCommand.setAngleStep(atof(value.c_str()));
			if(MOOSStrCmp(param, "CONTINUOUS"))
			    m_msgHeadCommand.setContinuous(MOOSStrCmp(value,"true"));
			if(MOOSStrCmp(param, "GAIN"))
			    m_msgHeadCommand.setGain(atof(value.c_str()));
			if(MOOSStrCmp(param, "LEFTLIMIT"))
			    m_msgHeadCommand.setLeftLimit(atof(value.c_str()));
			if(MOOSStrCmp(param, "RIGHTLIMIT"))
			    m_msgHeadCommand.setRightLimit(atof(value.c_str()));
      if(MOOSStrCmp(param, "POWERED_AT_START"))
          m_bIsPowered = MOOSStrCmp(value.c_str(),"TRUE");
      if(MOOSStrCmp(param, "SERIAL_PORT"))
          m_portName = value.c_str();
      if(MOOSStrCmp(param, "SONAR_TYPE"))
      {
          m_snrType = (MOOSStrCmp(value.c_str(),"Miniking"))?SeaNetMsg::MiniKingNotDST:SeaNetMsg::SonarTypeError;
          if (m_snrType == SeaNetMsg::SonarTypeError)
            m_snrType = (MOOSStrCmp(value.c_str(),"Micron"))?SeaNetMsg::MicronDST:SeaNetMsg::SonarTypeError;
          if (m_snrType == SeaNetMsg::SonarTypeError)
            reportConfigWarning("Sonar type initialization error...\n");
      }
    }
	}
	else
		MOOSTrace("No configuration read.\n");

	m_bPortOpened = this->m_Port.Create(m_portName.c_str(), 115200);
  if(!m_bPortOpened)
    reportConfigWarning("Error Openning Serial Port");

  string tt;
  STRING_LIST params;
  // string ttt1("PORT=");
  // char numstr1[30] = {0}; // enough to hold all numbers up to 64-bits
  // sprintf(numstr1, "%s", m_portName.c_str());
  // tt = ttt1 + numstr1;
  // params.push_back(tt);
  // tt = "";
  // string ttt("BAUDRATE=");
  // char numstr[30] = {0}; // enough to hold all numbers up to 64-bits
  // sprintf(numstr, "%d", 115200);
  // tt = ttt + numstr;
  // params.push_back(tt);

  params.push_back("PORT=" + m_portName);
  params.push_back("BAUDRATE=115200");

  if(m_bPortOpened)
  {
    m_bPortOpened = this->m_Port.Configure(params);
  	//this->m_Port.SetTermCharacter('\n');
  	m_Port.Flush();
  }

  	m_timewarp = GetMOOSTimeWarp();

  	RegisterVariables();

  	if (m_bPortOpened && m_bIsPowered && !m_serial_thread.IsThreadRunning())
      m_serial_thread.Start();

	return(true);
}

void Sonar::RegisterVariables()
{
  AppCastingMOOSApp::RegisterVariables();
	Register("MICRON_PARAMS", 0);
	Register("MICRON_POLL", 0);
  Register("POWERED_SONAR", 0);
  Register("MINIKING_PARAMS", 0);
  Register("MINIKING_POLL", 0);
  Register("POWERED_MINIKING", 0);
}
//------------------------------------------------------------
// Procedure: buildReport()

bool Sonar::buildReport()
{
  #if 0 // Keep these around just for template
    m_msgs << "============================================ \n";
    m_msgs << "File:                                        \n";
    m_msgs << "============================================ \n";

    ACTable actab(4);
    actab << "Alpha | Bravo | Charlie | Delta";
    actab.addHeaderLines();
    actab << "one" << "two" << "three" << "four";
    m_msgs << actab.getFormattedString();
  #endif
    // m_msgs << "==============================================================\n";
    // m_msgs << "iSonarStatus :                                                \n";
    // m_msgs << "==============================================================\n";


    ACTable actab(4);
    actab << "SerialPort | Baudrate | Is Powered | Thread Running ";
    actab.addHeaderLines();
    string sonarIsPowered = (m_bIsPowered)?"yes":"no";
    string sonarThreadIsRunning = (m_serial_thread.IsThreadRunning())?"yes":"no";
    actab << m_portName << m_Port.GetBaudRate() << sonarIsPowered << sonarThreadIsRunning;
    m_msgs << actab.getFormattedString();
    m_msgs << "\n==============================================================\n";

    ACTable actab2(4);
    actab2 << "Has params | Sent cfg | Is ready | Is polling";
    actab2.addHeaderLines();
    string sonarHasParams = (m_bHasParams)?"yes":"no";
    string sonarSentCfg = (m_bSentCfg)?"yes":"no";
    string sonarIsReady = (m_bSonarReady)?"yes":"no";
    string sonarIsPolling = (m_bPollSonar)?"yes":"no";
    actab2 << sonarHasParams << sonarSentCfg << sonarIsReady << sonarIsPolling;
    m_msgs << actab2.getFormattedString();
    m_msgs << "\n================== SNR PARAMETERS ===========================\n";

    ACTable actab3(5);
    actab3 << "Range | NBins | AngleStep | Gain | Inverted";
    actab3.addHeaderLines();
    actab3 << m_msgHeadCommand.getRange() << m_msgHeadCommand.getNbins() << m_msgHeadCommand.getAngleStep() << m_msgHeadCommand.getGain() << m_msgHeadCommand.getInverted();
    m_msgs << actab3.getFormattedString();
    m_msgs << "\n==============================================================\n";
    ACTable actab4(5);
    actab4 << "Sonar Type | VoS | Continuous | LeftLimit | RightLimit";
    string sonarType;
    if (m_snrType == SeaNetMsg::SonarTypeError)
      sonarType = "Sonar Type!";
    else if(m_snrType == SeaNetMsg::MicronDST)
      sonarType = "Micron Sonar";
    else if(m_snrType == SeaNetMsg::MiniKingNotDST)
      sonarType = "MiniKing Sonar";
    else if(m_snrType == SeaNetMsg::noBBUserData)
      sonarType = "No mtBBUserData!";
    actab4 << sonarType << m_msgHeadCommand.getVOS() << m_msgHeadCommand.getContinuous() << m_msgHeadCommand.getLeftLimit() << m_msgHeadCommand.getRightLimit();
    m_msgs << actab4.getFormattedString();

  return true;
}

double Sonar::MOOSGrad2Rad(double angle){
  return angle*M_PI/200.0;
}
