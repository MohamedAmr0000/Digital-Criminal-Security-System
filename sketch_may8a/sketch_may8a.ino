#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <AsyncTCP.h>
#include <Wire.h>
#include "MAX30105.h"
#include "heartRate.h"

MAX30105 particleSensor;

const char* ssid = "Mido";
const char* password = "2861947Wadida.";

AsyncWebServer server(80);
AsyncWebSocket ws("/ws");

long lastBeat = 0;
float beatsPerMinute = 0;
int beatAvg = 0;

void onEvent(AsyncWebSocket *server,
             AsyncWebSocketClient *client,
             AwsEventType type,
             void *arg,
             uint8_t *data,
             size_t len) {}

void setup() {

  Serial.begin(115200);

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) delay(500);

  Serial.println(WiFi.localIP());

  Wire.begin();

  if (!particleSensor.begin(Wire, I2C_SPEED_STANDARD)) {
    while (1);
  }

  particleSensor.setup();

  ws.onEvent(onEvent);
  server.addHandler(&ws);

  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {

String html = R"rawliteral(

<!DOCTYPE html>
<html>
<head>
<meta charset="UTF-8">
<title>ICU Heart Monitor</title>

<style>

body{
    margin:0;
    font-family:Arial;
    color:#00ffcc;
    background:#05080f;
    overflow:hidden;
    transition:1s;
}

/* الخلفية (تظهر فقط عند لمس السنسور) */
#bg{
    position:fixed;
    top:0;
    left:0;
    width:100%;
    height:100%;
    background:url('https://i.postimg.cc/8zGqQzFs/mee.jpg') no-repeat center center;
    background-size:cover;
    display:none;
    filter:brightness(0.5);
}

/* UI */
#ui{
    position:relative;
    text-align:center;
    padding-top:60px;
}

h1{
    font-size:28px;
    letter-spacing:2px;
    color:#00ffcc;
}

#bpm{
    font-size:60px;
    margin-top:20px;
}

/* القلب */
#heart{
    font-size:90px;
    color:red;
    display:none;
    margin-top:20px;
    animation:pulse 0.8s infinite;
}

@keyframes pulse{
    0%{transform:scale(1);}
    50%{transform:scale(1.4);}
    100%{transform:scale(1);}
}

/* ECG احترافي */
canvas{
    position:fixed;
    bottom:0;
    left:0;
    width:100%;
    height:180px;
}

.glow{
    text-shadow:0 0 10px #00ffcc;
}

</style>

</head>

<body>

<div id="bg"></div>

<div id="ui">

<h1>ICU HEART MONITOR</h1>

<div id="bpm">Waiting...</div>

<div id="heart">❤️</div>

</div>

<canvas id="ecg"></canvas>

<script>

var ws = new WebSocket('ws://' + location.host + '/ws');

let audio = new AudioContext();

function beep(){
    let o = audio.createOscillator();
    let g = audio.createGain();

    o.type = "sine";
    o.frequency.value = 750;
    g.gain.value = 0.05;

    o.connect(g);
    g.connect(audio.destination);

    o.start();
    o.stop(audio.currentTime + 0.1);
}

/* ECG */
let c = document.getElementById("ecg");
let ctx = c.getContext("2d");

c.width = window.innerWidth;
c.height = 180;

let x = 0;
let y = 90;

function draw(active){

    ctx.strokeStyle = active ? "red" : "#00ff88";
    ctx.lineWidth = 2;

    let spike = active && Math.random() > 0.85 ? -60 : 0;

    let ny = 90 + spike;

    ctx.beginPath();
    ctx.moveTo(x, y);
    ctx.lineTo(x+2, ny);
    ctx.stroke();

    y = ny;
    x += 2;

    if(x > c.width){
        ctx.clearRect(0,0,c.width,c.height);
        x = 0;
    }
}

setInterval(()=>draw(false),30);

ws.onmessage = function(event){

    let data = event.data;

    if(data.startsWith("FINGER_ON")){

        let bpm = data.split(":")[1];

        document.getElementById("bg").style.display = "block";
        document.getElementById("heart").style.display = "block";
        document.getElementById("bpm").innerHTML = bpm + " BPM";

        draw(true);
        beep();

    } else {

        document.getElementById("bg").style.display = "none";
        document.getElementById("heart").style.display = "none";
        document.getElementById("bpm").innerHTML = "No Finger";

    }
};

</script>

</body>
</html>

)rawliteral";

    request->send(200, "text/html", html);
  });

  server.begin();
}

void loop() {

  long irValue = particleSensor.getIR();

  static unsigned long lastSend = 0;

  if (irValue > 20000) {

    if (checkForBeat(irValue)) {

      long delta = millis() - lastBeat;
      lastBeat = millis();

      if (delta > 0) {
        beatsPerMinute = 60 / (delta / 1000.0);

        if (beatsPerMinute > 40 && beatsPerMinute < 180) {
          beatAvg = (int)beatsPerMinute;
        }
      }
    }

    if (millis() - lastSend >= 2000) {
      lastSend = millis();
      ws.textAll("FINGER_ON:" + String(beatAvg));
    }

  } else {

    if (millis() - lastSend >= 2000) {
      lastSend = millis();
      ws.textAll("FINGER_OFF:0");
    }
  }

  ws.cleanupClients();
}