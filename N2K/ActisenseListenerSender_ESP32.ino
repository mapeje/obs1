// From https://github.com/ttlappalainen/NMEA2000/tree/master/Examples/ActisenseListenerSender
// Demo: NMEA2000 library. Bus listener and sender. 
//   Sends all bus data to serial in Actisense format.
//   Send all data received from serial in Actisense format to the N2kBus.
//   Use this e.g. with NMEA Simulator (see. http://www.kave.fi/Apps/index.html) to send simulated data to the bus.

//   Use with Openplotter: 1. Connect ESP to RPI/OP with USB; 2. Add serial device as NMEA2000 and Add to CAN bus with 115000 baud. 
//   3. N2K should come into SignalK. 4. Use signalk-to-nmea2000 plugin to send N2K messages back to NMEA network. 5. Use 
//   @signalk/simulatorplugin to simulate data ("environment.water.temperature" from "source: simulator.0"
//   Make sure using "NMEA 2000 Source: Actisense NGT-1 (canboatjs)" in SignalK

//   other stream to different port so that you can send data with NMEA Simulator and listen it on other port with 
//   Actisense NMEA Reader.

// Added ESP32 definition
#define ESP32_CAN_RX_PIN GPIO_NUM_15
#define ESP32_CAN_TX_PIN GPIO_NUM_2

#define N2k_CAN_INT_PIN 21
#include <Arduino.h>
#include <N2kMsg.h>
#include <NMEA2000.h>
#include <NMEA2000_CAN.h>
#include <ActisenseReader.h>


tActisenseReader ActisenseReader;

// Define READ_STREAM to port, where you write data from PC e.g. with NMEA Simulator.
#define READ_STREAM Serial       
// Define ForwardStream to port, what you listen on PC side. On Arduino Due you can use e.g. SerialUSB
#define FORWARD_STREAM Serial    

Stream *ReadStream=&READ_STREAM;
Stream *ForwardStream=&FORWARD_STREAM;

void setup() {
  // Define buffers big enough
  NMEA2000.SetN2kCANSendFrameBufSize(150);
  NMEA2000.SetN2kCANReceiveFrameBufSize(150);
  
  if (ReadStream!=ForwardStream) READ_STREAM.begin(115200);
  FORWARD_STREAM.begin(115200);
  NMEA2000.SetForwardStream(ForwardStream); 
  NMEA2000.SetMode(tNMEA2000::N2km_ListenAndSend);
  // NMEA2000.SetForwardType(tNMEA2000::fwdt_Text); // Show bus data in clear text
  if (ReadStream==ForwardStream) NMEA2000.SetForwardOwnMessages(false); // If streams are same, do not echo own messages.
  // NMEA2000.EnableForward(false);
  NMEA2000.Open();

  // I originally had problem to use same Serial stream for reading and sending.
  // It worked for a while, but then stopped. Later it started to work.
  ActisenseReader.SetReadStream(ReadStream);
  ActisenseReader.SetDefaultSource(75);
  ActisenseReader.SetMsgHandler(HandleStreamN2kMsg); 
}

void HandleStreamN2kMsg(const tN2kMsg &N2kMsg) {
  // N2kMsg.Print(&Serial);
  NMEA2000.SendMsg(N2kMsg,-1);
}

void loop() {
  NMEA2000.ParseMessages();
  ActisenseReader.ParseMessages();
}
