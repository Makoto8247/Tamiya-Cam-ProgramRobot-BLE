import 'dart:async';
import 'dart:io';

import 'package:flutter/material.dart';
import 'package:flutter_blue_plus/flutter_blue_plus.dart';
import 'package:permission_handler/permission_handler.dart';

void main() {
  if (Platform.isAndroid) {
    WidgetsFlutterBinding.ensureInitialized();
    [
      Permission.location,
      Permission.storage,
      Permission.bluetooth,
      Permission.bluetoothConnect,
      Permission.bluetoothScan,
    ].request().then((status) {
      runApp(MainApp());
    });
  } else {
    runApp(MainApp());
  }
}

class HomePage extends StatefulWidget {
  const HomePage({Key? key}) : super(key: key);

  @override
  State<HomePage> createState() => _HomePageState();
}

class _HomePageState extends State<HomePage> {
  final double icon_size = 50;
  double icon_space = 10;
  bool isConnect = false;
  bool isFoward = false;
  bool isBack = false;
  bool isLeft = false;
  bool isRight = false;

  AppBar header() {
    return AppBar(
      title: Text(
        'Motor BLE App',
        style: TextStyle(
          color: Colors.white,
          fontWeight: FontWeight.bold,
        ),
      ),
      backgroundColor: Colors.blue,
    );
  }

  /* --- BLE connect --- */
  FlutterBluePlus flutterBluePlus = FlutterBluePlus.instance;
  final String SERVICE_UUID = "567a56ff-42a7-474a-9dc4-103b31a73a81";
  final String CHARACTERISTIC_UUID = "207023dc-2496-4f3d-9f71-caa1a912b11b";
  final String TARGET_DEVICE_NAME = "MotorBLE";
  StreamSubscription<ScanResult>? scanSubScription;

  String connectionText = "NO CONNECT";

  BluetoothDevice? targetDevice;
  BluetoothCharacteristic? targetCharacteristic;

  void onConnect(bool e) {
    if (!isConnect) {
      // Device Scan
      setState(() {
        connectionText = "Start Scanning";
      });

      scanSubScription = flutterBluePlus
          .scan(timeout: const Duration(seconds: 4))
          .listen((scanResult) {
        if (scanResult.device.name == TARGET_DEVICE_NAME) {
          stopScan();
          setState(() {
            connectionText = "Found Target Device";
            targetDevice = scanResult.device;
          });

          connectToDevice();
        }
      }, onDone: stopScan);

      if (targetDevice != null) {
        setState(() {
          isConnect = true;
        });
      } else {
        setState(() {
          isConnect = false;
          connectionText = "NO CONNECT";
        });
      }
    } else {
      targetDevice?.disconnect();
      setState(() {
        targetDevice = null;
        scanSubScription = null;
        targetCharacteristic = null;
        isFoward = false;
        isBack = false;
        isLeft = false;
        isRight = false;
        isConnect = false;
        connectionText = "NO CONNECT";
      });
    }
  }

  void connectToDevice() async {
    if (targetDevice == null) return;

    setState(() {
      connectionText = "Device Connecting";
    });

    await targetDevice!.connect();
    setState(() {
      connectionText = "Device Connected";
    });

    discoverServices();
  }

  void discoverServices() async {
    if (targetDevice == null) return;

    List<BluetoothService> services = await targetDevice!.discoverServices();
    services.forEach((service) {
      if (service.uuid.toString() == SERVICE_UUID) {
        service.characteristics.forEach((characteristic) {
          if (characteristic.uuid.toString() == CHARACTERISTIC_UUID) {
            setState(() {
              targetCharacteristic = characteristic;
              connectionText = "All Ready with ${targetDevice!.name}";
              isConnect = true;
            });
          }
        });
      }
    });
  }

  void stopScan() {
    scanSubScription?.cancel();
    scanSubScription = null;
  }

  void writeData(Iterable<int> data) async {
    if (targetCharacteristic == null) return;
    List<int> list = List<int>.of(data);
    await targetCharacteristic!.write(list, withoutResponse: true);
  }

