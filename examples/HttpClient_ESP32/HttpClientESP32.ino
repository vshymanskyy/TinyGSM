
#include <ArduinoHttpClient.h>
#define TINY_GSM_MODEM_SIM800
#include <TinyGsmClient.h>

#define RXD2 16
#define TXD2 17

#define APN_NAME "internet.beeline.ru"
#define APN_USER "beeline"
#define APN_PSWD "beeline"

#define GSM_AUTOBAUD_MIN 9600
#define GSM_AUTOBAUD_MAX 38400

#define Max_Modem_Reboots 5

// set your cad pin (optional)
#define SIM_PIN ""

TinyGsm modemGSM(Serial2);

TinyGsmClient clientGSM(modemGSM);

String server = "www.feds.ru";
const int  port = 80;

int Modem_Reboots_Counter = 0;

HttpClient* http;

void RestartGSMModem()
{
    Serial.println("Restarting GSM...");
    if (!modemGSM.restart())
    {
      Serial.println("\tFailed. :-(\r\n");
      //ESP.restart();
    } 
    if (Modem_Reboots_Counter < Max_Modem_Reboots)
    {
      Init_GSM_SIM800();
    }
    Modem_Reboots_Counter++;
}

String GSMSignalLevel(int level)
{
  switch (level)
  {
    case 0:
      return "-115 dBm or less";
    case 1:
      return "-111 dBm";
    case 31:
      return "-52 dBm or greater";
    case 99:
      return "not known or not detectable";
    default:   
         if (level > 1 && level < 31)
           return "-110... -54 dBm";
  }
  return "Unknown";
}

String GSMRegistrationStatus(RegStatus state)
{
  switch (state)
  {
    case REG_UNREGISTERED:
      return "Not registered, MT is not currently searching a new operator to register to";
    case REG_SEARCHING:
      return "Not registered, but MT is currently searching a new operator to register to";
    case REG_DENIED:
      return "Registration denied";
    case REG_OK_HOME:
      return "Registered, home network";
    case REG_OK_ROAMING:
      return "Registered, roaming";
    case REG_UNKNOWN:
      return "Unknown"; 
  }
  return "Unknown";
}

String SIMStatus(SimStatus state)
{
  switch (state)
  {
    case SIM_ERROR:
      return "SIM card ERROR";
    case SIM_READY:
      return "SIM card is READY";
    case SIM_LOCKED:
      return "SIM card is LOCKED. PIN/PUK needed.";
  }
  return "Unknown STATUS";
}

String SwapLocation(String location)
{
   int i = location.indexOf(',');
   int j = location.indexOf(',', i+1);
   String longitude = location.substring(i+1, j); 
   i = location.indexOf(',', j+1);
   String latitude = location.substring(j+1, i); 
   return latitude + "," + longitude;
}

bool SendTextByPOST(String server, String url, String postData)
{
  bool stateGPRS = modemGSM.isGprsConnected();
  bool res = false;
  if (stateGPRS)
  {
    http = new HttpClient(clientGSM, server, port);
    Serial.println("Send POST request...");
    http->beginRequest();
    http->post(url);
    http->sendHeader("Content-Type", "application/x-www-form-urlencoded");
    http->sendHeader("Content-Length", postData.length());
    //http.sendHeader("X-Custom-Header", "custom-header-value");
    http->beginBody();
    http->print(postData);
    http->endRequest();

    // read the status code and body of the response
    int statusCode = http->responseStatusCode();
    if (statusCode == t_http_codes::HTTP_CODE_OK)
    {
      String response = http->responseBody();
      Serial.println("Status code: " + String(statusCode));
      Serial.println("Response: " + response);
      res = false;
    }
    else
    {
      Serial.println("Error code: " + String(statusCode));
      res = true;
    }
    
    http->stop();
   }
  return res;
}

void Init_GSM_SIM800() 
{
  Serial.println("Initialize GSM modem...");
  Serial2.begin(9600, SERIAL_8N1, RXD2, TXD2);
  Serial.println("Serial GSM Txd is on GPIO" + String(TXD2));
  Serial.println("Serial GSM Rxd is on GPIO" + String(RXD2));

  //pinMode(LED_PIN, OUTPUT);

  delay(3000);

  TinyGsmAutoBaud(Serial2, GSM_AUTOBAUD_MIN, GSM_AUTOBAUD_MAX);

  String info = modemGSM.getModemInfo();
  
  Serial.println(info);
 
  if (!modemGSM.restart())
  {
    RestartGSMModem();
  }
  else
  {
    Serial.println("Modem restart OK");
  }

  if (modemGSM.getSimStatus() != SIM_READY)
  {
    Serial.println("Check PIN code for the SIM. SIM status: " + SimStatus(modemGSM.getSimStatus()));
    if (SIM_PIN != "")
    {
     Serial.println("Try to unlock SIM PIN.");
      modemGSM.simUnlock(SIM_PIN);
      delay(3000);
      if (modemGSM.getSimStatus() != 3)
      {
        RestartGSMModem();
      }
    }
  }
 
  if (!modemGSM.waitForNetwork()) 
  {
    Serial.println("Failed to connect to network");
    RestartGSMModem();
  }
  else
  {
    RegStatus registration = modemGSM.getRegistrationStatus();
    Serial.println("Registration: [" + GSMRegistrationStatus(registration) + "]"); 
    Serial.println("Modem network OK");
  }
  
  Serial.println(modemGSM.gprsConnect(APN_NAME,APN_USER,APN_PSWD) ? "GPRS Connect OK" : "GPRS Connection failed");

  bool stateGPRS = modemGSM.isGprsConnected();
  if (!stateGPRS)
  {
    RestartGSMModem();
  }
  
  String state = stateGPRS ? "connected" : "not connected";
  Serial.println("GPRS status: " + state);
  Serial.println("CCID: " + modemGSM.getSimCCID());
  Serial.println("IMEI: " + modemGSM.getIMEI());
  Serial.println("Operator: " + modemGSM.getOperator());

  IPAddress local = modemGSM.localIP();
  Serial.println("Local IP: " + local.toString());

  int csq = modemGSM.getSignalQuality();
  if (csq == 0)
  {
    Serial.println("Signal quality is 0. Restart modem.");
    RestartGSMModem();
  }
  Serial.println("Signal quality: " + GSMSignalLevel(csq) + " [" + String(csq) + "]");

  int battLevel = modemGSM.getBattPercent();
  Serial.println("Battery level: " + String(battLevel) + "%");

  float battVoltage = modemGSM.getBattVoltage() / 1000.0F;
  Serial.println("Battery voltage: " + String(battVoltage));

  String gsmLoc = modemGSM.getGsmLocation();
  Serial.println("GSM location: " + gsmLoc);
  Serial.println("GSM location: " + SwapLocation(gsmLoc));
}

void setup() 
{
  Serial.begin(9600);
  
  Init_GSM_SIM800();

  float h0 = 20 + (float)random(20)/10;
  float t0 = 30 + (float)random(30)/10;

  String postData = "IMEI=861349031663619&PTYPE=DUMP";
         postData += "&H0=" + String(h0); //.replace(',','.');
         postData += "&T0=" + String(t0); //.replace(',','.');

  Serial.println("Send data [" + postData + "] to [" + server + "].");
  SendTextByPOST(server, "/", postData);
}

void loop() {
  // put your main code here, to run repeatedly:

}
