// ####################################################################################################################
// 
// Code part of CarCluster project by Andrej Rolih. See .ino file more details
// 
// ####################################################################################################################

#ifndef MQB_DASH
#define MQB_DASH

#include "../../Libs/MCP_CAN/mcp_can.h" // CAN Bus Shield Compatibility Library ( https://github.com/coryjfowler/MCP_CAN_lib )
#include "../../Libs/X9C10X/X9C10X.h" // For fuel level simulation ( https://github.com/RobTillaart/X9C10X )

#include "../Cluster.h"

// Known CAN IDs
#define AIRBAG_01_ID 0x40
#define KLEMMEN_STATUS_01_ID 0x3C0 // Ignition status
#define DIMMUNG_01_ID 0x5F0 // Backlight
#define GATEWAY_72_ID 0x3db
#define GATEWAY_76_ID 0x3df
#define ESP_02_ID 0x101
#define ESP_10_ID 0x116
#define ESP_20_ID 0x65D // tire circumference
#define ESP_21_ID 0xFD // speed/distance
#define KOMBI_01_ID 0x30B // generated by kombi
#define MOTOR_04_ID 0x107 // RPM
#define MOTOR_07_ID 0x640 // oil temp
#define MOTOR_09_ID 0x647 // coolant temp
#define MOTOR_14_ID 0x3BE
#define MOTOR_18_ID 0x670
#define MOTOR_26_ID 0x3c7
#define MOTOR_CODE_01_ID 0x641
#define ESP_24_ID 0x31B // speed on kombi
#define TSK_07_ID 0x31E
#define LH_EPS_01_ID 0x32A
#define RKA_01_ID 0x663
#define OBD_01_ID 0x391
#define WBA_03_ID 0x394 // gear position
#define BLINKMODI_02_ID 0x366 // Blinkers
#define MFSW_ID 0x5BF // MultiFunction Steering Wheel
#define TPMS_ID 0x64A  // TPMS. ID 0x5f9 is also somehow connected to TPMS (playing with values there can show TPMS display)
#define SWA_01_ID 0x30F // Lane Change assist (SpurWechselAssistent)?
#define PARKBRAKE_ID 0x30d // Electronic parking brake
#define LWR_AFS_01 0x395 // Something to do with lights?
#define ESP_05_ID 0x106
#define LICHT_VORNE_01_ID 0x658 // Lights front
#define LICHT_HINTEN_01_ID 0x3D6 // Lights rear
#define LICHT_ANF_ID 0x3D5 // Lights... somewhere
#define DOOR_STATUS_ID 0x583 // Door status
#define OUTDOOR_TEMP_ID 0x5e1 // Outdoor temperature
#define DATE_ID 0x17331100 // From car radio to cluster
// WARNING: NEVER TOUCH ADDRESS 0x6B4 !!!!! This is part of component protection/VIN

class VWMQBCluster: public Cluster {
  public:
  static ClusterConfiguration clusterConfig() {
    ClusterConfiguration config;
    config.minimumCoolantTemperature = 50;
    config.maximumCoolantTemperature = 130;
    config.maximumSpeedValue = 260;
    config.maximumRPMValue = 8000;
    config.minimumFuelPotValue = 18;
    config.maximumFuelPotValue = 83;
    config.minimumFuelPot2Value = 17;
    config.maximumFuelPot2Value = 75;
    config.isDualFuelPot = true;

    return config;
  }

  VWMQBCluster(MCP_CAN& CAN, int fuelPotIncPin, int fuelPotDirPin, int fuelPot1CsPin, int fuelPot2CsPin, bool passthroughMode = false);
  void updateWithGame(GameState& game);
  void updateTestBuffer(uint8_t val0, uint8_t val1, uint8_t val2, uint8_t val3, uint8_t val4, uint8_t val5, uint8_t val6, uint8_t val7);
  void sendSteeringWheelControls(int button);
  void sendTestBuffers();

  void handleReceivedData(long unsigned int canRxId, unsigned char canRxLen, unsigned char canRxBuf[]);

  private:
    MCP_CAN &CAN;
    X9C102 fuelPot = X9C102();
    X9C102 fuelPot2 = X9C102();

    // For passthrough mode
    bool passthroughMode = false;

    unsigned long dashboardUpdateTime50 = 50;
    unsigned long dashboardUpdateTime500 = 500;
    unsigned long lastDashboardUpdateTime = 0; // Timer for the fast updated variables - like ABS/speed/RPM
    unsigned long lastDashboardUpdateTime500ms = 0; // Timer for slow updated variables - like TPMS

    int turning_lights_counter = 0;

    unsigned char i = 0, seq = 0, crc;
    unsigned long rpmVal, vSpeed, esp24Speed, esp24Inc, esp24Distance = 0, prevTime = 0;
    unsigned char esp24Overflow = 0;

