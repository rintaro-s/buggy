import 'package:flutter/material.dart';
import 'package:flutter_hooks/flutter_hooks.dart';
import 'package:http/http.dart' as http;
import 'package:flutter_form_builder/flutter_form_builder.dart';

void main() {
  runApp(MyApp());
}

class MyApp extends StatelessWidget {
  @override
  Widget build(BuildContext context) {
    return MaterialApp(
      title: 'ESP32 Control App',
      theme: ThemeData(primarySwatch: Colors.blue),
      home: ControlPage(),
    );
  }
}

class ControlPage extends HookWidget {
  @override
  Widget build(BuildContext context) {
    final ipController = useTextEditingController(); // IPアドレス入力コントローラ
    final formKey = GlobalKey<FormBuilderState>();
    final speedIndex = useState(2); // 速度のインデックス（0: Front Fast, 1: Front Slow, 2: Stop, 3: Back Slow, 4: Back Fast）

    // 速度コマンドのリスト
    final speedCommands = [      'motor?direction=Front&speed=Fast',      'motor?direction=Front&speed=Slow',      'motor?direction=Stop&speed=Stop',      'motor?direction=Back&speed=Slow',      'motor?direction=Back&speed=Fast',    ];

    Future<void> sendCommand(String command) async {
      final ip = ipController.text;
      if (ip.isEmpty) {
        ScaffoldMessenger.of(context).showSnackBar(
          SnackBar(content: Text('Please enter the IP address')),
        );
        return;
      }
      final url = Uri.parse('http://$ip/$command');
      try {
        final response = await http.get(url);
        if (response.statusCode == 200) {
          print('Command sent: $command');
        } else {
          print('Failed to send command');
        }
      } catch (e) {
        print('Error: $e');
      }
    }

    return Scaffold(
      appBar: AppBar(
        title: Text('ESP32 Control App'),
      ),
      body: Row(
        children: [
          // 左側にスライダー
          Expanded(
            flex: 1,
            child: Column(
              mainAxisAlignment: MainAxisAlignment.center,
              children: [
                Text('Speed: ${['Front Fast', 'Front Slow', 'Stop', 'Back Slow', 'Back Fast'][speedIndex.value]}'),
                RotatedBox(
                  quarterTurns: 3, // 縦にするために90度回転
                  child: Slider(
                    value: speedIndex.value.toDouble(),
                    min: 0.0,
                    max: 4.0,
                    divisions: 4,
                    label: ['Front Fast', 'Front Slow', 'Stop', 'Back Slow', 'Back Fast'][speedIndex.value],
                    onChanged: (double newValue) {
                      speedIndex.value = newValue.toInt();
                      sendCommand(speedCommands[speedIndex.value]); // 速度更新コマンド
                    },
                  ),
                ),
              ],
            ),
          ),

          // 右側にIPアドレス入力とボタン
          Expanded(
            flex: 1,
            child: Column(
              children: [
                // IPアドレス入力
                Padding(
                  padding: const EdgeInsets.all(8.0),
                  child: FormBuilder(
                    key: formKey,
                    child: FormBuilderTextField(
                      name: 'ip',
                      controller: ipController,
                      decoration: InputDecoration(labelText: 'ESP32 IP Address'),
                      keyboardType: TextInputType.url,
                    ),
                  ),
                ),

                // 方向転換のボタン
                Row(
                  mainAxisAlignment: MainAxisAlignment.center,
                  children: [
                    ElevatedButton(
                      onPressed: () => sendCommand('servo?direction=left2'),
                      child: Text('Left 2'),
                    ),
                    SizedBox(width: 10),
                    ElevatedButton(
                      onPressed: () => sendCommand('servo?direction=center'),
                      child: Text('front'),
                    ),
                    SizedBox(width: 10),
                    ElevatedButton(
                      onPressed: () => sendCommand('servo?direction=right2'),
                      child: Text('Right 2'),
                    ),
                  ],
                ),
                SizedBox(height: 20),
                Row(
                  mainAxisAlignment: MainAxisAlignment.center,
                  children: [
                    ElevatedButton(
                      onPressed: () => sendCommand('servo?direction=left1'),
                      child: Text('Left 1'),
                    ),
                    SizedBox(width: 20),
                    ElevatedButton(
                      onPressed: () => sendCommand('servo?direction=right1'),
                      child: Text('Right 1'),
                    ),
                  ],
                ),
              ],
            ),
          ),
        ],
      ),
    );
  }
}
