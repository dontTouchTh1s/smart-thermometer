// Import required libraries
#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebSrv.h>

#include <ESP8266HTTPClient.h>


#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <EEPROM.h>

const char* ssid     = "ESP8266-Access-Point";
const char* password = "123456789";

uint8_t DHTPin = D3;      // Digital pin connected to the DHT sensor

// Uncomment the type of sensor in use:
#define DHTTYPE DHT11
#define EEPROM_SIZE 1024

DHT dht(DHTPin, DHTTYPE);

int accountSettingEEPROMAddress = 50;
// current temperature & humidity, updated in loop()
float t = 0.0;
float h = 0.0;
String accountUserName;
String accountPassword;
int accountId;
bool needSaveAccount = false;
bool needRgisterAccount = false;
String serverPath = "http://broifeelhot.iapp.ir";
// Create AsyncWebServer object on port 80
AsyncWebServer server(80);

// Generally, you should use "unsigned long" for variables that hold time
// The value will quickly become too large for an int to store
unsigned long previousMillis = 0;    // will store last time DHT was updated

// Updates DHT readings every 10 seconds
const long interval = 5000;  

const char account_setting_html[] PROGMEM = R"rawliteral(
  <!DOCTYPE HTML>
<html dir="ltr" lang="en">
<head>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <style>
    @font-face {
  font-weight: 100 900;
  font-style: normal;
  font-display: swap;
}
    html {
        height: 100%;
        direction: ltr;
     font-family: Arial;
     display: inline-block;
     font-size: 14px;
     font-family: 'Vazirmatn RD';
    }
    body{
        height: 100%;
        margin: 0;
        padding:0 0;
    }
    h2 { 
        font-size: 1.0rem; 
        margin: 0;
    }
    p { font-size: 1.0rem; }
    .units { font-size: 1.2rem; }
    .dht-labels{
      font-size: 1.5rem;
      vertical-align:middle;
      padding-bottom: 15px;
    }
    a{
        color: rgb(39, 36, 36);

        text-decoration: none;
    }
    .header-menu{
        width: 100%;
        background-color: rgb(228, 228, 228);
    }
    .header-menu ul{
        display: -webkit-box;
        display: -moz-box;
        display: -ms-flexbox;
        display: -moz-flex;
        display: -webkit-flex;
        -webkit-justify-content: space-between; 
        -moz-justify-content: space-between;
        display: flex;
        justify-content: space-around;
        padding: 12px 0;
        width: 100%;
        margin: 0;
        list-style: none;
    }
    .header-menu ul li{
      padding: 5px;
      font-size: 22px;
      cursor: pointer;
      text-align: center;
    }
    .container{
        height: 100%;
        background-color: rgb(242, 245, 248);
        margin: 0;
        padding: 0 8px;
    }
    .content{
        padding: 8px;
    }
    input{
        height: 42px;
        width: 90%;
        border-radius: 8px;
        margin: 0;
        padding: 0;
        border: 1px solid rgb(110, 110, 110);
    }
    form{
        display: flex;
        background-color: rgb(255, 255, 255);
        flex-direction: column;
        border-radius: 15px;
        box-shadow: 3px 3px 7px rgb(66, 66, 66);
        padding: 12px 16px;
    }
    label{
        display: block;
    }
    .form-control{
        margin-top: 8px;
    }
    .form-control input{
        margin-right: 8px;
        margin-left: 8px;
    }
    button{
        display: flex;
        justify-content: center;
        align-items: center;
        font-family: 'Vazirmatn RD';
        min-height: 42px;
        padding: 4px 8px;
        border-radius: 8px;
        border: none;
        background-color: rgb(255, 213, 121);
    }
  </style>
</head>
<body>
  <div class="header-menu">
    <ul>
      <li><a href="/">Home</a></li>
      <li><a href="/setting">Settings</a></li>
      <li><a href="/account-setting">Account settings</a></li>
    </ul>
  </div>
  <div class="container">
    <div class="content">
      <h2>User accounts settings</h2>
      <p>Enter your account username and password to access you device over internet.</p>
      <form method="get" action="/change-account-setting">
        <div class="form-control">
          <label>Username</label>
          <input type="text" name="user-name"/>
        </div>
        <div class="form-control">
          <label>Password</label>
          <input type="password" name="password"/>
        </div>
        <div class="form-control">
          <button type="submit" >Save</button>
        </div>
      </form>
    </div>
  </div>
</body>
)rawliteral";


const char setting_html[] PROGMEM = R"rawliteral(
  <!DOCTYPE HTML>
