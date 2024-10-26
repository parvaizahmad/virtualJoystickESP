// Define the board type
#define ESP8266_BOARD 1
#define ESP32_BOARD 2

// Select the board type here
#define BOARD_TYPE ESP32_BOARD // Change to ESP32_BOARD for ESP32

#if BOARD_TYPE == ESP8266_BOARD
  #include <ESP8266WiFi.h>
  #include <ESP8266WebServer.h>
  #include <ESP8266WiFiMulti.h>
  #include <ESP8266mDNS.h>
  ESP8266WebServer server(80); // Define server for ESP8266
  ESP8266WiFiMulti WiFiMulti;  // Define WiFiMulti for ESP8266
#elif BOARD_TYPE == ESP32_BOARD
  #include <WiFi.h>
  #include <WebServer.h>
  #include <WiFiMulti.h>
  #include <ESPmDNS.h>
  WebServer server(80);        // Define server for ESP32
  WiFiMulti WiFiMulti;         // Define WiFiMulti for ESP32
#endif

#include <WebSocketsServer.h>
#include <ArduinoJson.h>


const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>ESP8266/32 Joystick Control</title>
    <style>
        body {
            font-family: 'Arial', sans-serif;
            display: flex;
            flex-direction: column;
            align-items: center;
            justify-content: top;
            height: 100vh;
            margin: 2px;
            padding: 5px;
            background-color: #f0f0f0;
            box-sizing: border-box;
        }

        .container {
            display: flex;
            flex-direction: column;
            align-items: center;
            gap: 5px; /* Increased gap for better spacing */
            padding: 0px 15px 0px 15px; /* Add padding for inner spacing */
            background: linear-gradient(135deg, #f0f0f0, #e0e0e0); /* Gradient background */
            border: 1px solid #ccc; /* Subtle border */
            border-radius: 15px; /* Rounded corners */
            box-shadow: 0 4px 8px rgba(0, 0, 0, 0.1); /* Subtle shadow */
            margin: 10px auto; /* Center the container with margin */
            max-width: 95%; /* Responsive width */
        }

        .joystick-container {
            position: relative;
            width: 250px;
            height: 250px;
            background: radial-gradient(circle, #f0f0f0, #ccc); /* Gradient background */
            border-radius: 50%;
            box-shadow: 0 8px 16px rgba(0, 0, 0, 0.2); /* Subtle shadow */
            border: 1px solid #bbb; /* Border for a more defined look */
            display: flex;
            justify-content: center;
            align-items: center;
        }     

        .joystick-container::before,
        .joystick-container::after {
            content: '';
            position: absolute;
            background-color: #cfcfcf; /* Line color */
        }

        .joystick-container::before {
            width: 1px; /* Line thickness */
            height: 100%;
            top: 0;
            left: 50%;
            transform: translateX(-50%);
        }

        .joystick-container::after {
            width: 100%;
            height: 1px; /* Line thickness */
            top: 50%;
            left: 0;
            transform: translateY(-50%);
        }

        .joystick {
            position: absolute;
            width: 50px;
            height: 50px;
            background: radial-gradient(circle, #007bff, #0056b3); /* Gradient background */
            border-radius: 50%;
            top: 50%;
            left: 50%;
            transform: translate(-50%, -50%);
            box-shadow: 0 8px 16px rgba(0, 0, 0, 0.2); /* Subtle shadow */
            touch-action: none;
            border: 2px solid #004080; /* Border for a more defined look */
            z-index: 1;
        }

        .joystick::before {
            content: '';
            position: absolute;
            width: 10px; /* Size of the red dot */
            height: 10px; /* Size of the red dot */
            background-color: rgb(121, 25, 25);
            border-radius: 50%;
            top: 50%;
            left: 50%;
            transform: translate(-50%, -50%);
            box-shadow: 0 2px 4px rgba(0, 0, 0, 0.2); /* Subtle shadow for the dot */
        }

        .sliders {
            display: flex;
            flex-direction: column; /* Align sliders vertically */
            justify-content: center;
            align-items: center;
            padding: 10px; /* Add padding to all sides */
            gap: 10px; /* Add space between sliders */
        }
        .slider-container {
            display: flex;
            flex-direction: column;
            align-items: center;
        }

        .slider {
            -webkit-appearance: none; /* Override default CSS styles */
            appearance: none;
            width: 300px; /* Full width */
            height: 15px; /* Height of the slider */
            background: linear-gradient(to right, green, red); /* Gradient background */
            outline: none; /* Remove outline */
            opacity: 0.7; /* Set transparency */
            transition: opacity .2s; /* Transition effect */
            border-radius: 10px; /* Rounded edges */
            /* transform: rotate(-90deg); Rotate the slider */
        }
        .slider:hover {
            opacity: 1; /* Fully opaque on hover */
        }
        .slider::-webkit-slider-thumb {
            -webkit-appearance: none; /* Override default CSS styles */
            appearance: none;
            width: 25px; /* Width of the thumb */
            height: 25px; /* Height of the thumb */
            background: #4CAF50; /* Green background */
            cursor: pointer; /* Cursor on hover */
            border-radius: 5px; /* Rounded edges */
        }
        .slider::-moz-range-thumb {
            width: 25px; /* Width of the thumb */
            height: 30px; /* Height of the thumb */
            background: #4CAF50; /* Green background */
            cursor: pointer; /* Cursor on hover */
            border-radius: 5px; /* Rounded edges */
        }

        .buttons {
            display: flex;
            justify-content: space-around;
            width: 330px;
            flex-wrap: wrap;
        }

        .button {
            width: 70px;
            height: 60px;
            background-color: #28a745;
            border: none;
            color: white;
            font-size: 16px;
            border-radius: 10px;
            box-shadow: 0 4px 8px rgba(0, 0, 0, 0.2);
            cursor: pointer;
            transition: background-color 0.3s ease;
            margin: 5px;
            user-select: none; /* Prevent text selection */
        }

        .button.switch-on {
            background-color: #ff0707;
        }

        .button.push-active {
            background-color: #0056b3;
            user-select: none; /* Prevent text selection */
        }
        .common-label {
            font-size: 16px;
            font-weight: bold;
            margin-right: 1px;
            color: #333;
        }

        .toggle-checkbox {
            width: 20px;
            height: 20px;
            cursor: pointer;
            accent-color: #007bff; /* Change the color of the checkbox */
        }
        .check-button-container {
            display: flex;
            justify-content: left;
            width: 100%;
            align-items: center;
            gap: 5px;
        }
        .status-label {
            font-size: 14px;
            font-weight: bold;
            padding: 5px 10px;
            border-radius: 5px;
            color: #fff;
        }

        .status-connected {
            background-color: #28a745; /* Green background for connected */
        }

        .status-disconnected {
            background-color: #dc3545; /* Red background for disconnected */
        }
        
    </style>
</head>
<body>
    <div class="container">
        <div id="status" class="status-label status-disconnected">Disconnected</div>

        <div class="buttons">
            <button id="switch1" class="button" onclick="toggleSwitch(1)">SW 1</button>
            <button id="switch2" class="button" onclick="toggleSwitch(2)">SW 2</button>
            <button id="switch3" class="button" onclick="toggleSwitch(3)">SW 3</button>
            <button id="switch4" class="button" onclick="toggleSwitch(4)">SW 4</button>
            <button id="push1" class="button" onmousedown="sendPushData(1, 1)" onmouseup="sendPushData(1, 0)" ontouchstart="sendPushData(1, 1)" ontouchend="sendPushData(1, 0)">Push 1</button>
            <button id="push2" class="button" onmousedown="sendPushData(2, 1)" onmouseup="sendPushData(2, 0)" ontouchstart="sendPushData(2, 1)" ontouchend="sendPushData(2, 0)">Push 2</button>
            <button id="push3" class="button" onmousedown="sendPushData(3, 1)" onmouseup="sendPushData(3, 0)" ontouchstart="sendPushData(3, 1)" ontouchend="sendPushData(3, 0)">Push 3</button>
            <button id="push4" class="button" onmousedown="sendPushData(4, 1)" onmouseup="sendPushData(4, 0)" ontouchstart="sendPushData(4, 1)" ontouchend="sendPushData(4, 0)">Push 4</button>
        </div>
        
        <div class="check-button-container">
            <label class="common-label" for="toggle-return">RTH:</label>
            <input class="toggle-checkbox" type="checkbox" id="toggle-return" checked onchange="handleToggleChange()">
            <label class="common-label" for="toggle-sl">SL:</label>
            <input class="toggle-checkbox" type="checkbox" id="toggle-sl" unchecked onchange="handleSL()">
            
            
        </div>
        <div class="joystick-container">
            <div class="joystick" id="joystick"></div>
        </div>

        <div class="sliders">
            <div class="slider-container">
                <label class="common-label" id="labelslider1" for="slider1">Slider 1: 0</label>
                <input type="range" id="slider1" class="slider" min="0" max="100" value="0" onchange="sendSlider1Data()">
            </div>
            <div class="slider-container">
                <label class="common-label" id="labelslider2" for="slider2">Slider 2: 0</label>
                <input type="range" id="slider2" class="slider" min="0" max="100" value="0" onchange="sendSlider2Data()">
            </div>
        </div>

        <div class="buttons">
            <button id="switch5" class="button" onclick="toggleSwitch(5)">SW 5</button>
            <button id="switch6" class="button" onclick="toggleSwitch(6)">SW 6</button>
            <button id="switch7" class="button" onclick="toggleSwitch(7)">SW 7</button>
            <button id="switch8" class="button" onclick="toggleSwitch(8)">SW 8</button>
            <button id="push5" class="button" onmousedown="sendPushData(5, 1)" onmouseup="sendPushData(5, 0)" ontouchstart="sendPushData(5, 1)" ontouchend="sendPushData(5, 0)">Push 5</button>
            <button id="push6" class="button" onmousedown="sendPushData(6, 1)" onmouseup="sendPushData(6, 0)" ontouchstart="sendPushData(6, 1)" ontouchend="sendPushData(6, 0)">Push 6</button>
            <button id="push7" class="button" onmousedown="sendPushData(7, 1)" onmouseup="sendPushData(7, 0)" ontouchstart="sendPushData(7, 1)" ontouchend="sendPushData(7, 0)">Push 7</button>
            <button id="push8" class="button" onmousedown="sendPushData(8, 1)" onmouseup="sendPushData(8, 0)" ontouchstart="sendPushData(8, 1)" ontouchend="sendPushData(8, 0)">Push 8</button>
        </div>
    </div>

    <script>
        const joystick = document.getElementById('joystick');
        const joystickContainer = document.querySelector('.joystick-container');
        const slider1 = document.getElementById('slider1');
        const slider2 = document.getElementById('slider2');
        const toggleReturn = document.getElementById('toggle-return');
        const statusLabel = document.getElementById('status');
        
        let joystickOffset = { x: 0, y: 0 };
        let isDragging = false;
        let xPos = 0;
        let yPos = 0;

        let socket;

        // Initialize WebSocket
        function initWebSocket() {
            socket = new WebSocket('ws://' + location.hostname + ':81'); // Replace with your server's IP and port

            socket.onopen = function() {
                console.log("WebSocket connection opened");
                updateStatus(true);
            };

            socket.onmessage = function(event) {
                console.log("Message from server: ", event.data);
            };

            socket.onclose = function() {
                console.log("WebSocket connection closed");
                updateStatus(false);
            };

            socket.onerror = function(error) {
                console.error("WebSocket error: ", error);
                updateStatus(false);
            };
        }

        // Send joystick position
        function sendJoystickData(x, y) {
            const data = JSON.stringify({ type: 'joystick', x: Math.round(x*125), y: Math.round(y*(-125)) });
            socket.send(data);
        }

        // Send slider data
        function sendSlider1Data() {
            const data = JSON.stringify({ slider1: slider1.value });
            // set the slider label to the slider name and then value of the slider like Slider 1: 50
            document.getElementById('labelslider1').innerHTML = "Slider 1: " + slider1.value;
            socket.send(data);
        }

        // Send slider data
        function sendSlider2Data() {
            const data = JSON.stringify({ slider2: slider2.value });
            document.getElementById('labelslider2').innerHTML = "Slider 2: " + slider2.value;
            socket.send(data);
        }

        function initSliders() {
            // initialize the sliders to 0
            document.getElementById('slider1').value = 0;
            document.getElementById('slider2').value = 0;
        }
        initSliders();

        // Toggle switch
        function toggleSwitch(id) {
            const button = document.getElementById('switch' + id);
            const isOn = button.classList.toggle('switch-on');  // Toggle switch status
            const value = isOn ? 1 : 0;
            const data = JSON.stringify({ type: 'switch', id: id, value: value });
            socket.send(data);
        }

        // Send push button data
        function sendPushData(id, value) {
            const button = document.getElementById('push' + id);
            const data = JSON.stringify({ type: 'push', id: id, value: value });
            socket.send(data);

            // Add visual effect on press
            if (value === 1) {
                button.classList.add('push-active');
            } else {
                button.classList.remove('push-active');
            }
        }

        // Joystick drag and reset
        joystick.addEventListener('mousedown', startDrag);
        joystick.addEventListener('touchstart', startDrag, { passive: true });

        
        function startDrag(event) {
            event.preventDefault();
            isDragging = true;

            document.addEventListener('mousemove', dragJoystick);
            document.addEventListener('mouseup', stopDrag);
            document.addEventListener('touchmove', dragJoystick, { passive: true });
            document.addEventListener('touchend', stopDrag);

            const rect = joystickContainer.getBoundingClientRect();

            if (event.touches) {
                joystickOffset.x = event.touches[0].clientX - rect.left - xPos;
                joystickOffset.y = event.touches[0].clientY - rect.top - yPos;
            } else {
                joystickOffset.x = event.clientX - rect.left - xPos;
                joystickOffset.y = event.clientY - rect.top - yPos;
            }
        }

        function dragJoystick(event) {
            if (!isDragging) return;

            const rect = joystickContainer.getBoundingClientRect();
            const limitRadius = rect.width / 2 - joystick.offsetWidth / 2;

            if (event.touches) {
                xPos = event.touches[0].clientX - rect.left - joystickOffset.x;
                yPos = event.touches[0].clientY - rect.top - joystickOffset.y;
            } else {
                xPos = event.clientX - rect.left - joystickOffset.x;
                yPos = event.clientY - rect.top - joystickOffset.y;
            }

            const distance = Math.sqrt(xPos * xPos + yPos * yPos);
            if (distance > limitRadius) {
                const angle = Math.atan2(yPos, xPos);
                xPos = limitRadius * Math.cos(angle);
                yPos = limitRadius * Math.sin(angle);
            }

            joystick.style.left = `${xPos + rect.width / 2}px`;
            joystick.style.top = `${yPos + rect.height / 2}px`;

            // Send joystick data to server
            sendJoystickData(xPos / limitRadius, yPos / limitRadius);
        }

        function stopDrag() {
            isDragging = false;
            document.removeEventListener('mousemove', dragJoystick);
            document.removeEventListener('mouseup', stopDrag);
            document.removeEventListener('touchmove', dragJoystick);
            document.removeEventListener('touchend', stopDrag);

            if (toggleReturn.checked) {
                sendJoystickData(0, 0);
                xPos = 0;
                yPos = 0;
                joystick.style.left = '50%';
                joystick.style.top = '50%';
            }
        }
        function updateStatus(isConnected) {
            if (isConnected) {
                statusLabel.textContent = 'Connected';
                statusLabel.classList.remove('status-disconnected');
                statusLabel.classList.add('status-connected');
            } else {
                statusLabel.textContent = 'Disconnected';
                statusLabel.classList.remove('status-connected');
                statusLabel.classList.add('status-disconnected');
            }
        }
        function handleToggleChange() {
            if (toggleReturn.checked) {
                sendJoystickData(0, 0);
                xPos = 0;
                yPos = 0;
                joystick.style.left = '50%';
                joystick.style.top = '50%';
            } else {
                console.log('Return to Home is disabled');
                // Add your logic here for when the checkbox is unchecked
            }
        }

        function handleSL() {
            // make the container unscrollable when the checkbox is checked
            if (document.getElementById('toggle-sl').checked) {
                // disable touch action of container
                document.querySelector('.container').style.touchAction = 'none';
            } else {
                // enable touch action of container
                document.querySelector('.container').style.touchAction = 'auto';
            }
        }

        window.onload = initWebSocket;
    </script>
</body>

</html>
)rawliteral";

#if BOARD_TYPE == ESP8266_BOARD
    unsigned long lastMDNSUpdate = 0;
    const unsigned long MDNS_UPDATE_INTERVAL = 100; // Update mDNS every 100 mili second
#endif

WebSocketsServer webSocket = WebSocketsServer(81);

void handleRoot() {
  server.send(200, "text/html", index_html);
}


void handleWebSocketMessage(uint8_t *data, size_t len) {
    // Allocate the JSON document
    StaticJsonDocument<200> doc;

    // Deserialize the JSON document
    DeserializationError error = deserializeJson(doc, data, len);

    // Test if parsing succeeds
    if (error) {
        Serial.print(F("deserializeJson() failed: "));
        Serial.println(error.f_str());
        return;
    }

    // Handle the received data based on type
    if (doc["type"] == "joystick") {
        Serial.println("Joystick data received");
        Serial.print("x: ");
        Serial.print(doc["x"].as<float>());
        Serial.print(" y: ");
        Serial.println(doc["y"].as<float>());
    } else if (doc["slider1"].as<int>() > 0) {
        Serial.println("Slider 1 data received");
        Serial.print(" Slider 1: ");
        Serial.println(doc["slider1"].as<int>());
    } else if (doc["slider2"].as<int>() > 0) {
        Serial.println("Slider 2 data received");
        Serial.print(" Slider 2: ");
        Serial.println(doc["slider2"].as<int>());
    } else if (doc["type"] == "switch") {
        Serial.println("Switch data received");
        Serial.print("Switch ID: ");
        Serial.print(doc["id"].as<int>());
        Serial.print(" Value: ");
        Serial.println(doc["value"].as<int>());
    } else if (doc["type"] == "push") {
        Serial.println("Push button data received");
        Serial.print("Push button ID: ");
        Serial.print(doc["id"].as<int>());
        Serial.print(" Value: ");
        Serial.println(doc["value"].as<int>());
    } else {
        Serial.println("Unknown data received");
    }
}

void onWebSocketEvent(uint8_t num, WStype_t type, uint8_t *payload, size_t length) {
  switch (type) {
    case WStype_DISCONNECTED:
      Serial.printf("[%u] Disconnected!\n", num);
      break;
    case WStype_CONNECTED: {
      IPAddress ip = webSocket.remoteIP(num);
      Serial.printf("[%u] Connection from ", num);
      Serial.println(ip.toString());
      break;
    }
    case WStype_TEXT:
      handleWebSocketMessage(payload, length);
      break;
    case WStype_BIN:
      Serial.printf("[%u] get binary length: %u\n", num, length);
      break;
    case WStype_PING:
      Serial.printf("[%u] Received PING\n", num);
      break;
    case WStype_PONG:
      Serial.printf("[%u] Received PONG\n", num);
      break;
    case WStype_ERROR:
      Serial.printf("[%u] WebSocket Error!\n", num);
      break;
  }
}

void setup() {
  Serial.begin(115200);

  // Connect to the external WiFi network (STA mode)
  WiFiMulti.addAP("WelcomingRouter", "VeryEasyPassword"); // Add your SSID and password

  // Wait for the ESP to connect to WiFi
  while (WiFiMulti.run() != WL_CONNECTED) {
    delay(100);
  }

  // Print the IP address 
  Serial.print("Connected to ");
  Serial.println(WiFi.SSID());
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  // Start mDNS service
  String hostname = "joystick";
  if (MDNS.begin(hostname.c_str())) {
    Serial.printf("mDNS responder started with hostname: %s.local\n", hostname.c_str());
  } else {
    Serial.println("Error setting up mDNS responder!");
  }

  // Set up AP mode
  WiFi.softAP("Joystick", "password123"); // Set your AP SSID and password
  IPAddress myIP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(myIP);

  // Start the HTTP server
  server.on("/", handleRoot);
  server.begin();
  Serial.println("HTTP server started");

  // Start the WebSocket server
  webSocket.begin();
  webSocket.onEvent(onWebSocketEvent);
  Serial.println("WebSocket server started");
}

void loop() {
  server.handleClient();
  webSocket.loop();
  
  #if BOARD_TYPE == ESP8266_BOARD
    // Handle mDNS queries at a reduced frequency
    unsigned long currentMillis = millis();
    if (currentMillis - lastMDNSUpdate >= MDNS_UPDATE_INTERVAL) {
        lastMDNSUpdate = currentMillis;
        MDNS.update();
    }
    #endif
}