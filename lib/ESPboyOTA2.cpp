/*
ESPboyOTA2 class AppStore (OTA2) v7_2
www.ESPboy.com
Proprietary license 2021 (C) RomanS espboy.edu@gmail.com
*/

#include "ESPboyOTA2.h"

const char* PROGMEM ESPboyOTA2::finalMnu[] = {"1 DOWNLOAD", "2 LIKE", "3 BACK"};
// ОПТИМИЗАЦИЯ: Стандартная нуль-терминированная строка (избавились от лишних символов "\0x0")
const char* PROGMEM ESPboyOTA2::itemBck = "BACK"; 

ESPboyOTA2::ESPboyOTA2(ESPboyTerminalGUI* GUIobjOTA) {    
   terminalGUIobj = GUIobjOTA;
   terminalGUIobj->toggleDisplayMode(1);
   WiFi.persistent(true);
   client.setInsecure();
   http.setFollowRedirects(HTTPC_STRICT_FOLLOW_REDIRECTS);
   http.setTimeout(OTA_TIMEOUT_CONNECTION);
   checkOTA();
}

void ESPboyOTA2::OTAprogress(int cur, int total) {
  terminalGUIobj->printConsole((String)(cur * 100 / total) + "%", TFT_GREEN, 1, 1);
}

bool ESPboyOTA2::UPDATEmethod() {
 uint8_t header[4];
 int size;
    // ОПТИМИЗАЦИЯ: Использование макроса F() напрямую
    if (http.begin(client, String(F(GOOGL_DOWNLOAD_URL)) + globalStr.updlink)) {
      if (http.GET() == HTTP_CODE_OK) {
        size = http.getSize();
        WiFiClient *tcp = http.getStreamPtr();
        WiFiClient::stopAllExcept(tcp);
        if (tcp->peekBytes(header, sizeof(header)) == sizeof(header))
          if (header[0] == 0xE9)
            if (Update.begin(size, U_FLASH, LED_BUILTIN, LOW)) {
              Update.onProgress([this](int a, int b){this->OTAprogress(a,b);});
              if (Update.writeStream(*tcp) == (size_t)size)
                if (Update.end()) 
                  return (true);
            }
      }
    http.end();
    }
  return (false);
}

String ESPboyOTA2::GETmethod(GET_CMD GETcmd){
 String response;
 String urlstr; 
 // ОПТИМИЗАЦИЯ: Исключение промежуточных копий String путем резервирования и каскадного использования +=
 urlstr.reserve(150); 
 urlstr = F(GOOGL_URL_START);

  switch (GETcmd){
    case GET_USER: urlstr += F(GOOGL_USER_URL); break;
    case GET_MESS: urlstr += F(GOOGL_MESS_URL); break;
    default:       urlstr += F(GOOGL_LIST_URL); 
  }
  
  urlstr += F(GOOGL_URL_END);
  urlstr += F("?cmd=");
  urlstr += GETcmd;
  urlstr += F("&dat=");

  switch (GETcmd){
    case GET_USER:
      urlstr += (String)ESP.getChipId();
      urlstr += ';';
      urlstr += WiFi.macAddress();
      break;
    case GET_GENRE:
      urlstr += globalStr.type;
      break;
    case GET_APP:
      urlstr += globalStr.genre;
      break;
    case GET_INFO:
      urlstr += globalStr.app;
      break;
 }
  
  if (http.begin(client, urlstr)) {
    if (http.GET() == HTTP_CODE_OK) {
      response = http.getString();
    }
   http.end();
  }
 
 return (response);
}
  