<html dir="ltr" lang="en">
<head>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <style>
    @font-face {
  font-weight: 100 900;
  font-style: normal;
  font-display: swap;
}
    html {
      height: 100%;
      direction: ltr;
      font-family: Arial;
      display: inline-block;
      font-size: 14px;
      font-family: 'Vazirmatn RD';
    }
    body{
        height: 100%;
        margin: 0;
        padding:0 0;
    }
    h2 { 
        font-size: 1.0rem; 
        margin: 0;
    }
    p { font-size: 1.0rem; }
    .units { font-size: 1.2rem; }
    .dht-labels{
      font-size: 1.5rem;
      vertical-align:middle;
      padding-bottom: 15px;
    }
    a{
        color: rgb(39, 36, 36);
        text-decoration: none;
    }
    .header-menu{
        width: 100%;
        background-color: rgb(228, 228, 228);
    }
    .header-menu ul{
        display: -webkit-box;
        display: -moz-box;
        display: -ms-flexbox;
        display: -moz-flex;
        display: -webkit-flex;
        -webkit-justify-content: space-between; 
        -moz-justify-content: space-between;
        display: flex;
        justify-content: space-around;
        padding: 12px 0;
        width: 100%;
        margin: 0;
        list-style: none;
    }
    .header-menu ul li{
      padding: 5px;
      font-size: 22px;
      cursor: pointer;
      text-align: center;
    }
    .container{
        height: 100%;
        background-color: rgb(242, 245, 248);
        margin: 0;
        padding: 0 8px;
    }
    .content{
        padding: 8px;
    }
    input{
        height: 42px;
        width: 90%;
        border-radius: 8px;
        margin: 0;
        padding: 0;
        border: 1px solid rgb(110, 110, 110);
    }
    form{
        display: flex;
        background-color: rgb(255, 255, 255);
        flex-direction: column;
        border-radius: 15px;
        box-shadow: 3px 3px 7px rgb(66, 66, 66);
        padding: 12px 16px;
    }
    label{
        display: block;
    }
    .form-control{
        margin-top: 8px;
    }
    .form-control input{
        margin-right: 8px;
        margin-left: 8px;
    }
    button{
        display: flex;
        justify-content: center;
        align-items: center;
        font-family: 'Vazirmatn RD';
        min-height: 42px;
        padding: 4px 8px;
        border-radius: 8px;
        border: none;
        background-color: rgb(255, 213, 121);
    }
    .success{
    background-color: rgb(179, 255, 179);
    }
    .error{
        background-color: rgb(255, 146, 146);
    }
  </style>
</head>
<body>
  <div class="header-menu">
      <ul>
          <li><a href="/">Home</a></li>
          <li><a href="/setting">Settings</a></li>
          <li><a href="/account-setting">Account Settings</a></li>
      </ul>
  </div>
  <div class="container">
    <div class="content">
      <h2>Device Settings</h2>
      <p>Enter SSID and password of the wifi network you want device to connect at, and send data onver internet.</p>
      <form method="get" action="/change-setting">
        <div class="form-control">
          <label>SSID: </label>
          <input type="text" name="ssid"/>
        </div>
        <div class="form-control">
          <label>Password</label>
          <input type="password" name="password"/>
        </div>
        <div class="form-control">
          <button type="submit" >Save</button>
        </div>
      </form>
    </div>
  </div>
</body>
)rawliteral";



const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML><html>
<head>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <style>
  <style>
    @font-face {
  font-weight: 100 900;
  font-style: normal;
  font-display: swap;
}
    html {
    height: 100%;
    direction: ltr;
     font-family: Arial;
     display: inline-block;
     font-size: 14px;
     font-family: 'Vazirmatn RD';
    }
    body{
        height: 100%;
        margin: 0;
        padding:0 0;
    }
    h2 { 
        font-size: 1.0rem; 
        margin: 0;
    }
    p { font-size: 1.0rem; }
    .units { font-size: 1.2rem; }
    .dht-labels{
      font-size: 1.5rem;
      vertical-align:middle;
      padding-bottom: 15px;
    }
    a{
        color: rgb(39, 36, 36);

        text-decoration: none;
    }
    .header-menu{
        width: 100%;
        background-color: rgb(228, 228, 228);
    }
    .header-menu ul{
        display: -webkit-box;
        display: -moz-box;
        display: -ms-flexbox;
        display: -moz-flex;
        display: -webkit-flex;
        -webkit-justify-content: space-between; 
        -moz-justify-content: space-between;
        display: flex;
        justify-content: space-around;
        padding: 12px 0;
        margin: 0;
        list-style: none;
    }
    .header-menu ul li{
      padding: 5px;
      font-size: 22px;
      cursor: pointer;
      text-align: center;
    }
    .container{
        background-color: rgb(242, 245, 248);
        margin: 0;
        padding: 0 8px;
    }
    .content{
        padding: 8px;
    }

    html {
     font-family: Arial;
     display: inline-block;
     margin: 0px auto;
     text-align: center;
    }
    h2 { font-size: 3.0rem; }
    p { font-size: 3.0rem; }
    .units { font-size: 1.2rem; }
    .dht-labels{
      font-size: 1.5rem;
      vertical-align:middle;
      padding-bottom: 15px;
    }
  </style>