  /* --- Motor Button --- */
  void stopButton() {
    if (isConnect) {
      writeData([0x91]);
      setState(() {
        isFoward = false;
        isBack = false;
        isLeft = false;
        isRight = false;
      });
    }
  }

  void fowardButton() {
    if ((isConnect && !isFoward) && !(isBack || isLeft || isRight)) {
      writeData([0x92]);
      setState(() {
        isFoward = true;
      });
    }
  }

  void backButton() {
    if ((isConnect && !isBack) && !(isFoward || isLeft || isRight)) {
      writeData([0x95]);
      setState(() {
        isBack = true;
      });
    }
  }

  void leftButton() {
    if ((isConnect && !isLeft) && !(isFoward || isBack || isRight)) {
      writeData([0x94]);
      setState(() {
        isLeft = true;
      });
    }
  }

  void rightButton() {
    if ((isConnect && !isRight) && !(isFoward || isBack || isLeft)) {
      writeData([0x93]);
      setState(() {
        isRight = true;
      });
    }
  }

  @override
  Widget build(BuildContext context) {
    return Scaffold(
      appBar: header(),
      body: Column(
        children: [
          /* --- BLE connection switch --- */
          Row(
            mainAxisAlignment: MainAxisAlignment.end,
            children: [
              Text(
                'Connect : ',
                style: TextStyle(
                  fontSize: 30,
                ),
              ),
              Switch(
                value: isConnect,
                onChanged: onConnect,
              ),
            ],
          ),
          Text(
            connectionText,
            style: TextStyle(
              fontSize: 30,
            ),
          ),
          /* --- Arrow Button --- */
          Expanded(
            child: Container(
              width: double.infinity,
              child: Center(
                child: Column(
                  mainAxisSize: MainAxisSize.min,
                  children: <Widget>[
                    // foward
                    IconButton(
                      iconSize: icon_size,
                      isSelected: isFoward,
                      onPressed: fowardButton,
                      icon: const Icon(Icons.arrow_upward_outlined),
                      selectedIcon: const Icon(
                        Icons.arrow_upward,
                        color: Colors.lightBlueAccent,
                      ),
                    ),
                    SizedBox(
                      height: icon_space,
                    ),
                    Row(
                      mainAxisAlignment: MainAxisAlignment.center,
                      children: [
                        // left
                        IconButton(
                          iconSize: icon_size,
                          isSelected: isLeft,
                          onPressed: leftButton,
                          icon: const Icon(Icons.arrow_back_outlined),
                          selectedIcon: const Icon(
                            Icons.arrow_back,
                            color: Colors.lightBlueAccent,
                          ),
                        ),

                        SizedBox(
                          width: icon_space,
                        ),

                        // stop
                        IconButton(
                          iconSize: icon_size,
                          onPressed: stopButton,
                          icon: const Icon(
                            Icons.cancel,
                            color: Colors.red,
                          ),
                        ),

                        SizedBox(
                          width: icon_space,
                        ),

                        // right
                        IconButton(
                          iconSize: icon_size,
                          isSelected: isRight,
                          onPressed: rightButton,
                          icon: const Icon(Icons.arrow_forward_outlined),
                          selectedIcon: const Icon(
                            Icons.arrow_forward,
                            color: Colors.lightBlueAccent,
                          ),
                        ),
                      ],
                    ),
                    SizedBox(
                      height: icon_space,
                    ),
                    // back
                    IconButton(
                      iconSize: icon_size,
                      isSelected: isBack,
                      onPressed: backButton,
                      icon: const Icon(Icons.arrow_downward_outlined),
                      selectedIcon: const Icon(
                        Icons.arrow_downward,
                        color: Colors.lightBlueAccent,
                      ),
                    ),
                  ],
                ),
              ),
            ),
          ),
        ],
      ),
    );
  }
}

class MainApp extends StatelessWidget {
  const MainApp({super.key});

  @override
  Widget build(BuildContext context) {
    return MaterialApp(
      theme: ThemeData(
        colorSchemeSeed: const Color(0xff6780a4),
        useMaterial3: true,
      ),
      home: HomePage(),
      debugShowCheckedModeBanner: false,
    );
  }
}