    // Buffers
    unsigned char kStatusBuf[4] = { 0x00, 0x00, 0x03, 0x00 },
                  airbag01Buf[8] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },
                  gateway72Buf[8] = { 0x50, 0x80, 0x00, 0x00, 0x07, 0x10, 0x01, 0x8C },
                  gateway76Buf[8] = { 0x04, 0x91, 0x00, 0x28, 0x15, 0x00, 0x00, 0x00 },
                  dimmungBuf[8] = { 0xFD, 0x00, 0x64, 0x00, 0x00, 0x59, 0x00, 0x00 },
                  esp20Buf[8] = { 0x00, 0x30, 0x2B, 0x12, 0x00, 0x00, 0xB4, 0x79 },
                  esp21Buf[8] = { 0x00, 0xD0, 0x1F, 0x80, 0xd8, 0x0d, 0x00, 0x00 },              
                  esp24Buf[8] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00 },
                  tsk07Buf[8] = { 0xCA, 0xEF, 0x3F, 0x00, 0x00, 0x00, 0x00, 0x40 },
                  lhEps01Buf[8] = { 0x4B, 0x08, 0x00, 0x00, 0x02, 0x02, 0x00, 0x00 },
                  rka01Buf[8] = { 0x60, 0x28, 0x20, 0x07, 0x0D, 0xBC, 0x2C, 0x00 },
                  obd01Buf[8] = { 0x2A, 0x89, 0x22, 0x2C, 0x00, 0x26, 0x00, 0x20 },
                  wba03Buf[8] = { 0x29, 0x40, 0x00, 0x04, 0x00, 0x04, 0x00, 0x00 },
                  motor04Buf[8] = { 0x00, 0x00, 0x00, 0xE8, 0x03, 0x00, 0x00, 0x00 },
                  motor07Buf[8] = { 0x00, 0x84, 0x99, 0xBE, 0x74, 0x20, 0x05, 0x20 },
                  motor09Buf[8] = { 0xB8, 0xFD, 0xFF, 0x7F, 0x00, 0x00, 0x00, 0xC1 },
                  motor14Buf[8] = { 0x00, 0x30, 0xE7, 0x51, 0x88, 0xC0, 0x0C, 0x00 },
                  motor18Buf[8] = { 0x00, 0x00, 0x00, 0x00, 0xFE, 0x00, 0x00, 0x80 },
                  motor26Buf[8] = { 0xFD, 0x10, 0x28, 0x00, 0x00, 0x40, 0x80, 0x00 },
                  mfswBuf[4] = { 0x00, 0x00, 0x00, 0x40 },
                  motorCode01Buf[8] = { 0x00, 0x00, 0x00, 0xE8, 0x03, 0x00, 0x00, 0x00 },
                  blinkerBuff[8] = { 0x00, 0x00, 0x04, 0x14, 0x0A, 0xFC, 0x00, 0x00 },
                  tpmsBuff[8] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },
                  swa01Buff[8] = { 0x50, 0x05, 0x30, 0x00, 0x00, 0x00, 0x00, 0x00 },
                  parkBrakeBuff[4] = { 0x0, 0x00, 0x0, 0x0 },
                  lichtVorne01Buff[8] = { 0x00, 0x00, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00 },
                  doorStatusBuff[8] = { 0x00, 0x10, 0x05, 0x00, 0x00, 0x44, 0x55, 0x00 },
                  outdoorTempBuff[8] = { 0x9A, 0x2A, 0x00, 0x60, 0xFE, 0x00, 0x00, 0x00 },
                  lichtAnfBuff[8] = { 0x00, 0x04, 0xC0, 0x00, 0x00, 0x00, 0x00, 0x00 },
                  lichtHintenBuff[8] = { 0x00, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },
                  testBuff[8] = { 0x04, 0x06, 0x40, 0x00, 0xFF, 0xFE, 0x69, 0x2C },
                  timeBuff[5]= { 0x24, 0x51, 0x00, 0x00, 0x00 },
                  dateBuff[5]= { 0x24, 0x50, 0x00, 0x00, 0x00 };

    void sendIgnitionStatus(boolean ignition);
    void sendBacklightBrightness(uint8_t brightness);
    void sendESP20();
    void sendESP21(int speed);
    void sendTSK07();
    void sendLhEPS01();
    void sendMotor(int rpm, int coolantTemperature);
    void sendESP24();
    void sendGear(uint8_t gear);
    void sendAirbag01();
    void sendBlinkers(boolean leftTurningIndicator, boolean rightTurningIndicator, boolean turningIndicatorsBlinking);
    void sendTPMS();
    void sendSWA01();
    void sendParkBrake(boolean handbrakeActive);
    void sendLights(boolean highBeam, boolean rearFogLight);
    void sendDoorStatus(boolean doorOpen);
    void sendOutdoorTemperature(int temperature);
    void sendOtherLights();
    void sendTime(uint8_t clockHour, uint8_t clockMinute);
    void sendDate(uint8_t clockYear, uint8_t clockMonth, uint8_t clockDay);
    
    void setFuel(GameState& game);
    uint8_t mapGenericGearToLocalGear(GearState inputGear);
    int mapSpeed(GameState& game);
    int mapRPM(GameState& game);
    int mapCoolantTemperature(GameState& game);
};

#endif