</head>
<body>
<div class="header-menu">
      <ul>
          <li><a href="/">Home</a></li>
          <li><a href="/setting">Settings</a></li>
          <li><a href="/account-setting">Account Settings</a></li>
      </ul>
  </div>
  <div class="container">
    <div class="content">
  <h2>ESP8266 DHT Server</h2>
  <p>
    <span class="dht-labels">Temperature</span> 
    <span id="temperature">%TEMPERATURE%</span>
    <sup class="units">&deg;C</sup>
  </p>
  <p>
    <span class="dht-labels">Humidity</span>
    <span id="humidity">%HUMIDITY%</span>
    <sup class="units">%</sup>
  </p>
  </div>
</div>
</body>

<script>
setInterval(function ( ) {
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      document.getElementById("temperature").innerHTML = this.responseText;
    }
  };
  xhttp.open("GET", "/temperature", true);
  xhttp.send();
}, 10000 ) ;

setInterval(function ( ) {
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      document.getElementById("humidity").innerHTML = this.responseText;
    }
  };
  xhttp.open("GET", "/humidity", true);
  xhttp.send();
}, 10000 ) ;
</script>
</html>)rawliteral";

// Replaces placeholder with DHT values
String processor(const String& var)
{
  //Serial.println(var);
  if(var == "TEMPERATURE"){
    return String(t);
  }
  else if(var == "HUMIDITY"){
    return String(h);
  }

  return String();
}

void WifiBegin(String wifissid, String wifiPassword)
{
    WiFi.disconnect(false);
    WiFi.mode(WIFI_AP_STA);
    WiFi.setAutoConnect(true);
    WiFi.begin(wifissid, wifiPassword);
    Serial.println(WiFi.status());
    
    delay(100);

    if (WiFi.status() == WL_CONNECTED) {
      Serial.println("WiFi connected..!");
      Serial.print("Got IP: ");  Serial.println(WiFi.localIP());
    }
    Serial.println(WiFi.status());
}

int CommitEEPROM(String text, int address = 0)
{
  int i;
  EEPROM.begin(EEPROM_SIZE);
  for (i = 0; i < text.length(); i++) {
    EEPROM.write(i + address, text[i]);
  }

  EEPROM.write(text.length() + address, '\0');
  EEPROM.commit();
  EEPROM.end();
  return i + 1;
}

String GetEEPROM(int address = 0) {
  EEPROM.begin(EEPROM_SIZE);
  String text;
  char readChar = 36;
  int i = 0 + address;

  while (readChar != '\0') {
    readChar = char(EEPROM.read(i));
    i++;

    if (readChar != '\0') {
      text += readChar;
    }
  }
  EEPROM.end();
  return text;
}
bool GetParams(AsyncWebServerRequest *request, String paramName, String &valueOut)
{
    if (request->hasParam(paramName))
    {
        valueOut = request->getParam(paramName)->value();
        return true;
    }
    else
    {
        return false;
    }
}
void setup(){
  // Serial port for debugging purposes
  Serial.begin(115200);
  delay(100);
  // Reading id from EEPROM
  accountId = GetEEPROM(accountSettingEEPROMAddress).toInt();
  Serial.println("");
  Serial.print("Setting AP (Access Point)…");
  // Remove the password parameter, if you want the AP (Access Point) to be open
  WiFi.softAP(ssid, password);

  IPAddress IP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(IP);

  // Print ESP8266 Local IP Address
  Serial.println(WiFi.localIP());

  String wifiSSID = GetEEPROM();
  Serial.println(wifiSSID);

  if (wifiSSID != "")
  {
    String wifiPassword = GetEEPROM(wifiSSID.length() + 1);
    Serial.println(wifiPassword);
    WifiBegin(wifiSSID, wifiPassword);
  }

  // Config web server
  // Route for root / web page
  // This methods work like listeners
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/html", index_html, processor);
  });
  server.on("/temperature", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/plain", String(t).c_str());
  });
  server.on("/humidity", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/plain", String(h).c_str());
  });
  server.on("/setting", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/html", setting_html);
  });
  server.on("/account-setting", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/html", account_setting_html);
  });
  // server.on("/create-account", HTTP_GET, [](AsyncWebServerRequest *request){
  //   needRgisterAccount = true;
  // });
  server.on("/change-account-setting", HTTP_GET, [](AsyncWebServerRequest *request){
    String userName;
    String password;
    if (!GetParams(request, "user-name", userName))
    {
      request->redirect("/account-setting");
      return;
    }
    if (!GetParams(request, "password", password))
    {
      request->redirect("/account-setting");
      return;
    }

    Serial.println(userName);
    Serial.println(password);
    accountUserName = userName;
    accountPassword = password;
    needSaveAccount = true;
    request->redirect("/account-setting");
  });
  server.on("/change-setting", HTTP_GET, [](AsyncWebServerRequest *request){
    String ssid;
    String password;

    if (!GetParams(request, "ssid", ssid))
    {
      request->redirect("/setting");
      return;
    }
    if (!GetParams(request, "password", password))
    {
      request->redirect("/setting");
      return;
    }

    Serial.println(ssid);
    Serial.println(password);

    int address = CommitEEPROM(ssid);
    CommitEEPROM(password, address);

    Serial.println("Settings saved successfully");

    // Redirecting to setting page with response
    request->redirect("/setting");
    WifiBegin(ssid, password);

  });
  // Start server
  server.begin();

  pinMode(DHTPin, INPUT);
  dht.begin(); 
}