String ESPboyOTA2::POSTmethod(POST_CMD POST_cmd){
 String postData;
 String response;
 String toPrint;
 String urlstr;

 // ОПТИМИЗАЦИЯ: Предварительное выделение памяти для URL
 urlstr.reserve(150);
 urlstr = F(GOOGL_URL_START);

  switch(POST_cmd){
    case POST_CONNECT:
      urlstr += F(GOOGL_LOG_URL);
      toPrint = F("CONNECT ");
      toPrint += globalStr.nicknameUser;
      postData = fillPayload(toPrint);
      break;
    case POST_LIKE:
      urlstr += F(GOOGL_LIKE_URL);
      postData = globalStr.nicknameUser + ';' + globalStr.app;
      break;
    case POST_LOG_LIKE:
      urlstr += F(GOOGL_LOG_URL);
      toPrint = F("LIKE ");
      toPrint += globalStr.app;
      postData = fillPayload(toPrint);
      break;
    case POST_DOWNLOAD:
      urlstr += F(GOOGL_LOG_URL);
      toPrint = F("DOWNLOAD ");
      toPrint += globalStr.app;
      postData = fillPayload(toPrint);
      break;
  }

  urlstr += F(GOOGL_URL_END);

  if (http.begin(client, urlstr)) {
    if (http.POST(postData) == HTTP_CODE_OK) {
      response = http.getString();
      http.end();
    }
  }
 return (response);
}

String ESPboyOTA2::fillPayload(String linkField) {
  String payload = "";
  // ОПТИМИЗАЦИЯ: резервируем блок памяти (256 байт) для предотвращения дефрагментации Heap
  payload.reserve(256); 
  
  payload += WiFi.macAddress();   // MAC address
  payload += F("; ");  // date/time
  payload += F("; "); 
  payload += WiFi.localIP().toString();
  payload += F("; "); 
  payload += linkField;  // download name
  payload += F("; "); 
  payload += (String)ESP.getFreeHeap();
  payload += F("; "); 
  payload += (String)ESP.getFreeContStack();
  payload += F("; "); 
  payload += (String)ESP.getChipId();
  payload += F("; "); 
  payload += (String)ESP.getFlashChipId();
  payload += F("; "); 
  payload += (String)ESP.getCoreVersion();
  payload += F("; "); 
  payload += (String)ESP.getSdkVersion();
  payload += F("; "); 
  payload += (String)ESP.getCpuFreqMHz();
  payload += F("; "); 
  payload += (String)ESP.getSketchSize();
  payload += F("; "); 
  payload += (String)ESP.getFreeSketchSpace();
  payload += F("; "); 
  payload += (String)ESP.getSketchMD5();
  payload += F("; "); 
  payload += (String)ESP.getFlashChipSize();
  payload += F("; "); 
  payload += (String)ESP.getFlashChipRealSize();
  payload += F("; "); 
  payload += (String)ESP.getFlashChipSpeed();
  payload += F("; ");
  payload += (String)ESP.getCycleCount();
  payload += F("; "); 
  payload += WiFi.SSID();
  payload += F("; "); 
  payload += OTAv;
  payload += F("; ");
  payload += globalStr.accessUser;
  payload += F("; ");
  payload += globalStr.userLineNo;
  payload += F("; ");
  payload += globalStr.appLineNo;
  
  return (payload);
}

uint16_t ESPboyOTA2::scanWiFi() {
  terminalGUIobj->printConsole(F("Scaning WiFi..."), TFT_MAGENTA, 1, 0);
  int16_t WifiQuantity = WiFi.scanNetworks();
  
  if (WifiQuantity != -1 && WifiQuantity != -2 && WifiQuantity != 0) {
    wfList.reserve(WifiQuantity); // ОПТИМИЗАЦИЯ: резервируем память
    
    for (uint8_t i = 0; i < WifiQuantity; i++) {
      // ОПТИМИЗАЦИЯ: инициализируем структуру прямо при добавлении в вектор
      wfList.push_back({WiFi.SSID(i), (uint8_t)WiFi.RSSI(i), (uint8_t)WiFi.encryptionType(i)});
      delay(0);
    }
    
    sort(wfList.begin(), wfList.end(), lessRssi());
    return (WifiQuantity);
  } else {
    terminalGUIobj->printConsole(F("WiFi not found"), TFT_RED, 1, 0);
    delay(3000);
    ESP.restart(); // ОПТИМИЗАЦИЯ: restart() безопаснее reset()
    return (0);
  }
}

