package com.dibya.arduinousb;

import android.annotation.SuppressLint;
import android.app.AlertDialog;
import android.app.PendingIntent;
import android.app.ProgressDialog;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.DialogInterface;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.SharedPreferences;
import android.hardware.usb.UsbDevice;
import android.hardware.usb.UsbDeviceConnection;
import android.hardware.usb.UsbManager;
import android.os.Bundle;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.View;
import android.widget.LinearLayout;
import android.widget.TextView;
import android.widget.Toast;

import androidx.appcompat.app.AppCompatActivity;

import com.felhr.usbserial.UsbSerialDevice;
import com.felhr.usbserial.UsbSerialInterface;
import com.google.android.material.button.MaterialButton;
import com.google.android.material.snackbar.Snackbar;
import com.google.android.material.textfield.TextInputEditText;

import org.json.JSONArray;
import org.json.JSONException;
import org.json.JSONObject;

import java.nio.charset.StandardCharsets;
import java.util.HashMap;
import java.util.Map;
import java.util.Objects;

public class EnrollActivity extends AppCompatActivity {
    TextInputEditText editTextStdBranch;
    TextInputEditText editTextStdName;
    TextInputEditText editTextStdSec;
    TextInputEditText editTextStdid;
    TextView responseData;
    MaterialButton enrollnow;
    MaterialButton load;
    MaterialButton save;
    LinearLayout mainLayout;
    TextView result_data;

    UsbManager usbManager;
    UsbDevice device;
    UsbSerialDevice serialPort;
    UsbDeviceConnection connection;
    public final String ACTION_USB_PERMISSION = "com.dibya.arduinousb.USB_PERMISSION";
    SharedPreferences preferences;
    SharedPreferences.Editor editor;
    DataBaseHelper dataBaseHelper;
    private MaterialButton takeAttendance;
    private int id = 0;
    private String response;
    ProgressDialog progressDialog;

    UsbSerialInterface.UsbReadCallback mCallback = new UsbSerialInterface.UsbReadCallback() { //Defining a Callback which triggers whenever data is read.
        @Override
        public void onReceivedData(byte[] arg0) {
            String data = null;
            try {
                data = new String(arg0, StandardCharsets.UTF_8);
                data.concat("/n");
                if (progressDialog.isShowing()) {
                    progressDialog.dismiss();
                }
                tvAppend(responseData, data);
            } catch (Exception e) {
                e.printStackTrace();
            }
        }
    };

