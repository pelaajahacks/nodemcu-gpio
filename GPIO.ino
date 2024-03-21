#ifdef ESP32
  #include <WiFi.h>
  #include <AsyncTCP.h>
#else
  #include <ESP8266WiFi.h>
  #include <ESPAsyncTCP.h>
#endif
#include <ESPAsyncWebServer.h>

// REPLACE WITH YOUR NETWORK CREDENTIALS
const char* ssid = "espoon_oppilas";
const char* password = "Runeberg1804";


const char* botToken = "tele bot token";
const String chatID = "-1002048212750";
const String topicID = "2";
String url = "/bot" + String(botToken) + "/sendMessage";
String host = "api.telegram.org";
const int httpsPort = 443;





const int MAX_PINS = 5; // Define the maximum number of pins

struct Switch {
  int gpio;
  bool status;
};

// Array to store the status of each GPIO pin (assuming all are initially off)
Switch switchStatus[MAX_PINS] = {
  {5, false},  // D1
  {4, false},  // D2
  {14, false}, // D5
  {12, false}, // D6
  {13, false}, // D7
};

// HTML web page
const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML>
<html>
  <head>
    <title>ESP Web Server</title>
    <meta name="viewport" content="width=device-width, initial-scale=1">
    <style>
      body { font-family: Arial, Helvetica, sans-serif; text-align: center; margin:0px auto; padding-top: 30px;}
      .button {
        padding: 10px 20px;
        font-size: 24px;
        text-align: center;
        outline: none;
        color: #fff;
        background-color: #ff0522;
        border: none;
        border-radius: 5px;
        cursor: pointer;
        -webkit-touch-callout: none;
        -webkit-user-select: none;
        -khtml-user-select: none;
        -moz-user-select: none;
        -ms-user-select: none;
        user-select: none;
        -webkit-tap-highlight-color: rgba(0,0,0,0);
      }  
      .button:hover {background-color: #ff0522 }
      .button:active {
        background-color: #1fe036;
        transform: translateY(2px);
      }
      .dropdown {
        font-family: Arial, sans-serif;
        margin: 20px;
      }

      select {
        padding: 10px;
        font-size: 16px;
        border: 1px solid #ccc;
        border-radius: 5px;
        width: 64px;
        appearance: none;
        -webkit-appearance: none;
        -moz-appearance: none;
        background-image: url('data:image/svg+xml;utf8,<svg fill="%23000" viewBox="0 0 24 24" xmlns="http://www.w3.org/2000/svg"><path d="M7 10l5 5 5-5z"/></svg>');
        background-repeat: no-repeat;
        background-position: right 10px top 50%;
        background-size: 20px;
      }

      .switch {
    position: relative;
    display: inline-block;
    width: 90px;
    height: 40px;
    }

    .switch input {
    opacity: 0;
    width: 0;
    height: 0;
    }

    .slider {
    position: absolute;
    cursor: pointer;
    top: 0;
    left: 0;
    right: 0;
    bottom: 0;
    background-color: #ccc;
    -webkit-transition: .4s;
    transition: .4s;
    border-radius: 34px;
    }

    .slider:before {
    position: absolute;
    content: "";
    height: 32px;
    width: 32px;
    left: 4px;
    bottom: 4px;
    background-color: white;
    -webkit-transition: .4s;
    transition: .4s;
    border-radius: 50%;
    }

    input:checked + .slider {
    background-color: #2196F3;
    }

    input:focus + .slider {
    box-shadow: 0 0 1px #2196F3;
    }

    input:checked + .slider:before {
    -webkit-transform: translateX(50px);
    -ms-transform: translateX(50px);
    transform: translateX(50px);
    }

    .main {
        display: flex;
        flex-direction: column;
        margin: auto 0;
    }

    .content {
        margin: 0 auto;
    }

    .switches {
        display: flex;

    }

    .switches > * {
        margin: auto 5px;
    }

  


    </style>
  </head>
  <body>
    <div class="main">
        <h1>NodeMCU GPIO Switches</h1>
        <div class="content">
            <div class="switches">
                <button class="button panic" onmousedown="panic()">Panic</button>
                <button class="button" onmousedown="toggleCheckbox(1);" ontouchstart="toggleCheckbox(1);" onmouseup="toggleCheckbox(0);" ontouchend="toggleCheckbox(0);">Momentary</button>
                <label class="switch" onmousedown="toggleCheckbox(checkSwitchState())">
                    <input type="checkbox">
                    <span class="slider round"></span>
                </label>
            </div>
          

            <div class="dropdown">
              <select id="pins">
                <option value="5" selected>D1</option>
                <option value="4">D2</option>
                <option value="14">D5</option>
                <option value="12">D6</option>
                <option value="13">D7</option>
              </select>
            </div>
        
          <div class="custom-req">
            
          </div>
        </div>
        
        
          
          
    
   <script>
    function toggleCheckbox(x) {
        var xhr = new XMLHttpRequest();
        xhr.open("POST", "update_status?gpio=" + getPin() + "&status=" + x, true);
        xhr.setRequestHeader("Content-Type", "application/json;charset=UTF-8"); // Set the appropriate content type
        xhr.send();
    }

   function checkSwitchState() {
    return document.querySelector('.switch input').checked ? '0' : "1"
   }

    function getPin() {
        return document.getElementById("pins").value;

   }

   function panic() {
        var xhr = new XMLHttpRequest();
        xhr.open("GET", "/panic", true);
        xhr.send();
   }

   function changePin() {
     var pin = document.getElementById("pins").value;
     console.log("Selected pin: " + pin);
     // You can perform further actions with the selected pin if needed
   }
  </script>
  </body>
</html>

)rawliteral";