bool ESPboyOTA2::wifiConnect() {
 uint16_t wifiNo = 0;
 uint32_t timeOutTimer;
 static uint8_t connectionErrorFlag = 0;
  
  if (!connectionErrorFlag && !(terminalGUIobj->getKeys()&PAD_ESC)) {
    wificl.ssid = WiFi.SSID();
    terminalGUIobj->printConsole(F("Last network:"), TFT_MAGENTA, 0, 0);
    terminalGUIobj->printConsole(wificl.ssid, TFT_GREEN, 0, 0);
    WiFi.mode(WIFI_STA);
    WiFi.begin();
  } 
  else 
  {
      wificl.ssid = "";
      wificl.pass = "";
    
    if (scanWiFi()) {
      for (uint8_t i = wfList.size(); i > 0; i--) {
        // ОПТИМИЗАЦИЯ: Локальный буфер на стеке вместо медленной конкатенации String
        char buf[64];
        snprintf(buf, sizeof(buf), "%d %s %d%s", 
                 i, 
                 wfList[i - 1].ssid.c_str(), 
                 wfList[i - 1].rssi, 
                 (wfList[i - 1].encription == ENC_TYPE_NONE) ? "" : "*");
        terminalGUIobj->printConsole(String(buf), TFT_YELLOW, 0, 0);
      }
    }

    while (!wifiNo) {
      terminalGUIobj->printConsole(F("Choose WiFi No:"), TFT_MAGENTA, 0, 0);
      wifiNo = terminalGUIobj->getUserInput().toInt();
      if (wifiNo < 1 || wifiNo > wfList.size()) wifiNo = 0;
    }

    wificl.ssid = wfList[wifiNo - 1].ssid;
    terminalGUIobj->printConsole(wificl.ssid, TFT_GREEN, 1, 0);

    while (!wificl.pass.length()) {
      terminalGUIobj->printConsole(F("Password:"), TFT_MAGENTA, 0, 0);
      wificl.pass = terminalGUIobj->getUserInput();
    }
    terminalGUIobj->printConsole(/*pass*/F("******"), TFT_GREEN, 0, 0);
  
    wfList.clear();
    wfList.shrink_to_fit();
  
    WiFi.mode(WIFI_STA);
    WiFi.begin(wificl.ssid, wificl.pass);
  }

  terminalGUIobj->printConsole(F("Connection..."), TFT_MAGENTA, 0, 0);
  timeOutTimer = millis();
  String dots = "";
  while (WiFi.status() != WL_CONNECTED &&
         (millis() - timeOutTimer < OTA_TIMEOUT_CONNECTION)) {
    delay(700);
    terminalGUIobj->printConsole(dots, TFT_MAGENTA, 0, 1);
    dots += ".";
    if (dots.length()>20) dots=".";
  }

  if (WiFi.status() != WL_CONNECTED) {
    connectionErrorFlag = 1;
    terminalGUIobj->printConsole(getWiFiStatusName(), TFT_RED, 0, 1);
    delay(1000);
    terminalGUIobj->printConsole("", TFT_BLACK, 0, 0);
    return (false);
  } else {
    terminalGUIobj->printConsole(getWiFiStatusName(), TFT_MAGENTA, 0, 1);
    return (true);
  }
}

String ESPboyOTA2::getWiFiStatusName() {
  String stat;
  switch (WiFi.status()) {
    case WL_IDLE_STATUS:
      stat = (F("Idle"));
      break;
    case WL_NO_SSID_AVAIL:
      stat = (F("No SSID available"));
      break;
    case WL_SCAN_COMPLETED:
      stat = (F("Scan completed"));
      break;
    case WL_CONNECTED:
      stat = (F("WiFi connected"));
      break;
    case WL_CONNECT_FAILED:
      stat = (F("Wrong passphrase"));
      break;
    case WL_CONNECTION_LOST:
      stat = (F("Connection lost"));
      break;
    case WL_DISCONNECTED:
      stat = (F("Wrong password"));
      break;
    default:
      stat = (F("Unknown"));
      break;
  };
  return stat;
}

