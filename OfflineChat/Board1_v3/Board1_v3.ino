/*
  OfflineChat Board 1 - v3 - Minimal/Stable
  No libraries except built-in WiFi + WebServer
  HTML served in small PROGMEM chunks - fixes stack crash
*/
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>

ESP8266WebServer server(80);

#define MAX_MSG 8
String mSender[MAX_MSG];
String mText[MAX_MSG];
String mTime[MAX_MSG];
int mCount = 0, mHead = 0, mTotal = 0;

String uptime() {
  unsigned long s = millis() / 1000;
  char b[12]; sprintf(b, "%02lu:%02lu:%02lu", s/3600,(s%3600)/60,s%60);
  return String(b);
}

void addMsg(const String& s, const String& t) {
  int idx = (mHead + mCount) % MAX_MSG;
  if (mCount == MAX_MSG) mHead = (mHead+1)%MAX_MSG; else mCount++;
  mSender[idx]=s; mText[idx]=t; mTime[idx]=uptime(); mTotal++;
}

const char H1[] PROGMEM = "<!DOCTYPE html><html><head><meta charset='UTF-8'><meta name='viewport' content='width=device-width,initial-scale=1'><title>OfflineChat</title><style>*{box-sizing:border-box;margin:0;padding:0}body{font-family:sans-serif;background:#111;color:#eee;display:flex;flex-direction:column;height:100vh}";
const char H2[] PROGMEM = "#hdr{background:#0a2a0a;padding:10px 14px;border-bottom:2px solid #0f9;font-size:1.1em;color:#0f9}#chat{flex:1;overflow-y:auto;padding:10px;display:flex;flex-direction:column;gap:5px}.m{padding:7px 11px;border-radius:9px;max-width:78%;font-size:13px;word-break:break-word}";
const char H3[] PROGMEM = ".b1{background:#1a3a5c;align-self:flex-end}.b2{background:#1a3a1a;align-self:flex-start}.sy{background:#2a2a2a;align-self:center;font-size:11px;color:#999}.mt{font-size:10px;color:#555;margin-top:2px}#bar{display:flex;gap:6px;padding:8px;background:#181818;border-top:1px solid #2a2a2a}";
const char H4[] PROGMEM = "select{background:#222;color:#0f9;border:1px solid #0f9;border-radius:5px;padding:6px 8px;font-size:13px}input{flex:1;background:#222;color:#eee;border:1px solid #333;border-radius:5px;padding:6px 10px;font-size:13px;outline:none}input:focus{border-color:#0f9}button{background:#0f9;color:#000;border:none;border-radius:5px;padding:6px 14px;font-weight:bold;cursor:pointer}</style></head>";
const char H5[] PROGMEM = "<body><div id='hdr'>&#x1F4E1; OfflineChat</div><div id='chat'></div><div id='bar'><select id='nm'><option>Board1</option><option>Board2</option><option>Guest</option></select><input id='tx' placeholder='Message...' maxlength='120' onkeydown='if(event.key==\"Enter\")go()'><button onclick='go()'>Send</button></div>";
const char H6[] PROGMEM = "<script>var L=0;function e(s){return s.replace(/&/g,'&amp;').replace(/</g,'&lt;').replace(/>/g,'&gt;');}function load(){fetch('/msg').then(r=>r.json()).then(d=>{if(d.n===L)return;L=d.n;var c=document.getElementById('chat');c.innerHTML='';";
const char H7[] PROGMEM = "d.m.forEach(function(x){var v=document.createElement('div');var cl=x.s.startsWith('Board1')?'b1':x.s.startsWith('Board2')?'b2':'sy';v.className='m '+cl;v.innerHTML='<b>'+e(x.s)+'</b><br>'+e(x.t)+'<div class=\"mt\">'+e(x.ts)+'</div>';c.appendChild(v);});c.scrollTop=c.scrollHeight;}).catch(()=>{});}";
const char H8[] PROGMEM = "function go(){var t=document.getElementById('tx').value.trim();var n=document.getElementById('nm').value;if(!t)return;document.getElementById('tx').value='';fetch('/send',{method:'POST',headers:{'Content-Type':'application/x-www-form-urlencoded'},body:'s='+encodeURIComponent(n)+'&t='+encodeURIComponent(t)}).then(load);}load();setInterval(load,2000);</script></body></html>";

void handleRoot() {
  server.setContentLength(CONTENT_LENGTH_UNKNOWN);
  server.send(200, "text/html", "");
  server.sendContent_P(H1); yield();
  server.sendContent_P(H2); yield();
  server.sendContent_P(H3); yield();
  server.sendContent_P(H4); yield();
  server.sendContent_P(H5); yield();
  server.sendContent_P(H6); yield();
  server.sendContent_P(H7); yield();
  server.sendContent_P(H8); yield();
  server.sendContent("");
}

void handleMsg() {
  String j = "{\"n\":" + String(mTotal) + ",\"m\":[";
  for (int i = 0; i < mCount; i++) {
    int idx = (mHead+i)%MAX_MSG;
    if (i) j += ",";
    String txt = mText[idx]; txt.replace("\"","'");
    String snd = mSender[idx]; snd.replace("\"","'");
    j += "{\"s\":\""+snd+"\",\"t\":\""+txt+"\",\"ts\":\""+mTime[idx]+"\"}";
  }
  j += "]}";
  server.sendHeader("Access-Control-Allow-Origin","*");
  server.send(200,"application/json",j);
}

void handleSend() {
  String s = server.arg("s");
  String t = server.arg("t");
  if (!s.length()) s = "Guest";
  if (s.length() > 15) s = s.substring(0,15);
  if (t.length() > 120) t = t.substring(0,120);
  if (t.length()) addMsg(s,t);
  server.sendHeader("Access-Control-Allow-Origin","*");
  server.send(200,"application/json","{\"ok\":1}");
}

void setup() {
  Serial.begin(115200);
  delay(1000);
  Serial.println(F("\n=== OfflineChat v3 ==="));

  WiFi.persistent(false);
  WiFi.mode(WIFI_OFF);
  delay(200);
  WiFi.mode(WIFI_AP);
  delay(200);

  bool ok = WiFi.softAP("OfflineChat", "chat1234");
  Serial.println(ok ? F("AP OK") : F("AP FAILED"));
  Serial.print(F("IP: ")); Serial.println(WiFi.softAPIP());

  addMsg("System","OfflineChat ready!");

  server.on("/",     handleRoot);
  server.on("/msg",  handleMsg);
  server.on("/send", handleSend);
  server.begin();
  Serial.println(F("Ready! http://192.168.4.1"));
}

void loop() {
  server.handleClient();
  yield();
}
