# smart-thermometer
Smart thermometer (i know you see this name on every project) is a arduino project that measures **temperature** and **humidity** of enviroment.
In this project we used nodemcu esp8286 board and HDT11 sensor.
When device turn on, user will connect to device access point and see temerature and humidity. also can set a SSID and password of near wifi to let device to connect to it and use internet to share data in website so user can access data anywhere in our website.
Anyway, device will share data in a web server created in its access point in `192.168.4.1` address. So you can check temperature and humidity in 2 ways.
## Web back-end
For back-end of this project to store data, we are using **PHP** and **MySQL** database. device use api to update data, create new account or login to created account to send data in database.
User can create account from website and login to it from device, or do it from deivice.
Current hosted website url is `http://broifeelhot.iapp.ir`.
## Temperature and humidity sensor
For messure temperature and humidity, we used HDT11 sensor.
Working with this sensor is simple when you use `Adafruit_Sensor` library. With this library you can interface HDT11 sensor so easy.
## Storing in EEPROM
`EEPROM` library used to write wifi and account information in device EEPROM.
## Web server
For creating web server on access point `ESP8266WiFi`, `ESPAsyncTCP` and `ESPAsyncWebSrv` really helped us and make every thing easy.
A web server will create on device access point on `192.168.4.1` address so user can change setting of davice or see data.
## HTTP Requests
For sending http request to api `ESP8266HTTPClient` library is really helpfull that make works fast and easy. requests and responses sending in **Json**.
Requests sending with post method.
For handling responses and errors, api response with http status codes so we can handle states in device side easy.