RESULT_STATE ESPboyOTA2::get_mess(){
 String resp="";
 char *pch;
  
  resp = GETmethod(GET_MESS);
  
  // ОПТИМИЗАЦИЯ: Защищенный от утечек парсинг строки
  char* buffer = new char[resp.length() + 1];
  resp.toCharArray(buffer, resp.length() + 1);
  pch = strtok(buffer, ";");
  
  if((String)pch != "0"){
    delete[] buffer;
    return(FAULT_STATE);
  }

  for (uint8_t i=0; i<5; i++){
    pch = strtok (NULL,";");
    if(pch) messStr[i].messmess = pch; 
    pch = strtok (NULL,";");
    if(pch) messStr[i].messcolor = atoi(pch);
    pch = strtok (NULL,";");
    if(pch) messStr[i].messdelay = atol(pch);
  }
  
  delete[] buffer;
  return (OK_STATE);
};
  
RESULT_STATE ESPboyOTA2::get_user(){
 String resp="";
 char *pch;

  resp = GETmethod(GET_USER); 
  
  char* buffer = new char[resp.length() + 1];
  resp.toCharArray(buffer, resp.length() + 1);
  pch = strtok (buffer,";");

  if((String)pch == "0"){
    pch = strtok (NULL,";");
    if (pch) globalStr.nicknameUser = pch;
    pch = strtok (NULL,";");
    if (pch) globalStr.accessUser = atoi(pch);
    pch = strtok (NULL,";");
    if (pch) globalStr.userLineNo = atoi(pch);
    pch = strtok (NULL,";");
    if (pch) globalStr.lastVisitTimestamp = atoi(pch);
    globalStr.appLineNo = 0;
    delete[] buffer;
    return(OK_STATE); 
  }

  if((String)pch == "2" || (String)pch == "3"){
    globalStr.nicknameUser = F("Unknown");
    globalStr.accessUser = 0;
    globalStr.userLineNo = 0;
    delete[] buffer;
    return(OK_STATE);
  }

  delete[] buffer;
  return(FAULT_STATE);
};

RESULT_STATE ESPboyOTA2::get_type(){
 String resp;
 char *pch;
 uint16_t itemsQuant = 0;
 uint16_t selectedItem;
 char buf[64];

  terminalGUIobj->printConsole(F(""), TFT_BLACK, 1, 0);
  terminalGUIobj->printConsole(F("TYPEs update"), TFT_MAGENTA, 1, 0);
  
  resp = GETmethod(GET_TYPE);
  
  char* buffer = new char[resp.length() + 1];
  resp.toCharArray(buffer, resp.length() + 1);
  pch = strtok (buffer,";");
  
  if((String)pch != "0"){
      delete[] buffer;
      return(FAULT_STATE);
  }

  pch = strtok (NULL,";");
  if (pch) itemsQuant = atoi(pch);
  
  iList.reserve(itemsQuant); // ОПТИМИЗАЦИЯ
  
  for (uint16_t i=0; i<itemsQuant; i++){
    yield();
    iList.push_back(iL());
    pch = strtok (NULL,";");
    if(pch) iList[i].iItem = pch;
    pch = strtok (NULL,";");
    if(pch) iList[i].iData = atoi(pch); 
  }
  
  delete[] buffer;

  terminalGUIobj->printConsole(F("TYPE:"), TFT_GREEN, 1, 1);
  
  for (uint16_t i=0; i<itemsQuant; i++){
    char statusChar = ' ';
    if(globalStr.lastVisitTimestamp < iList[i].iData && globalStr.userLineNo) statusChar = '!';
    
    snprintf(buf, sizeof(buf), "%d%c%s", i+1, statusChar, iList[i].iItem.c_str());
    terminalGUIobj->printConsole(String(buf), TFT_YELLOW, 1, 0);
  }

  terminalGUIobj->printConsole(F("Choose No:"), TFT_MAGENTA, 1, 0);

  selectedItem = 0;
  while (selectedItem==0 || selectedItem>itemsQuant){ 
    selectedItem = terminalGUIobj->getUserInput().toInt();
  };

  globalStr.type = iList[selectedItem-1].iItem;
  terminalGUIobj->printConsole(globalStr.type, TFT_GREEN, 1, 1);

  iList.clear(); 
  iList.shrink_to_fit(); 

  return (OK_STATE);
};
  