void loop(){  
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= interval) {
    // save the last time you updated the DHT values
    previousMillis = currentMillis;
    // Read temperature as Celsius (the default)
    float newT = dht.readTemperature();
    // Read temperature as Fahrenheit (isFahrenheit = true)
    //float newT = dht.readTemperature(true);
    // if temperature read failed, don't change t value
    if (isnan(newT)) {
      Serial.println("Failed to read from DHT sensor!");
    }
    else {
      t = newT;
      Serial.println(t);
    }
    // Read Humidity
    float newH = dht.readHumidity();
    // if humidity read failed, don't change h value 
    if (isnan(newH)) {
      Serial.println("Failed to read from DHT sensor!");
    }
    else {
      h = newH;
      Serial.println(h);

    }


    if(WiFi.status()== WL_CONNECTED){
      Serial.println("Connected, sending data ...");

      WiFiClient client;
      HTTPClient http;
      
      http.begin(client, (serverPath + "/howHot.php").c_str());
      // Specify content-type header
      http.addHeader("Content-Type", "application/json");
      int httpResponseCode = http.POST("{\"temperature\":\""+String(newT)+"\",\"humidity\":\""+String(newH)+"\", \"id\":"+ accountId + "}");   // Send HTTP POST request
      Serial.println(httpResponseCode);
      http.end();
    }
    if (needSaveAccount == true)
    {
      WiFiClient client;
      HTTPClient http;
    
      http.begin(client, (serverPath + "/login.php").c_str());
      // Specify content-type header
      http.addHeader("Content-Type", "application/json");
      String json =  "{\"user-name\":\"" + accountUserName + "\",\"password\":\"" + accountPassword + "\"}";           
      int httpResponseCode = http.POST(json);   // Send HTTP POST request
      Serial.println(httpResponseCode);
      if (httpResponseCode == 200){
        // Was successfull
        String responseContent = http.getString().c_str();
        Serial.println(responseContent);
        CommitEEPROM(responseContent, accountSettingEEPROMAddress);
        accountId = responseContent.toInt();
        Serial.println("Account setting saved successfully");
        http.end();
      }
      else if (httpResponseCode == 401){
        http.end();
        Serial.println("User name or password is wrong!");
      }
      else{
      http.end();
      Serial.println("Not excpeting errors ...");
      }
      needSaveAccount = false;
    }
    // if (needRgisterAccount == true)
    // {
    //   WiFiClient client;
    //   HTTPClient http;
    
    //   http.begin(client, (serverPath + "/iFeelRomantical.php").c_str());
    //   // Specify content-type header
    //   http.addHeader("Content-Type", "application/json");
    //   String json =  "{\"user-name\":\"" + accountUserName + "\",\"password\":\"" + accountPassword + "\"}";           
    //   int httpResponseCode = http.POST(json);   // Send HTTP POST request
    //   Serial.println(httpResponseCode);
    //   if (httpResponseCode == 200){
    //     // Was successfull
    //     String responseContent = http.getString().c_str();
    //     Serial.println(responseContent);
    //     CommitEEPROM(responseContent, accountSettingEEPROMAddress);
    //     accountId = responseContent.toInt();
    //     Serial.println("Account created successfully");
    //     http.end();
    //   }
    //   else if (httpResponseCode == 409){
    //     http.end();
    //     Serial.println("User already exist!");
    //   }
    //   else{
    //   http.end();
    //   Serial.println("Saving changes failed!");
    //   }
    //   needRgisterAccount = false;
    // }
  }
}