AsyncWebServer server(80);

String escapify(const String& str) {
  String escapedStr = str;
  escapedStr.replace("_", "\\\\_");
  escapedStr.replace("*", "\\\\*");
  escapedStr.replace("[", "\\\\[");
  escapedStr.replace("`", "\\\\`");
  escapedStr.replace("!", "\\\\!");
  escapedStr.replace("#", "\\\\#");
  return escapedStr;
}

void sendTelegramMessage(String msg) {
  WiFiClientSecure httpsClient;
  httpsClient.setInsecure();
  if (!httpsClient.connect(host, httpsPort)) {
    Serial.println("oh noooo");
    return;
  }

  String payload = "{\"chat_id\":\"" + chatID + "\",\"message_thread_id\":\"" + topicID + "\",\"parse_mode\":\"MarkdownV2\",\"text\":\"" + msg + "\"}";
  Serial.println(payload);

  httpsClient.print("GET " + url + " HTTP/1.1\r\n");
  httpsClient.print("Content-Type: application/json\r\n");
  httpsClient.print("Host: " + host + "\r\n");
  httpsClient.print("Content-Length: " + String(payload.length()) + "\r\n");
  httpsClient.print("\r\n");
  httpsClient.print(payload);

  // Disconnect from the server
  httpsClient.stop();
}

void setup() {
  Serial.begin(9600);

  for (int i = 0; i < MAX_PINS; i++) {
    pinMode(switchStatus[i].gpio, OUTPUT);
    digitalWrite(switchStatus[i].gpio, LOW);
  }
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  if (WiFi.waitForConnectResult() != WL_CONNECTED) {
    Serial.println(String(ssid) + "//" + String(password)) + "\n";
    return;
  }
  sendTelegramMessage("*NodeMCU has connected to WiFi*\n\n*📶 Local IP*\n>`" + WiFi.localIP().toString() + "`\n\n*Network info*\n*📡 SSID*\n||" + escapify(ssid) + "||\n*🔑 Password*\n||" + escapify(password) + "||");

  // Send web page to client
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/html", index_html);
  });

  // Receive an HTTP POST request for updating status
  server.on("/update_status", HTTP_POST, [](AsyncWebServerRequest *request) {
    // Check if there is any content in the request
     if (request->hasParam("gpio") && request->hasParam("status")) {
      
      int gpio = request->getParam("gpio")->value().toInt();
      String status = request->getParam("status")->value();

      //String clientIP = request->client()->remoteIP().toString();
      //sendTelegramMessage("*Status Update*\n\n*GPIO*\n" + escapify(String(gpio)) + "\n*Status*\n" + (status ? "On" : "Off") + "\n\n*Local IP*\n" + escapify(clientIP));
      // Set pinMode and digitalWrite based on the status
      
      pinMode(gpio, OUTPUT);
      digitalWrite(gpio, (status.equalsIgnoreCase("true") || status.equals("1")) ? true : false);
      switchStatus[gpio].status = (status.equalsIgnoreCase("true") || status.equals("1")) ? true : false;

      // Send response
      request->send(200, "text/plain", "OK");
    } else {
      request->send(400, "text/plain", "No payload received");
    }
  });

  server.on("/panic", HTTP_GET, [](AsyncWebServerRequest *request) {
    for (int i = 0; i < MAX_PINS; i++) {
      pinMode(switchStatus[i].gpio, OUTPUT);
      digitalWrite(switchStatus[i].gpio, LOW);
      Serial.println(switchStatus[i].gpio);
      
    }
    
    request->send(200, "text/plain", "NeekeritVittuunSuomesta");
  });


  /*server.on("/status", HTTP_GET, [](AsyncWebServerRequest *request) {
    


    // Send the JSON response
    request->send(200, "application/json", response);
  }*/

  server.begin();
}

void loop() {
  // Your loop code here
}