RESULT_STATE ESPboyOTA2::get_genre(){
 String resp;
 char *pch;
 uint16_t itemsQuant = 0;
 int16_t selectedItem;
 char buf[64]; 
  
  terminalGUIobj->printConsole("", TFT_BLACK, 1, 0);
  terminalGUIobj->printConsole("GENREs update", TFT_MAGENTA, 1, 0);

  resp = GETmethod(GET_GENRE);
  
  char* buffer = new char[resp.length() + 1];
  resp.toCharArray(buffer, resp.length() + 1);
  pch = strtok (buffer,";");
  
  if((String)pch != "0"){
      delete[] buffer;
      return(FAULT_STATE);
  }

  pch = strtok (NULL,";");
  if(pch) itemsQuant = atoi(pch);
  
  iList.reserve(itemsQuant + 1); 
  
  for (uint16_t i=0; i<itemsQuant; i++){
    yield();
    iList.push_back(iL());
    pch = strtok (NULL,";");
    if(pch) iList[i].iItem = pch;
    pch = strtok (NULL,";");
    if(pch) iList[i].iData = atoi(pch); 
  }
  
  delete[] buffer;

  iList.push_back(iL());
  iList[itemsQuant].iItem = itemBck;
  iList[itemsQuant].iData = globalStr.lastVisitTimestamp;

  terminalGUIobj->printConsole(F("GENRE:"), TFT_GREEN, 1, 1);

  for (uint16_t i=0; i<iList.size(); i++){
    char statusChar = ' ';
    if(globalStr.lastVisitTimestamp < iList[i].iData && globalStr.userLineNo) statusChar = '!';
    
    snprintf(buf, sizeof(buf), "%d%c%s", i+1, statusChar, iList[i].iItem.c_str());
    terminalGUIobj->printConsole(String(buf), TFT_YELLOW, 1, 0);
  }

  terminalGUIobj->printConsole(F("Choose No:"), TFT_MAGENTA, 1, 0);

  selectedItem = 0;
  while (selectedItem==0 || selectedItem>itemsQuant+1){ 
    selectedItem = terminalGUIobj->getUserInput().toInt();
  };
  
  globalStr.genre = iList[selectedItem-1].iItem;
  terminalGUIobj->printConsole(globalStr.genre, TFT_GREEN, 1, 1);

  iList.clear(); 
  iList.shrink_to_fit(); 

  if(selectedItem>itemsQuant) return (BACK_STATE);
  else return (OK_STATE);
};