    private final BroadcastReceiver broadcastReceiver = new BroadcastReceiver() { //Broadcast Receiver to automatically start and stop the Serial connection.
        @Override
        public void onReceive(Context context, Intent intent) {
            if (intent.getAction().equals(ACTION_USB_PERMISSION)) {
                boolean granted = Objects.requireNonNull(intent.getExtras()).getBoolean(UsbManager.EXTRA_PERMISSION_GRANTED);
                if (granted) {
                    connection = usbManager.openDevice(device);
                    serialPort = UsbSerialDevice.createUsbSerialDevice(device, connection);
                    if (serialPort != null) {
                        if (serialPort.open()) { //Set Serial Connection Parameters.
                            serialPort.setBaudRate(9600);
                            serialPort.setDataBits(UsbSerialInterface.DATA_BITS_8);
                            serialPort.setStopBits(UsbSerialInterface.STOP_BITS_1);
                            serialPort.setParity(UsbSerialInterface.PARITY_NONE);
                            serialPort.setFlowControl(UsbSerialInterface.FLOW_CONTROL_OFF);
                            serialPort.read(mCallback);
                            Snackbar snackbar = Snackbar.make(mainLayout, "Serial Connection Opened!", Snackbar.LENGTH_SHORT);
                            snackbar.show();

                        } else {
                            Log.d("SERIAL", "PORT NOT OPEN");
                        }
                    } else {
                        Log.d("SERIAL", "PORT IS NULL");
                    }
                } else {
                    Log.d("SERIAL", "PERM NOT GRANTED");
                }
            } else if (intent.getAction().equals(UsbManager.ACTION_USB_DEVICE_ATTACHED)) {
                enrollnow.performClick();
            } else if (intent.getAction().equals(UsbManager.ACTION_USB_DEVICE_DETACHED)) {
                enrollnow.performClick();
            }
        }
    };

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.enroll_layout);
        usbManager = (UsbManager) getSystemService(USB_SERVICE);
        editTextStdBranch = findViewById(R.id.editTextStdBranch);
        editTextStdName = findViewById(R.id.editTextStdName);
        editTextStdSec = findViewById(R.id.editTextStdSec);
        enrollnow = findViewById(R.id.enrollnow);
        responseData = findViewById(R.id.response_data);
        load = findViewById(R.id.load);
        save = findViewById(R.id.save);
        takeAttendance = findViewById(R.id.takeattendance);
        mainLayout = findViewById(R.id.mainLayout);
        result_data = findViewById(R.id.result_data);

        progressDialog = new ProgressDialog(this);
        progressDialog.setTitle("Connecting to device...");
        progressDialog.setMessage("Please wait...");

        responseData.setVisibility(View.GONE);
        IntentFilter filter = new IntentFilter();
        filter.addAction(ACTION_USB_PERMISSION);
        filter.addAction(UsbManager.ACTION_USB_DEVICE_ATTACHED);
        filter.addAction(UsbManager.ACTION_USB_DEVICE_DETACHED);
        registerReceiver(broadcastReceiver, filter);
        dataBaseHelper = new DataBaseHelper(EnrollActivity.this);

        HashMap<String, UsbDevice> usbDevices = usbManager.getDeviceList();
        if (!usbDevices.isEmpty()) {
            boolean keep = true;
            for (Map.Entry<String, UsbDevice> entry : usbDevices.entrySet()) {
                device = entry.getValue();
                int deviceVID = device.getVendorId();
                if (deviceVID == 0x2341)//Arduino Vendor ID
                {
                    PendingIntent pi = PendingIntent.getBroadcast(EnrollActivity.this, 0, new Intent(ACTION_USB_PERMISSION), 0);
                    usbManager.requestPermission(device, pi);
                    keep = false;
                } else {
                    connection = null;
                    device = null;
                }
                if (!keep)
                    break;
            }
        }

        enrollnow.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                enrollNow();
                editTextStdBranch.setText("");
                editTextStdName.setText("");
                editTextStdSec.setText("");
            }
        });

        save.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                String s = responseData.getText().toString();
                String id = "\"id\":";
                int q = s.indexOf(id) + id.length();
                int sid = Integer.parseInt(s.substring(q, s.indexOf(",\"", q)));
                String key = "\"fp_packet\":";
                int p = s.indexOf(key) + key.length();
                String sr = (s.substring(p, s.indexOf("}")));
                long add = dataBaseHelper.enrollStudent(sid, (editTextStdSec.getText()).toString(),
                        (editTextStdBranch.getText()).toString(), sr);
                if (add != -1) {
                    Snackbar snackbar = Snackbar.make(mainLayout, "Data stored Successfully!", Snackbar.LENGTH_SHORT);
                    snackbar.show();
                    responseData.setText("");
                } else {
                    Snackbar snackbar = Snackbar.make(mainLayout, "Something went wrong!", Snackbar.LENGTH_SHORT);
                    snackbar.show();
                }
            }
        });

        takeAttendance.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                JSONArray fpArray = new JSONArray();
                final JSONObject student = new JSONObject();
                for (int i = 0; i < 512; i++) {
                    fpArray.put(0);
                }
                try {
                    student.put("op", 3);
                    student.put("id", 0);
                    student.accumulate("fp_packet", fpArray);
                    serialPort.write(String.valueOf(student).getBytes());
                } catch (JSONException e) {
                    Log.d("JSONException", String.valueOf(e));
                    Toast.makeText(EnrollActivity.this, String.valueOf(e), Toast.LENGTH_SHORT).show();
                }
            }
        });

        load.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                @SuppressLint("InflateParams") View sub_view = LayoutInflater.from(EnrollActivity.this).inflate(R.layout.loadattendance, null);
                AlertDialog.Builder alertdialog_sub = new AlertDialog.Builder(EnrollActivity.this);

                alertdialog_sub.setView(sub_view);
                alertdialog_sub.setTitle("Load Attendance");
                final TextInputEditText branch = findViewById(R.id.editTextStdBranch);
                final TextInputEditText section = findViewById(R.id.editTextStdSec);

                Log.d("branch", Objects.requireNonNull(branch.getText()).toString());

                alertdialog_sub.setPositiveButton("LOAD ATTENDANCE", new DialogInterface.OnClickListener() {
                    @Override
                    public void onClick(DialogInterface dialog_sub, int which_sub) {
                        AttendanceDetails[] attendanceDetails = dataBaseHelper.getData(branch.getText().toString(), Objects.requireNonNull(section.getText()).toString());
                        Toast.makeText(EnrollActivity.this, String.valueOf(attendanceDetails.length), Toast.LENGTH_SHORT).show();
//                        for (int i = 0; i < attendanceDetails.length; i++) {
//                            try {
//                                JSONObject loadData = new JSONObject();
//                                JSONArray jsonArray = new JSONArray();
//                                loadData.put("op", 2);
//                                loadData.put("id", attendanceDetails[i].getId());
//                                String s = attendanceDetails[i].getFp_packet();
//                                String sr = s.substring(s.indexOf('[') + 1, s.indexOf(']'));
//                                String[] strarr = sr.split(",");
//                                int[] arr = new int[strarr.length];
//                                for (int j = 0; j < strarr.length; j++) {
//                                    arr[j] = Integer.parseInt(strarr[j]);
//                                    jsonArray.put(Integer.parseInt(strarr[j]));
//                                }
//                                loadData.put("fp_packet", jsonArray);
//                                serialPort.write(String.valueOf(loadData).getBytes());
                                //need get the response
//                                String res = responseData.getText().toString();
//                                Toast.makeText(EnrollActivity.this, res, Toast.LENGTH_SHORT).show();
//                                if (!res.matches("")) {
//                                    String id = "\"staus_code\":";
//                                    int q = s.indexOf(id) + id.length();
//                                    int sid = Integer.parseInt(s.substring(q, s.indexOf("}", q)));
//                                    if (sid == 0) {
//                                        Snackbar snackbar = Snackbar.make(mainLayout, "Data loaded to device successfully!", Snackbar.LENGTH_SHORT);
//                                        snackbar.show();
//                                    } else {
//                                        Snackbar snackbar = Snackbar.make(mainLayout, "Something went wrong!", Snackbar.LENGTH_SHORT);
//                                        snackbar.show();
//                                    }
//                                }
//                            } catch (JSONException e) {
//                                Toast.makeText(EnrollActivity.this, String.valueOf(e), Toast.LENGTH_SHORT).show();
//                            }
//                        }
//                        if (cursor.moveToFirst()) {
//                            do {
//                                try {
//                                    JSONObject loadData = new JSONObject();
//                                    JSONArray jsonArray = new JSONArray();
//                                    loadData.put("op", 2);
//                                    loadData.put("id", Integer.parseInt(cursor.getString(cursor.getColumnIndex(AttendanceDetails.STUDENT_ID))));
//                                    String s = cursor.getString(cursor.getColumnIndex(AttendanceDetails.FP_PACKET));
//                                    String sr = s.substring(s.indexOf('[') + 1, s.indexOf(']'));
//                                    String[] strarr = sr.split(",");
//                                    int[] arr = new int[strarr.length];
//                                    for (int i = 0; i < strarr.length; i++) {
//                                        arr[i] = Integer.parseInt(strarr[i]);
//                                        jsonArray.put(Integer.parseInt(strarr[i]));
//                                    }
//                                    loadData.put("fp_packet", jsonArray);
//                                    serialPort.write(String.valueOf(loadData).getBytes());
//                                    //need get the response
//                                    String res = responseData.getText().toString();
//                                    Toast.makeText(EnrollActivity.this, res, Toast.LENGTH_SHORT).show();
//                                    if (!res.matches("")) {
//                                        String id = "\"staus_code\":";
//                                        int q = s.indexOf(id) + id.length();
//                                        int sid = Integer.parseInt(s.substring(q, s.indexOf("}", q)));
//                                        if (sid == 0) {
//                                            Snackbar snackbar = Snackbar.make(mainLayout, "Data loaded to device successfully!", Snackbar.LENGTH_SHORT);
//                                            snackbar.show();
//                                        } else {
//                                            Snackbar snackbar = Snackbar.make(mainLayout, "Something went wrong!", Snackbar.LENGTH_SHORT);
//                                            snackbar.show();
//                                        }
//                                    }
//                                } catch (JSONException | InterruptedException e) {
//                                    Toast.makeText(EnrollActivity.this, String.valueOf(e), Toast.LENGTH_SHORT).show();
//                                }
//                            } while (cursor.moveToNext());
//                        }
//                        cursor.close();
                        takeAttendance.setVisibility(View.VISIBLE);
                    }
                });
                alertdialog_sub.show();
            }
        });
    }

    private void tvAppend(TextView tv, CharSequence text) {
        final TextView ftv = tv;
        final CharSequence ftext = text;
        runOnUiThread(new Runnable() {
            @Override
            public void run() {
                ftv.append(ftext);
            }
        });
    }

    public void test() {
        Toast.makeText(this, "Hello", Toast.LENGTH_SHORT).show();
    }

    public void enrollNow() {
        if (Objects.requireNonNull(editTextStdBranch.getText()).toString().matches("") && Objects.requireNonNull(editTextStdSec.getText()).toString().matches("")
                && Objects.requireNonNull(editTextStdName.getText()).toString().matches("")) {

            Snackbar snackbar = Snackbar.make(mainLayout, "Please fill all the details!", Snackbar.LENGTH_SHORT);
            snackbar.show();
        } else {
            JSONArray fpArray = new JSONArray();
            for (int i = 0; i < 512; i++) {
                fpArray.put(0);
            }
            JSONObject student = new JSONObject();
            try {
                student.put("op", 1);
                student.put("id", ++id);
                student.accumulate("fp_packet", fpArray);
            } catch (JSONException e) {
                Log.d("JSONException", String.valueOf(e));
            }

            String mainData = String.valueOf(student);
            if (serialPort != null) {
                serialPort.write(mainData.getBytes());
                Toast.makeText(this, String.valueOf(id), Toast.LENGTH_SHORT).show();
                Snackbar snackbar = Snackbar.make(mainLayout, "Data Sent!", Snackbar.LENGTH_SHORT);
                snackbar.show();
                save.setVisibility(View.VISIBLE);
            } else {
                Snackbar snackbar = Snackbar.make(mainLayout, "Serial Port Null", Snackbar.LENGTH_SHORT);
                snackbar.show();
            }
            Log.d("JSON", String.valueOf(student));
        }
    }

}

