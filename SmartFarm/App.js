import React, { useState, useEffect } from "react";
import {
  Image,
  StyleSheet,
  Platform,
  ScrollView,
  View,
  Text,
  SafeAreaView,
} from "react-native";
import { Picker } from "@react-native-picker/picker";
import { Switch } from "react-native-switch";
import Slider from "@react-native-community/slider";
import AsyncStorage from "@react-native-async-storage/async-storage";
import init from "react_native_mqtt";

init({
  size: 10000,
  storageBackend: AsyncStorage,
  defaultExpires: 1000 * 3600 * 24,
  enableCache: true,
  sync: {},
});
const options = {
  host: "broker.emqx.io",
  port: 8083,
  path: "smartfarm",
  id: "id_" + parseInt(Math.random() * 100000),
};

const client = new Paho.MQTT.Client(
  options.host,
  options.port,
  options.path + options.id
);
export default function HomeScreen() {
  const [msg, setMsg] = useState("No message");
  const [statusLed, setStatusLed] = useState("off");
  const [tempValue, setTempValue] = useState();
  const [humidValue, setHumidValue] = useState();
  const [selectedValue, setSelectedValue] = useState("manual");
  const [switchValueManual, setSwitchValueManual] = useState(false);
  const [switchValueAuto, setSwitchValueAuto] = useState(false);
  const [sliderValueTemp, setSliderValueTemp] = useState(23);

  // Tải lại nhiệt độ, độ ẩm mỗi 3s
  useEffect(() => {
    //step 1 connect Mqtt broker
    connect();
    // step 3 handling when message arrived
    client.onMessageArrived = onMessageArrived;

    fetchData();
  }, []);

  // Xử lý tự động hóa theo nhiệt độ
  useEffect(() => {
    let interval;
    if (switchValueAuto) {
      interval = setInterval(() => {
        if (tempValue < sliderValueTemp) {
          handleButtonOn();
        } else {
          handleButtonOff();
        }
      }, 1000); // Kiểm tra mỗi giây
    }
    return () => clearInterval(interval); // Xóa interval khi component unmount hoặc khi switchValueAuto thay đổi
  }, [switchValueAuto, tempValue, sliderValueTemp]);

  //kết nối lấy nhiệt độ, độ ẩm từ url docker
  const fetchData = async () => {
    try {
      const response = await fetch("http://192.168.1.14:5557/devices");
      const data = await response.json();
      if (data && data.length > 0) {
        const deviceData = data[0];
        setTempValue(parseFloat(deviceData.temperature));
        setHumidValue(parseFloat(deviceData.humidity));
        setSwitchValueManual(deviceData.airconditioner === "On");
      }
    } catch (error) {
      console.error("Error fetching data:", error);
    }
  };

  const connect = () => {
    client.connect({
      onSuccess: () => {
        console.log("connect MQTT broker ok!");
        //step 2 subscribe topic
        // subscribeTopic(); // ledstatus
        subscribeTempTopic();
      },
      useSSL: false,
      timeout: 5,
      onFailure: () => {
        console.log("connect fail");
        connect();
        console.log("reconnect ...");
      },
    });
  };

  const publishTopic = (deviceStatus) => {
    const s =
      '{"message":"turn on/off engine","name":"air conditioner","status":"' +
      deviceStatus +
      '"}';
    var message = new Paho.MQTT.Message(s);
    message.destinationName = "engine";
    client.send(message);
  };

  const subscribeTopic = () => {
    client.subscribe("engine", { qos: 0 });
  };

  const subscribeTempTopic = () => {
    client.subscribe("temp", { qos: 0 });
  };

  const onMessageArrived = async (message) => {
    console.log("onMessageArrived:" + message.payloadString);
    setMsg(message.payloadString);
    const jsondata = JSON.parse(message.payloadString);
    console.log(jsondata.message);
    setStatusLed(jsondata.status);
  };

  // Bật đèn thông qua MQTT và Bluetooth
  const handleButtonOn = async () => {
    console.log("turn on led...");

    fetch('http://192.168.1.14:3000/simpleGateway/on')
    .then(response => {
      if (response.ok) {
        console.log('Successfully turned on');
      } else {
        console.error('Failed to turn on');
      }
    })
    .catch(error => {
      console.error('Error turning on:', error);
    });

    publishTopic("On");
  };

  // Tắt đèn thông qua MQTT và Bluetooth
  const handleButtonOff = async () => {
    console.log("turn off led...");

    fetch('http://192.168.1.14:3000/simpleGateway/off')
    .then(response => {
      if (response.ok) {
        console.log('Successfully turned off');
      } else {
        console.error('Failed to turn off');
      }
    })
    .catch(error => {
      console.error('Error turning off:', error);
    });

    publishTopic("Off");
  };

  const handleSwitchChangeManual = (val) => {
    setSwitchValueManual(val);
    if (val) {
      setSwitchValueAuto(false);
      handleButtonOn();
    } else {
      handleButtonOff();
    }
  };

  const handleSwitchChangeAuto = (val) => {
    setSwitchValueAuto(val);
    if (val) {
      setSwitchValueManual(false);
      // Kiểm tra ngay lập tức khi bật công tắc tự động
      if (tempValue < sliderValueTemp) {
        handleButtonOn();
      } else {
        handleButtonOff();
      }
    } else {
      setSwitchValueManual(false);
      handleButtonOff();
    }
  };

  return (
    <SafeAreaView style={{ marginTop: 100 }}>
      <View style={styles.container}>
        <View style={styles.cart}>
          <View style={styles.topCart}>
            <View style={styles.titleCart}>
              <Text style={styles.titleTextCart}>Engine 1</Text>
            </View>
            <View style={styles.optionControl}>
              <Picker
                style={styles.optionPicker}
                selectedValue={selectedValue}
                onValueChange={(itemValue, itemIndex) =>
                  setSelectedValue(itemValue)
                }
              >
                <Picker.Item label="Manual" value="manual" />
                <Picker.Item label="Auto" value="auto" />
              </Picker>
            </View>
          </View>
          {selectedValue === "manual" ? (
            <View style={styles.contentCart}>
              <View style={styles.sensorValue}>
                <View style={styles.sensorValueText}>
                  <Text style={{ textAlign: "center" }}>
                    {Math.round(tempValue)}℃
                  </Text>
                  <Text>Temperature</Text>
                </View>
                <View style={styles.sensorValueText}>
                  <Text style={{ textAlign: "center" }}>
                    {Math.round(humidValue)}%
                  </Text>
                  <Text>Humidity</Text>
                </View>
              </View>
              <View style={styles.status}>
                <View style={styles.leftSide}>
                  <Text>Status</Text>
                  <Text
                    style={[
                      {
                        color: switchValueManual ? "#2D642C" : "#B13232",
                        fontWeight: "bold",
                      },
                    ]}
                  >
                    {switchValueManual ? "On" : "Off"}
                  </Text>
                </View>
                <View style={styles.rightSide}>
                  <Switch
                    value={switchValueManual}
                    onValueChange={handleSwitchChangeManual}
                    disabled={false}
                    barHeight={32}
                    circleSize={26}
                    circleBorderActiveColor="#fff"
                    circleBorderInactiveColor="#fff"
                    backgroundActive={"#2D642C"}
                    backgroundInactive={"#828282"}
                    circleActiveColor={"#fff"}
                    circleInActiveColor={"#fff"}
                    changeValueImmediately={true}
                    innerCircleStyle={{
                      alignItems: "center",
                      justifyContent: "center",
                    }}
                    renderInActiveText={false}
                    renderActiveText={false}
                    switchLeftPx={2.6}
                    switchRightPx={2.6}
                    switchBorderRadius={30}
                  />
                </View>
              </View>
            </View>
          ) : (
            <View style={styles.contentCart}>
              <View style={styles.sensorValue}>
                <View style={styles.sensorValueText}>
                  <Text style={{ textAlign: "center" }}>
                    {Math.round(tempValue)}℃
                  </Text>
                  <Text>Temperature</Text>
                </View>
                <View style={styles.sensorValueText}>
                  <Text style={{ textAlign: "center" }}>
                    {Math.round(humidValue)}%
                  </Text>
                  <Text>Humidity</Text>
                </View>
              </View>
              <View style={styles.status}>
                <View style={styles.leftSide}>
                  <Text>Status</Text>
                  <Text
                    style={[
                      {
                        color: switchValueAuto ? "#2D642C" : "#B13232",
                        fontWeight: "bold",
                      },
                    ]}
                  >
                    {switchValueAuto ? "On" : "Off"}
                  </Text>
                </View>
                <View style={styles.rightSide}>
                  <Switch
                    value={switchValueAuto}
                    onValueChange={handleSwitchChangeAuto}
                    disabled={false}
                    barHeight={32}
                    circleSize={26}
                    circleBorderActiveColor="#fff"
                    circleBorderInactiveColor="#fff"
                    backgroundActive={"#2D642C"}
                    backgroundInactive={"#828282"}
                    circleActiveColor={"#fff"}
                    circleInActiveColor={"#fff"}
                    changeValueImmediately={true}
                    // changeValueImmediately={true} // if rendering inside circle, change state immediately or wait for animation to complete
                    innerCircleStyle={{
                      alignItems: "center",
                      justifyContent: "center",
                    }} // style for inner animated circle for what you (may) be rendering inside the circle
                    renderActiveText={false}
                    renderInActiveText={false}
                    switchLeftPx={2.6} // denominator for logic when sliding to TRUE position. Higher number = more space from RIGHT of the circle to END of the slider
                    switchRightPx={2.6} // denominator for logic when sliding to FALSE position. Higher number = more space from LEFT of the circle to BEGINNING of the slider
                    // switchWidthMultiplier={2} // multiplied by the `circleSize` prop to calculate total width of the Switch
                    switchBorderRadius={30}
                  />
                </View>
              </View>
              <View style={styles.customValue}>
                <Text>Temperature</Text>
                <Slider
                  style={{ width: 200, height: 40 }}
                  value={sliderValueTemp}
                  onValueChange={(val) => setSliderValueTemp(val)}
                  minimumValue={0}
                  maximumValue={30}
                  minimumTrackTintColor="#2D642C"
                  maximumTrackTintColor="#000000"
                  thumbTintColor="#2D642C"
                />
                <Text style={styles.tempText}>
                  {Math.round(sliderValueTemp)}℃
                </Text>
              </View>
            </View>
          )}
        </View>
      </View>
    </SafeAreaView>
  );
}

