// Demo: NMEA2000 library. Send different NMEA 2000 messages as example to the bus.
#define ESP32_CAN_RX_PIN GPIO_NUM_15
#define ESP32_CAN_TX_PIN GPIO_NUM_2
#include <Arduino.h>
#include <NMEA2000_CAN.h>  // This will automatically choose right CAN library and create suitable NMEA2000 object
#include <N2kMessages.h>

void setup() {
  // Reserve enough buffer for sending all messages. This does not work on small memory devices like Uno or Mega
  NMEA2000.SetN2kCANSendFrameBufSize(250);
  // Set Product information
  NMEA2000.SetProductInformation("00000001", // Manufacturer's Model serial code
                                 100, // Manufacturer's product code
                                 "Message sender example",  // Manufacturer's Model ID
                                 "1.0.2.25 (2019-07-07)",  // Manufacturer's Software version code
                                 "1.0.2.0 (2019-07-07)" // Manufacturer's Model version
                                 );
  // Set device information
  NMEA2000.SetDeviceInformation(1, // Unique number. Use e.g. Serial number.
                                132, // Device function=Analog to NMEA 2000 Gateway. See codes on http://www.nmea.org/Assets/20120726%20nmea%202000%20class%20&%20function%20codes%20v%202.00.pdf
                                25, // Device class=Inter/Intranetwork Device. See codes on  http://www.nmea.org/Assets/20120726%20nmea%202000%20class%20&%20function%20codes%20v%202.00.pdf
                                2046 // Just choosen free from code list on http://www.nmea.org/Assets/20121020%20nmea%202000%20registration%20list.pdf                               
                               );
  // Uncomment 3 rows below to see, what device will send to bus                           
   Serial.begin(115200);
   NMEA2000.SetForwardStream(&Serial);  // PC output on due programming port
  // NMEA2000.SetForwardType(tNMEA2000::fwdt_Text); // Show in clear text. Leave uncommented for default Actisense format.
  // NMEA2000.SetForwardOwnMessages();

  // If you also want to see all traffic on the bus use N2km_ListenAndNode instead of N2km_NodeOnly below
  NMEA2000.SetMode(tNMEA2000::N2km_NodeOnly,22);
  //NMEA2000.SetDebugMode(tNMEA2000::dm_ClearText); // Uncomment this, so you can test code without CAN bus chips on Arduino Mega
  NMEA2000.EnableForward(true); // Disable all msg forwarding to USB (=Serial)
  NMEA2000.Open();
}

#define NavigationSendOffset 0
#define EnvironmentalSendOffset 40
#define BatterySendOffset 80
#define MiscSendOffset 120

bool IsTimeToUpdate(unsigned long NextUpdate) {
  return (NextUpdate<millis());
}
unsigned long InitNextUpdate(unsigned long Period, unsigned long Offset=0) {
  return millis()+Period+Offset;
}

void SetNextUpdate(unsigned long &NextUpdate, unsigned long Period) {
  while ( NextUpdate<millis() ) NextUpdate+=Period;
}

void loop() {
  SendN2kRapidData();
  SendN2kNavigation();
  SendN2kEnvironmental();
  SendN2kBattery();
  SendN2kMisc();
  NMEA2000.ParseMessages();
}

double ReadCabinTemp() {
  return CToKelvin(21.11); // Read here the true temperature e.g. from analog input
}

double ReadWaterTemp() {
  return CToKelvin(15.5); // Read here the true temperature e.g. from analog input
}

#define SlowDataUpdatePeriod 1000

void SendN2kNavigation() {
  static unsigned long SlowDataUpdated=InitNextUpdate(SlowDataUpdatePeriod,NavigationSendOffset);
  tN2kMsg N2kMsg;
  
  // !delay is not good practise withing loops! I use it in this sample since
  // I tested Arduino Mega as receiver without interrupt and it could not handle all messages
  // send without delay. With interrupt, it can do it and you can set DelayBetweenSend to 0.
  const int DelayBetweenSend=0; 
  
  if ( IsTimeToUpdate(SlowDataUpdated) ) {
    SetNextUpdate(SlowDataUpdated,SlowDataUpdatePeriod);

    SetN2kAttitude(N2kMsg,1,DegToRad(-3.1),DegToRad(2.4),DegToRad(-7.8));
    delay(DelayBetweenSend); NMEA2000.SendMsg(N2kMsg);

    SetN2kSystemTime(N2kMsg,1,17555,62000);
    delay(DelayBetweenSend); NMEA2000.SendMsg(N2kMsg);
    
    SetN2kLocalOffset(N2kMsg,17555,62000,120);
    delay(DelayBetweenSend); NMEA2000.SendMsg(N2kMsg);
    
    SetN2kRudder(N2kMsg,DegToRad(5),1,N2kRDO_MoveToStarboard,DegToRad(-5));
    delay(DelayBetweenSend); NMEA2000.SendMsg(N2kMsg);

    SetN2kGNSS(N2kMsg,1,17555,62000,60.1,22.5,10.5,N2kGNSSt_GPS,N2kGNSSm_GNSSfix,12,0.8,0.5,15,1,N2kGNSSt_GPS,15,2);
    delay(DelayBetweenSend); NMEA2000.SendMsg(N2kMsg);

    SetN2kGNSSDOPData(N2kMsg,1,N2kGNSSdm_Auto,N2kGNSSdm_Auto,1.2,-0.8,N2kDoubleNA);
    delay(DelayBetweenSend); NMEA2000.SendMsg(N2kMsg);

  }
}

