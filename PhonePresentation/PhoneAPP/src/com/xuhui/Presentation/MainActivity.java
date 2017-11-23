package com.xuhui.Presentation;

import android.app.Activity;
import android.hardware.Sensor;
import android.hardware.SensorEvent;
import android.hardware.SensorEventListener;
import android.hardware.SensorManager;
import android.os.Bundle;
import android.text.Editable;
import android.text.TextWatcher;
import android.util.Log;
import android.view.View;
import android.widget.EditText;
import android.widget.TextView;
import android.widget.Toast;

import java.io.BufferedWriter;
import java.io.IOException;
import java.io.OutputStreamWriter;
import java.net.Socket;
import java.text.DecimalFormat;

public class MainActivity extends Activity {

    private TextView tv_data;
    private EditText et_ip;
    private EditText et_port;
    private Socket socket;
    private SensorManager sensorManager;
    private Sensor orientationSensor;
    private MySensorEventListener listener;

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.main);
        tv_data = (TextView) findViewById(R.id.tv_data);
        et_ip = (EditText) findViewById(R.id.et_ip);
        et_port = (EditText) findViewById(R.id.et_port);
        tv_data.addTextChangedListener(DataWatcher);

        sensorManager = (SensorManager) getSystemService(SENSOR_SERVICE);
        orientationSensor = sensorManager.getDefaultSensor(Sensor.TYPE_ORIENTATION);
        listener = new MySensorEventListener();
    }

    private TextWatcher DataWatcher = new TextWatcher() {
        @Override
        public void beforeTextChanged(CharSequence s, int start, int count, int after) {
        }

        @Override
        public void onTextChanged(CharSequence s, int start, int before, int count) {
        }

        @Override
        public void afterTextChanged(Editable s) {
            new Thread() {
                @Override
                public void run() {
                    super.run();
                    try {
                        if (socket != null) {
                            BufferedWriter writer = new BufferedWriter(new OutputStreamWriter(
                                    socket.getOutputStream()));
                            String socketData = tv_data.getText().toString();
                            writer.write(socketData.replace("\n", " ") + "\n");
                            writer.flush();
                            Log.e("服务器收到消息", "服务器收到消息");
                        }
                    } catch (Exception e) {
                        e.printStackTrace();
                        Log.e("服务没有收到器消息", "!!!!!");
                    }
                }
            }.start();
        }
    };

    public void Connect(View view) {
        sensorManager.registerListener(listener, orientationSensor, SensorManager.SENSOR_DELAY_NORMAL);
        new Thread() {
            @Override
            public void run() {
                super.run();
                try {
                    String address = et_ip.getText().toString();
                    int num = Integer.parseInt(et_port.getText().toString());
                    socket = new Socket(address, num);
                } catch (Exception e) {
                    e.printStackTrace();
                }
            }
        }.start();
    }

    public void Disconnect(View view) {
        try {
            sensorManager.unregisterListener(listener);
            if (socket != null) {
                socket.close();
                Toast.makeText(this, "已断开", Toast.LENGTH_SHORT);
            }
        } catch (IOException e) {
            e.printStackTrace();
        }
    }

    private final class MySensorEventListener implements SensorEventListener {

        @Override
        public void onSensorChanged(SensorEvent event) {
            double a = -event.values[1];
            double b = event.values[2];
            //double c = event.values[0];
            if (a < 0) {
                a += 360;
            }
            DecimalFormat decimalFormat = (DecimalFormat) DecimalFormat.getInstance();
            decimalFormat.applyPattern("000.00");
            tv_data.setText(decimalFormat.format(a) + " " + decimalFormat.format(b));
            //input.setText(decimalFormat.format(a) + " " + decimalFormat.format(b) + " " + decimalFormat.format(c));
        }

        @Override
        public void onAccuracyChanged(Sensor sensor, int accuracy) {

        }
    }
}
