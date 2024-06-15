#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#define ssid "HSU_Students"
#define password "dhhs12cnvch"
// Thông tin về MQTT Broker
#define mqtt_server "broker.emqx.io"
const uint16_t mqtt_port = 8083; //Port của MQTT broker
#define mqtt_topic_pub_led "ledstatus"
#define mqtt_topic_sub_led "ledstatus"
#define mqtt_topic_pub_temp "temp"
#define mqtt_topic_sub_temp "temp"
#define mqtt_topic_pub_humid "humid"
#define mqtt_topic_sub_humid "humid"
#define mqtt_topic_pub_test "test"
#define mqtt_topic_sub_test "test"
WiFiClient espClient;
PubSubClient client(espClient);
StaticJsonDocument<256> doc; //PubSubClient limits the message size to 256 bytes (including header)
char ledstatus[32]="on";
void setup() {
 pinMode(D4, OUTPUT);
 digitalWrite(13, HIGH);
 Serial.begin(115200);
 // hàm thực hiện chức năng kết nối Wifi và in ra địa chỉ IP của ESP8266
 setup_wifi();
 // cài đặt server eclispe mosquitto / mqttx và lắng nghe client ở port 1883
 client.setServer(mqtt_server, mqtt_port);
 // gọi hàm callback để thực hiện các chức năng publish/subcribe
 client.setCallback(callback);
 // gọi hàm reconnect() để thực hiện kết nối lại với server khi bị mất kết nối
 reconnect();
}
void setup_wifi() {
 delay(10);
 Serial.println();
 Serial.print("Connecting to ");
 Serial.println(ssid);
 // kết nối đến mạng Wifi
 WiFi.begin(ssid, password);
 // in ra dấu . nếu chưa kết nối được đến mạng Wifi
 while (WiFi.status() != WL_CONNECTED) {
 delay(500);
 Serial.print(".");
 }
 // in ra thông báo đã kết nối và địa chỉ IP của ESP8266
 Serial.println("");
 Serial.println("WiFi connected");
 Serial.println("IP address: ");
 Serial.println(WiFi.localIP());
}
void callback(char* topic, byte* payload, unsigned int length) {
 //chuyen doi *byte sang json
 deserializeJson(doc, payload, length);
 //doc thong tin status tu chuỗi json trả về
 strlcpy(ledstatus, doc["status"] | "on", sizeof(ledstatus));
 String mystring(ledstatus);
 //in ra tên của topic và nội dung nhận được từ kênh MQTT lens đã publish
 Serial.print("Message arrived [");
 Serial.print(topic);
 Serial.print("] ");
 // in trang thai cua led
 Serial.println(ledstatus);


 if(mystring == "on")
 { //on
 Serial.print("turn on");
 digitalWrite(D4, HIGH);
 }else
 {
 Serial.print("turn off");
 digitalWrite(D4, LOW);
 }
 Serial.println();
}
void reconnect() {
 // lặp cho đến khi được kết nối trở lại
 while (!client.connected()) {
 Serial.print("Attempting MQTT connection...");
 // hàm connect có đối số thứ 1 là id đại diện cho mqtt client, đối số thứ 2 là username và đối số thứ 3 là password
 if (client.connect("mqttcontrolhome")) {
 Serial.println("connected");
 // publish gói tin "Hello esp8266!" đến topic mqtt_topic_pub_test

 char buffer[256];
 doc["message"] = "Hello esp8266!";
 size_t n = serializeJson(doc, buffer);
 client.publish(mqtt_topic_pub_test, buffer, n);
 // publish gói tin "{"message":"turn on led","name":"led","status":"on"}" đến topic mqtt_topic_pub_led
 doc["name"] = "led";
 doc["status"] = "on";
 doc["message"] = "turn on led";
 n=serializeJson(doc, buffer);
 client.publish(mqtt_topic_pub_led, buffer, n);
 // đăng kí nhận gói tin tại topic wemos/ledstatus
 client.subscribe(mqtt_topic_sub_led);
 } else {
 // in ra màn hình trạng thái của client khi không kết nối được với MQTT broker
 Serial.print("failed, rc=");
 Serial.print(client.state());
 Serial.println(" try again in 5 seconds");
 // delay 5s trước khi thử lại
 delay(5000);
 }
 }
}
void loop() {
 // kiểm tra nếu ESP8266 chưa kết nối được thì sẽ thực hiện kết nối lại
 if (!client.connected()) {
 reconnect();
 }
 client.loop();
}