void SendN2kEnvironmental() {
  static unsigned long SlowDataUpdated=InitNextUpdate(SlowDataUpdatePeriod,EnvironmentalSendOffset);
  tN2kMsg N2kMsg;
  
  // !delay is not good practise withing loops! I use it in this sample since
  // I tested Arduino Mega as receiver without inteerupt and it could not handle all messages
  // send without delay. With interrupt, it can do it and you can set DelayBetweenSend to 0.
  const int DelayBetweenSend=0; 
  
  if ( IsTimeToUpdate(SlowDataUpdated) ) {
    SetNextUpdate(SlowDataUpdated,SlowDataUpdatePeriod);

    SetN2kPressure(N2kMsg,0,2,N2kps_Atmospheric,mBarToPascal(1024));
    NMEA2000.SendMsg(N2kMsg);

    SetN2kSetPressure(N2kMsg,0,2,N2kps_CompressedAir,1255);
    NMEA2000.SendMsg(N2kMsg);

    SetN2kHumidity(N2kMsg,0xff,1,N2khs_InsideHumidity,41.8); //,43);
    NMEA2000.SendMsg(N2kMsg);

    SetN2kTemperatureExt(N2kMsg, 1, 1, N2kts_MainCabinTemperature, ReadCabinTemp(),CToKelvin(21.6));
    delay(DelayBetweenSend); NMEA2000.SendMsg(N2kMsg);

    SetN2kTemperature(N2kMsg, 1, 1, N2kts_MainCabinTemperature, ReadCabinTemp(),CToKelvin(21.6));
    delay(DelayBetweenSend); NMEA2000.SendMsg(N2kMsg);
    
    SetN2kEnvironmentalParameters(N2kMsg, 1, N2kts_MainCabinTemperature, ReadCabinTemp(),N2khs_InsideHumidity, 55, mBarToPascal(1013.5));
    delay(DelayBetweenSend); NMEA2000.SendMsg(N2kMsg);
    
    SetN2kOutsideEnvironmentalParameters(N2kMsg, 1, ReadWaterTemp(), CToKelvin(25.3), mBarToPascal(1013.5));
    delay(DelayBetweenSend); NMEA2000.SendMsg(N2kMsg);
  }
}

void SendN2kBattery() {
  static unsigned long SlowDataUpdated=InitNextUpdate(SlowDataUpdatePeriod,BatterySendOffset);
  tN2kMsg N2kMsg;
  
  // !delay is not good practise withing loops! I use it in this sample since
  // I tested Arduino Mega as receiver without inteerupt and it could not handle all messages
  // send without delay. With interrupt, it can do it and you can set DelayBetweenSend to 0.
  const int DelayBetweenSend=0; 
  
  if ( IsTimeToUpdate(SlowDataUpdated) ) {
    SetNextUpdate(SlowDataUpdated,SlowDataUpdatePeriod);

    SetN2kChargerStatus(N2kMsg, 1, 2, N2kCS_Absorption, N2kCM_Primary, N2kOnOff_On, N2kOnOff_On, 180);
    delay(DelayBetweenSend); NMEA2000.SendMsg(N2kMsg);

    SetN2kDCBatStatus(N2kMsg, 1, 12.72);
    delay(DelayBetweenSend); NMEA2000.SendMsg(N2kMsg);

    static unsigned char sid=0;
//    SetN2kDCBatStatus(N2kMsg, 0, 12.45, 5.08, CToKelvin(27.15));
    SetN2kPGN127508(N2kMsg, 0, 13.8, 0.95, N2kDoubleNA, sid++);
    delay(DelayBetweenSend); NMEA2000.SendMsg(N2kMsg);
    
    SetN2kBatConf(N2kMsg,1,N2kDCbt_AGM,N2kDCES_Yes,N2kDCbnv_12v,N2kDCbc_LeadAcid,AhToCoulomb(410),95,1.26,97);
    delay(DelayBetweenSend); NMEA2000.SendMsg(N2kMsg);
    
//    SetN2kDCStatus(N2kMsg,1,1,N2kDCt_Alternator,86,91,1420,0.215);
//    delay(DelayBetweenSend); NMEA2000.SendMsg(N2kMsg);

    NMEA2000.SendProductInformation();
    // Note that sometime NMEA Reader gets grazy, when ISO Address claim will be sent periodically.
    // If that happens, reopen NMEA Reader.
    NMEA2000.SendIsoAddressClaim();
  }
}