RESULT_STATE ESPboyOTA2::get_app(){
 String resp="";
 char *pch;
 uint16_t itemsQuant = 0;
 int16_t selectedItem;
 char buf[64]; 
  
  terminalGUIobj->printConsole("", TFT_BLACK, 1, 0);
  terminalGUIobj->printConsole("APPs update", TFT_MAGENTA, 1, 0);
  
  resp = GETmethod(GET_APP);
  
  char* buffer = new char[resp.length() + 1];
  resp.toCharArray(buffer, resp.length() + 1);
  pch = strtok (buffer,";");
  
  if((String)pch != "0"){
      delete[] buffer;
      return(FAULT_STATE);
  }

  pch = strtok (NULL,";");
  if(pch) itemsQuant = atoi(pch);

  iList.reserve(itemsQuant + 1); 

  for (uint16_t i=0; i<itemsQuant; i++){
    yield();
    iList.push_back(iL());
    pch = strtok (NULL,";");
    if(pch) iList[i].iItem = pch;
    pch = strtok (NULL,";");
    if(pch) iList[i].iData = atoi(pch); 
    pch = strtok (NULL,";");
    if(pch) iList[i].iAccsess = atoi(pch); 
    pch = strtok (NULL,";");
    if(pch) iList[i].iLike = atoi(pch); 
  }
  
  delete[] buffer;

  iList.push_back(iL());
  iList[itemsQuant].iItem = itemBck;

  terminalGUIobj->printConsole(F("APP:"), TFT_GREEN, 1, 1);
  
  for (uint16_t i=0; i<iList.size(); i++){
    char statusChar = ' ';
    if (globalStr.accessUser < iList[i].iAccsess) statusChar = '-';
    else if(globalStr.lastVisitTimestamp < iList[i].iData && globalStr.userLineNo) statusChar = '!';

    snprintf(buf, sizeof(buf), "%d%c%s", i+1, statusChar, iList[i].iItem.c_str());
    String toPrint = String(buf);
    
    if(i<itemsQuant){
       snprintf(buf, sizeof(buf), " %d/%d", iList[i].iAccsess, iList[i].iLike);
       String toPrint2 = String(buf);
       if (toPrint.length() > ((128/GUI_FONT_WIDTH) - toPrint2.length() - 1)) {
           toPrint = toPrint.substring(0, (128/GUI_FONT_WIDTH) - toPrint2.length() - 1);
       }
       toPrint += toPrint2;
    }
    terminalGUIobj->printConsole(toPrint, TFT_YELLOW, 1, 0);
  }

  terminalGUIobj->printConsole("", TFT_BLACK, 1, 0);

  // ОПТИМИЗАЦИЯ: Убран оператор goto, заменен на цикл while (true)
  while(true) {
    selectedItem = 0;
    terminalGUIobj->printConsole(F("Choose No:"), TFT_MAGENTA, 1, 1);
    
    while (selectedItem == 0 || selectedItem > itemsQuant + 1){ 
      selectedItem = terminalGUIobj->getUserInput().toInt();
    }

    if (selectedItem != itemsQuant + 1 && globalStr.accessUser < iList[selectedItem - 1].iAccsess){
       terminalGUIobj->printConsole(F("Low access level"), TFT_RED, 1, 1);
       delay(3000);
    } else {
       break; 
    }
  }

  globalStr.app = iList[selectedItem-1].iItem;
  terminalGUIobj->printConsole(globalStr.app, TFT_GREEN, 1, 1);

  iList.clear();
  iList.shrink_to_fit();

  terminalGUIobj->printConsole("", TFT_BLACK, 1, 0);

  if(selectedItem>itemsQuant) return (BACK_STATE);
  else return (OK_STATE);
};

RESULT_STATE ESPboyOTA2::get_info(){
 String resp="";
 char *pch;
 uint8_t selectedItem;
 String toPrint;

  terminalGUIobj->printConsole("", TFT_BLACK, 1, 0);
  
  resp = GETmethod(GET_INFO);  
 
  char* buffer = new char[resp.length() + 1];
  resp.toCharArray(buffer, resp.length() + 1);
  pch = strtok (buffer,";");
  
  if((String)pch != "0"){
      delete[] buffer;
      return(FAULT_STATE);
  }
  
  pch = strtok (NULL,";");
  terminalGUIobj->printConsole(F("Game name:"), TFT_GREEN, 1, 0);
  if(pch) terminalGUIobj->printConsole(pch, TFT_YELLOW, 1, 0);
  pch = strtok (NULL,";");
  terminalGUIobj->printConsole(F("Author:"), TFT_GREEN, 1, 0);
  if(pch) terminalGUIobj->printConsole(pch, TFT_YELLOW, 1, 0);
  pch = strtok (NULL,";");
  terminalGUIobj->printConsole(F("License:"), TFT_GREEN, 1, 0);
  if(pch) terminalGUIobj->printConsole(pch, TFT_YELLOW, 1, 0);
  pch = strtok (NULL,";");
  terminalGUIobj->printConsole(F("Description:"), TFT_GREEN, 1, 0);
  if(pch) terminalGUIobj->printConsole(pch, TFT_YELLOW, 1, 0);
  pch = strtok (NULL,";");
  if(pch) globalStr.updlink = pch;
  pch = strtok (NULL,";");
  terminalGUIobj->printConsole(F("Access level/likes:"), TFT_GREEN, 1, 0);
  if(pch) toPrint = pch;
  pch = strtok (NULL,";");
  toPrint += '/';
  if(pch) toPrint += pch;
  terminalGUIobj->printConsole(toPrint, TFT_YELLOW, 1, 0);
  pch = strtok (NULL,";");
  if(pch) globalStr.appLineNo = atoi(pch);
  
  delete[] buffer;
  
  terminalGUIobj->printConsole("", TFT_BLACK, 1, 0);
  
  String menu0 = FPSTR((const char*)pgm_read_dword(&finalMnu[0]));
  terminalGUIobj->printConsole(menu0, TFT_YELLOW, 1, 0);
  
  toPrint = FPSTR((const char*)pgm_read_dword(&finalMnu[1]));
  if (!globalStr.userLineNo) toPrint[1]='-';
  terminalGUIobj->printConsole(toPrint, TFT_YELLOW, 1, 0);
  
  String menu2 = FPSTR((const char*)pgm_read_dword(&finalMnu[2]));
  terminalGUIobj->printConsole(menu2, TFT_YELLOW, 1, 0);

  terminalGUIobj->printConsole("", TFT_BLACK, 1, 0);
  
  // ОПТИМИЗАЦИЯ: Убран оператор goto, заменен на цикл while (true)
  while (true) {
    terminalGUIobj->printConsole(F("Choose No:"), TFT_MAGENTA, 0, 1);
    selectedItem = 0;
    
    while (selectedItem != 1 && selectedItem != 2 && selectedItem != 3) { 
      selectedItem = terminalGUIobj->getUserInput().toInt();
    }

    if (selectedItem == 2 && !globalStr.userLineNo) {
      terminalGUIobj->printConsole(F("Low accsess level"), TFT_RED, 0, 1);
      delay(3000);
    } else {
      break;
    }
  }

  toPrint = FPSTR((const char*)pgm_read_dword(&finalMnu[selectedItem-1]));
  toPrint = toPrint.substring(2);
  terminalGUIobj->printConsole(toPrint, TFT_GREEN, 1, 1);

  switch (selectedItem){
    case 1: return (OK_STATE); break;
    case 2: return (LIKE_STATE); break;
    case 3: return (BACK_STATE); break;
  };
  
  return(FAULT_STATE);
};