const styles = StyleSheet.create({
  text_center: {
    textAlign: "center",
  },
  titleContainer: {
    flexDirection: "row",
    alignItems: "center",
    gap: 8,
  },
  stepContainer: {
    gap: 8,
    marginBottom: 8,
  },
  reactLogo: {
    flex: 1,
    top: -150,
    left: 0,
    zIndex: 0,
    position: "absolute",
  },
  rectangle: {
    bottom: 0,
    position: "absolute",
  },
  container: {
    justifyContent: "center",
    alignItems: "center",
  },
  cart: {
    width: "100%",
    backgroundColor: "white",
    borderRadius: 12,
    shadowColor: "#000",
    shadowOffset: {
      width: 0,
      height: 6,
    },
    shadowOpacity: 0.37,
    shadowRadius: 7.49,
    elevation: 12,
  },
  topCart: {
    flexDirection: "row",
    justifyContent: "space-between",
    marginBottom: 5,
  },
  titleCart: {
    alignItems: "center",
    justifyContent: "center",
    paddingTop: 2,
    paddingBottom: 2,
    paddingLeft: 16,
    paddingRight: 16,
    borderTopLeftRadius: 12,
    borderBottomRightRadius: 16,
    backgroundColor: "rgba(71, 119, 70, 0.3)",
  },
  titleTextCart: {
    textAlign: "center",
    justifyContent: "center",
    fontWeight: "bold",
  },
  optionControl: {
    paddingTop: 2,
    paddingBottom: 2,
    paddingLeft: 12,
    paddingRight: 12,
    backgroundColor: "#F1F1F1",
    borderTopRightRadius: 12,
    borderBottomLeftRadius: 16,
  },
  optionPicker: {
    width: 140,
    height: "auto",
    color: "black",
  },
  contentCart: {
    marginTop: 5,
    marginBottom: 5,
  },
  sensorValue: {
    flexDirection: "row",
    justifyContent: "space-around",
    marginTop: 10,
    marginBottom: 20,
  },
  sensorValueText: {
    justifyContent: "center",
    alignItems: "center",
  },
  status: {
    flexDirection: "row",
    justifyContent: "space-between",
    alignItems: "center",
    marginTop: 5,
    marginBottom: 5,
    paddingLeft: 10,
    paddingRight: 10,
  },
  leftSide: {
    flexDirection: "row",
    alignItems: "center",
    justifyContent: "center",
    gap: 16,
  },
  rightSide: {},
  customValue: {
    flexDirection: "row",
    justifyContent: "space-between",
    alignItems: "center",
    paddingLeft: 10,
    paddingRight: 10,
    marginTop: 5,
    marginBottom: 5,
  },
  tempText: {
    flex: 1,
    textAlign: "center",
  },
});