void SendN2kMisc() {
  static unsigned long SlowDataUpdated=InitNextUpdate(SlowDataUpdatePeriod,MiscSendOffset);
  tN2kMsg N2kMsg;
  
  // !delay is not good practise withing loops! I use it in this sample since
  // I tested Arduino Mega as receiver without inteerupt and it could not handle all messages
  // send without delay. With interrupt, it can do it and you can set DelayBetweenSend to 0.
  const int DelayBetweenSend=10; 
  
  if ( IsTimeToUpdate(SlowDataUpdated) ) {
    SetNextUpdate(SlowDataUpdated,SlowDataUpdatePeriod);

    SetN2kTransmissionParameters(N2kMsg,0,N2kTG_Forward,750000, CToKelvin(65.5),true,false,true);
    delay(DelayBetweenSend); NMEA2000.SendMsg(N2kMsg);

//AIS
    
// MesssageID set according to information found in https://github.com/ronzeiller/NMEA0183-AIS/blob/master/NMEA0183AISMessages.cpp
// För mer detaljer se https://www.navcen.uscg.gov/?pageName=AISMessagesA
// A Class A AIS unit broadcasts the following information every 2 to 10 seconds while underway, and every 3 minutes while at anchor at a power level of 12.5 watts.
// MessageID 1, 2 or 3

  SetN2kAISClassAPosition(N2kMsg, 1, tN2kAISRepeat::N2kaisr_First, 123456789, 55.577521, 12.816374, 1, 1, 1, DegToRad(20), 23,DegToRad(30), DegToRad(-10), tN2kAISNavStatus::N2kaisns_Under_Way_Motoring);  // MessageID=1, 23 m/s SOG
  delay(DelayBetweenSend); NMEA2000.SendMsg(N2kMsg);
 
  SetN2kAISClassAPosition(N2kMsg, 1, tN2kAISRepeat::N2kaisr_First, 706647389, 55.675523, 13.056202, true, true, 1, DegToRad(2), 20, DegToRad(30), DegToRad(10), tN2kAISNavStatus::N2kaisns_At_Anchor);  // Martin parkerad på piren
  delay(DelayBetweenSend); NMEA2000.SendMsg(N2kMsg);

// Daniel: Borde fungera och ge information kring bla namn
// In addition, the Class A AIS unit broadcasts the following information every 6 minutes. 

  SetN2kAISClassAStatic(N2kMsg, 5, tN2kAISRepeat::N2kaisr_First,     // MessageID=5
    123456789, 9999999, "ABCDEF",
    "MAGIC SIMPLY", 37, 8.0, 3.0, 4.0, 2.0,   // 37 Pleasure craft
    18250, 43200.0, 2.0, "LOMMA", // ETAdate=50*365 , ETAtime=12*60*60, 2.0, “LOMMA”,
    tN2kAISVersion::N2kaisv_ITU_R_M_1371_1,
    tN2kGNSStype::N2kGNSSt_GPS,
    tN2kAISDTE::N2kaisdte_Ready,
    tN2kAISTranceiverInfo::N2kaisti_Channel_B_VDL_transmission);
    delay(DelayBetweenSend); NMEA2000.SendMsg(N2kMsg);

   SetN2kAISClassAStatic(N2kMsg, 5, tN2kAISRepeat::N2kaisr_First,     // MessageID=5
    706647389, 7654321, "SE+46",
    "CAMOMILLA", 37, 8.6, 3.0, 4.3, 1.5,   // 37 Pleasure craft
    18250, 43200.0, 2.0, "LOMMA", // ETAdate=50*365 , ETAtime=12*60*60, 2.0, “Lomma”,
    tN2kAISVersion::N2kaisv_ITU_R_M_1371_1,
    tN2kGNSStype::N2kGNSSt_GPS,
    tN2kAISDTE::N2kaisdte_Ready,
    tN2kAISTranceiverInfo::N2kaisti_Channel_B_VDL_transmission);
    delay(DelayBetweenSend); NMEA2000.SendMsg(N2kMsg);

// Daniel: Denna borde fungera för position för Class B
// https://www.navcen.uscg.gov/?pageName=AISMessagesB

  SetN2kAISClassBPosition(N2kMsg, 18, tN2kAISRepeat::N2kaisr_First, 705686783, 55.675500, 13.068821, true, true, 1, DegToRad(90), 4, DegToRad(180), tN2kAISUnit::N2kaisunit_ClassB_SOTDMA, true, false, false, true, tN2kAISMode::N2kaismode_Autonomous, true); // Daniel i gästhamnen
// finns även MESSAGE 19: EXTENDED CLASS B EQUIPMENT POSITION REPORT
  delay(DelayBetweenSend); NMEA2000.SendMsg(N2kMsg);

// Daniel: Dessa går att testa men jag tror det finns en bug i timos bibliotek for MessageID=24 som gör att de inte kommer funka
  //SetN2kAISClassBStaticPartA(N2kMsg, 24, tN2kAISRepeat::N2kaisr_First, 705686783,  "SE+46705686783");
  //delay(DelayBetweenSend); NMEA2000.SendMsg(N2kMsg);

  //SetN2kAISClassBStaticPartB(N2kMsg, 24, tN2kAISRepeat::N2kaisr_First, 705686783, 36, "MAXI", "SE+46705686783", 8.0, 3.0, 4.0, 2.0, 826510421);  // Simply Magic MMSI: 826510421, Call sign SF2692
  //delay(DelayBetweenSend); NMEA2000.SendMsg(N2kMsg);

  SetN2kAISClassBStaticPartA(N2kMsg, 24, tN2kAISRepeat::N2kaisr_First, 705686783,  "SIMPLY MAGIC");
  delay(DelayBetweenSend); NMEA2000.SendMsg(N2kMsg);

  SetN2kAISClassBStaticPartB(N2kMsg, 24, tN2kAISRepeat::N2kaisr_First, 705686783, 36, "MAXI", "SF2692", 8.0, 3.0, 4.0, 2.0, 826510421);  // Simply Magic MMSI: 826510421, Call sign SF2692
  delay(DelayBetweenSend); NMEA2000.SendMsg(N2kMsg);
   
    //AIS

    SetN2kBinaryStatus(N2kMsg,2,N2kOnOff_On,N2kOnOff_Unavailable,N2kOnOff_Off);
    delay(DelayBetweenSend); NMEA2000.SendMsg(N2kMsg);

    tN2kBinaryStatus SwitchBoard;
    N2kResetBinaryStatus(SwitchBoard);
    N2kSetStatusBinaryOnStatus(SwitchBoard,N2kOnOff_On,7);
    SetN2kBinaryStatus(N2kMsg,3,SwitchBoard);
    delay(DelayBetweenSend); NMEA2000.SendMsg(N2kMsg);

    SetN2kEngineDynamicParam(N2kMsg,0,656000,CToKelvin(86.3),CToKelvin(82.1),14.21,5.67,hToSeconds(2137.55),N2kDoubleNA,N2kDoubleNA,N2kInt8NA,N2kInt8NA,true);
    delay(DelayBetweenSend); NMEA2000.SendMsg(N2kMsg);
    SetN2kEngineTripParameters(N2kMsg,0,237,3.6,4.5,4.4);
    delay(DelayBetweenSend); NMEA2000.SendMsg(N2kMsg);
    
  }
}

#define RapidDataUpdatePeriod 100 //167 // Some strange periot to cause Slow and rapid to run unsync.

void SendN2kRapidData() {
  static unsigned long RapidDataUpdated=InitNextUpdate(RapidDataUpdatePeriod);
  tN2kMsg N2kMsg;
  static double Heading=227.5;

  if ( IsTimeToUpdate(RapidDataUpdated) ) {
    SetNextUpdate(RapidDataUpdated,RapidDataUpdatePeriod);
    SetN2kEngineParamRapid(N2kMsg,0,4350); //,820000,48);
    NMEA2000.SendMsg(N2kMsg);
    SetN2kCOGSOGRapid(N2kMsg,1,N2khr_true,DegToRad(115.6),0.1);
    NMEA2000.SendMsg(N2kMsg);
    SetN2kMagneticHeading(N2kMsg, 0, DegToRad(Heading), DegToRad(-3.0), DegToRad(5.5)); 
    NMEA2000.SendMsg(N2kMsg);
    //Heading+=0.1; if (Heading>=360.0 ) Heading-=360.0;

  }
}
