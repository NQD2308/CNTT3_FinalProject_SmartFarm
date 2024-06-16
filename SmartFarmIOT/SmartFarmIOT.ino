#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <SoftwareSerial.h>
#include <DHT.h>

#define ssid "Nguyen 1"
#define password "07072011"

// #define ssid "HSU_Students"
// #define password "dhhs12cnvch"

// Thông tin về MQTT Broker
#define mqtt_server "broker.emqx.io"
const uint16_t mqtt_port = 1883;  //Port của MQTT broker
#define mqtt_topic_pub_led "engine"
#define mqtt_topic_sub_led "engine"
#define mqtt_topic_pub_temp "temp"
#define mqtt_topic_sub_temp "temp"
#define mqtt_topic_pub_humid "humid"
#define mqtt_topic_sub_humid "humid"
#define mqtt_topic_pub_test "test"
#define mqtt_topic_sub_test "test"

#define TX_PIN D7      //chân D7 arduino có chức năng của TX
#define RX_PIN D6      // chân D6 arduino có chức năng của RX
#define DHTPIN D5      // Define the DHT11 sensor pin
#define DHTTYPE DHT11  // Define the sensor type

DHT dht(DHTPIN, DHTTYPE);
SoftwareSerial bluetooth(RX_PIN, TX_PIN);

WiFiClient espClient;
PubSubClient client(espClient);
StaticJsonDocument<256> doc;  //PubSubClient limits the message size to 256 bytes (including header)

char engine[32] = "on";
void setup() {
  pinMode(D4, OUTPUT);
  digitalWrite(13, HIGH);
  Serial.begin(115200);
  bluetooth.begin(9600);

  pinMode(RX_PIN, INPUT);
  pinMode(TX_PIN, OUTPUT);
  dht.begin();
  Serial.println("Bluetooth On. Transmitting DHT11 sensor data...");

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
  strlcpy(engine, doc["ariconditioner"] | "on", sizeof(engine));
  String mystring(engine);
  //in ra tên của topic và nội dung nhận được từ kênh MQTT lens đã publish
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  // in trang thai cua led
  Serial.println(engine);


  if (mystring == "on") {  //on
    Serial.print("turn on");
    digitalWrite(D4, HIGH);
  } else {
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
    if (client.connect("smartfarm")) {
      Serial.println("connected");
      // publish gói tin "Hello esp8266!" đến topic mqtt_topic_pub_test

      char buffer[256];
      doc["message"] = "Hello esp8266!";
      size_t n = serializeJson(doc, buffer);
      client.publish(mqtt_topic_pub_test, buffer, n);
      // publish gói tin "{"message":"turn on led","name":"led","status":"on"}" đến topic mqtt_topic_pub_led
      doc["ariconditioner"] = "on";
      doc["message"] = "turn on ari conditioner";
      n = serializeJson(doc, buffer);
      client.publish(mqtt_topic_pub_led, buffer, n);
      // đăng kí nhận gói tin tại topic wemos/engine
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

  // Đọc dữ liệu từ Bluetooth
  float h = dht.readHumidity();
  float t = dht.readTemperature();

  delay(1000);

  // Check if any reads failed and exit early (to try again).
  if (isnan(h) || isnan(t)) {
    Serial.println("Failed to read from DHT sensor!");
    return;
  }

  if (bluetooth.available()) {
    // Send the data via Bluetooth
    bluetooth.print("Humidity: ");
    bluetooth.print(h);
    bluetooth.print(" %\t");
    bluetooth.print("Temperature: ");
    bluetooth.print(t);
    bluetooth.println(" *C");

    // Đóng gói và publish dữ liệu nhiệt độ lên MQTT
    doc.clear();
    doc["temp"] = t;
    char buffer[256];
    size_t n = serializeJson(doc, buffer);
    client.publish(mqtt_topic_pub_temp, buffer, n);

    // Đóng gói và publish dữ liệu độ ẩm lên MQTT
    doc.clear();
    doc["humid"] = h;
    n = serializeJson(doc, buffer);
    client.publish(mqtt_topic_pub_humid, buffer, n);
  } else {
    Serial.println("Not connect bluetooth");
  }

  bluetooth.print("Humidity: ");
  bluetooth.print(h);
  bluetooth.print(" %\t");
  bluetooth.print("Temperature: ");
  bluetooth.print(t);
  bluetooth.println(" *C");

  Serial.print("Published temp: ");
  Serial.println(t);
  Serial.print("Published humid: ");
  Serial.println(h);
}