void ESPboyOTA2::checkOTA() {
  String toPrint;

  toPrint = F("ESPboy AppStore v");
  terminalGUIobj->printConsole( toPrint + (String)OTAv, TFT_GREEN, 0, 0);
  terminalGUIobj->printConsole("", TFT_BLACK, 0, 0);
  
  pinMode(LED_BUILTIN, OUTPUT);
  WiFi.mode(WIFI_STA);

  while (!wifiConnect());

  terminalGUIobj->printConsole(F("Server check..."), TFT_MAGENTA, 0, 0);
  
  // ОПТИМИЗАЦИЯ: Перезагрузка при зависании
  if (get_mess() == OK_STATE && get_user() == OK_STATE) {
    terminalGUIobj->printConsole(F("Server OK"), TFT_MAGENTA, 0, 1);
  } else {
    terminalGUIobj->printConsole(F("FAULT, restarting..."), TFT_RED, 0, 0);
    delay(5000);
    ESP.restart(); 
  }

  terminalGUIobj->printConsole(F("Server init..."), TFT_MAGENTA, 0, 0);
  if (POSTmethod(POST_CONNECT) == "OK") {
    terminalGUIobj->printConsole(F("Init OK"), TFT_MAGENTA, 0, 1);
  } else {
    terminalGUIobj->printConsole(F("FAULT, restarting..."), TFT_RED, 0, 0);
    delay(5000);
    ESP.restart(); 
  }

  terminalGUIobj->printConsole("", TFT_BLACK, 0, 0);

  if (!globalStr.userLineNo){
    terminalGUIobj->printConsole(messStr[USERNOTFOUND_MESS].messmess, messStr[USERNOTFOUND_MESS].messcolor, 0, 0);
    toPrint = F("ID ");
    toPrint += ESP.getChipId();
    terminalGUIobj->printConsole(toPrint, messStr[USERNOTFOUND_MESS].messcolor, 0, 0);
    toPrint = F("MC ");
    toPrint += WiFi.macAddress();
    terminalGUIobj->printConsole(toPrint, messStr[USERNOTFOUND_MESS].messcolor, 0, 0);
    delay(messStr[USERNOTFOUND_MESS].messdelay);
  }
  else{
    terminalGUIobj->printConsole(messStr[USERFOUND_MESS].messmess, messStr[USERFOUND_MESS].messcolor, 0, 0);
    terminalGUIobj->printConsole(globalStr.nicknameUser, messStr[USERFOUND_MESS].messcolor, 0, 0);
    toPrint = F("Access level "); 
    toPrint += globalStr.accessUser;
    terminalGUIobj->printConsole(toPrint, messStr[USERFOUND_MESS].messcolor, 0, 0);
    
    toPrint = F("Last seen ");
    uint16_t yer = globalStr.lastVisitTimestamp/500;
    uint8_t mnt = (globalStr.lastVisitTimestamp - yer*500)/32;
    uint8_t dte = globalStr.lastVisitTimestamp - yer*500 - mnt*32;
    if (dte<10) toPrint+= '0';
    toPrint +=  (String)dte + ".";
    if (mnt<10) toPrint+= '0';
    toPrint += (String)mnt + "." + (String)yer;
    terminalGUIobj->printConsole(toPrint, messStr[USERFOUND_MESS].messcolor, 0, 0);
    delay(messStr[USERFOUND_MESS].messdelay);
  }

  terminalGUIobj->printConsole("", TFT_BLACK, 0, 0);
  terminalGUIobj->printConsole(messStr[START_MESS].messmess, messStr[START_MESS].messcolor, 0, 0);
  delay (messStr[START_MESS].messdelay);
    
  GET_cmd = GET_TYPE;
  
  while(1){
    RESULT_STATE recievedState;
    switch (GET_cmd){
      case GET_TYPE:
        if (get_type() == OK_STATE) GET_cmd = GET_GENRE;
        break;
      case GET_GENRE:
        recievedState = get_genre();
        if (recievedState == OK_STATE) GET_cmd = GET_APP;
        if (recievedState == BACK_STATE) GET_cmd = GET_TYPE;        
        break;      
      case GET_APP:
        recievedState = get_app();
        if (recievedState == OK_STATE) GET_cmd = GET_INFO;
        if (recievedState == BACK_STATE) GET_cmd = GET_GENRE;        
        break;      
      case GET_INFO:
        recievedState = get_info();
        if (recievedState == OK_STATE) GET_cmd = GET_UPDATE;
        if (recievedState == BACK_STATE) GET_cmd = GET_APP;  
        if (recievedState == LIKE_STATE) GET_cmd = GET_LIKE;        
        break;      
      case GET_LIKE:
        terminalGUIobj->printConsole("", TFT_BLACK, 1, 0);
        terminalGUIobj->printConsole(F("Update like..."), TFT_MAGENTA, 1, 0);
        POSTmethod(POST_LIKE);
        POSTmethod(POST_LOG_LIKE);
        terminalGUIobj->printConsole(F("OK"), TFT_GREEN, 1, 0);
        GET_cmd = GET_INFO;
        break;  
      case GET_UPDATE:
        terminalGUIobj->printConsole("", TFT_BLACK, 0, 0);
        terminalGUIobj->printConsole(messStr[BEFOREUPDATE_MESS].messmess, messStr[BEFOREUPDATE_MESS].messcolor, 1, 0);
        POSTmethod(POST_DOWNLOAD);
        delay(messStr[BEFOREUPDATE_MESS].messdelay);
        terminalGUIobj->printConsole("", TFT_BLACK, 0, 0);
        terminalGUIobj->printConsole("", TFT_BLACK, 0, 0);        
        if (UPDATEmethod()) {
          terminalGUIobj->printConsole("", TFT_BLACK, 0, 0);
          terminalGUIobj->printConsole(messStr[UPDATEFINISHWD_MESS].messmess, messStr[UPDATEFINISHWD_MESS].messcolor, 1, 0);
          delay(messStr[UPDATEFINISHWD_MESS].messdelay);
          ESP.restart();
        }
        else {
          terminalGUIobj->printConsole(F("Download FAILED"), TFT_RED, 0, 0);
          delay(3000);
          GET_cmd = GET_INFO;}      
          break;         
      }
    }
}