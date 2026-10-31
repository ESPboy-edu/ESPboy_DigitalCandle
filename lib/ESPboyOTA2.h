/*
ESPboyOTA2 class AppStore (OTA2) v7_2
www.ESPboy.com
Proprietary license 2021 (C) RomanS espboy.edu@gmail.com
*/

#ifndef ESPboy_OTA
#define ESPboy_OTA

#define OTAv 7.2

#include <FS.h> 
using fs::FS;
#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include "ESPboyTerminalGUI.h"

#define OTA_TIMEOUT_CONNECTION 10000

#define PAD_LEFT        0x01
#define PAD_UP          0x02
#define PAD_DOWN        0x04
#define PAD_RIGHT       0x08
#define PAD_ACT         0x10
#define PAD_ESC         0x20
#define PAD_LFT         0x40
#define PAD_RGT         0x80
#define PAD_ANY         0xff

// ОПТИМИЗАЦИЯ: Возвращены макросы для использования совместно с F()
#define GOOGL_URL_START "https://script.google.com/macros/s/"
#define GOOGL_URL_END "/exec"
#define GOOGL_LOG_URL "AKfycbyK38VudmWzFLzQCXpCor6dbjUaVdJyLs678DYUQw"
#define GOOGL_USER_URL "AKfycbz5Pzjixa7cyDsA-ML9KTE9YNg8BkMb90Y6OT45J3UUNWqY2uVm"
#define GOOGL_MESS_URL "AKfycbzK7UH3YJb9w27dWCNLqrMKzMjDAAzPY1z1lZGspKTrEFaHDpbp"
#define GOOGL_LIST_URL "AKfycbwKMaHpDg_a52oSNdlbO9q1XEDlFMRVbFdKrarnp0HyHoCAGak"
#define GOOGL_LIKE_URL "AKfycbySuuY9VLKanprDx9N7kBLiQE1kk3TIXh28GC4iP22FLz0WjP4"
#define GOOGL_DOWNLOAD_URL "https://drive.google.com/uc?export=download&id=" 

struct wificlient{
  String ssid;
  String pass;
};


struct wf {
    String ssid;
    uint8_t rssi;
    uint8_t encription;
};


struct lessRssi{
    inline bool operator() (const wf& str1, const wf& str2) {return (str1.rssi > str2.rssi);}
};


struct iL {
  uint32_t iData;
  int8_t iAccsess;
  int16_t iLike;
  String iItem;
};


struct globalStruct{
  uint8_t accessUser;
  uint32_t userLineNo;
  uint32_t appLineNo;
  uint32_t lastVisitTimestamp;
  String nicknameUser;
  String type;
  String genre;
  String app;
  String updlink;
};


struct messStruct{
 String   messmess;
 uint16_t messcolor;
 uint32_t messdelay;
}; 

enum GET_CMD {GET_TYPE=1, GET_GENRE, GET_APP, GET_INFO, GET_MESS, GET_USER, GET_UPDATE, GET_LIKE};
enum POST_CMD {POST_CONNECT=1, POST_LIKE, POST_DOWNLOAD, POST_LOG_LIKE};
enum RESULT_STATE {FAULT_STATE=0, OK_STATE, BACK_STATE, LIKE_STATE};
enum MESS {START_MESS=0, USERFOUND_MESS, USERNOTFOUND_MESS, BEFOREUPDATE_MESS, UPDATEFINISHWD_MESS};

class ESPboyOTA2{
  private:
    ESPboyTerminalGUI *terminalGUIobj = NULL;
    wificlient wificl;
    WiFiClientSecure client;
    HTTPClient http;
    
    std::vector<wf> wfList; 
    std::vector<iL> iList;

    GET_CMD GET_cmd;
    POST_CMD POST_cmd;
    RESULT_STATE GET_result;
    messStruct messStr[5];  
    globalStruct globalStr;

    const static char* PROGMEM finalMnu[];
    const static char* PROGMEM itemBck;
  
    void OTAprogress(int cur, int total);
    String GETmethod(GET_CMD GETcmd);
    String POSTmethod(POST_CMD POST_cmd);
    String fillPayload(String linkField);
    String getWiFiStatusName();
    uint16_t scanWiFi();
    bool wifiConnect();
    bool UPDATEmethod();
    RESULT_STATE get_mess();
    RESULT_STATE get_user();
    RESULT_STATE get_type();
    RESULT_STATE get_genre();
    RESULT_STATE get_app();
    RESULT_STATE get_info();
    
public:
   ESPboyOTA2(ESPboyTerminalGUI* GUIobjOTA);
   void checkOTA();
};

#endif