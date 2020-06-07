package com.gf.arrow4over6;

import androidx.annotation.Nullable;
import androidx.appcompat.app.AppCompatActivity;

import android.content.Intent;
import android.net.VpnService;
import android.os.Bundle;
import android.os.Handler;
import android.os.Looper;
import android.view.View;
import android.widget.Button;
import android.widget.EditText;
import android.widget.TextView;

import org.w3c.dom.Text;

public class MainActivity extends AppCompatActivity {

    // state == 0, not connected; 1, connected; 2, being connecting
    private int connectButtonState = 0;

    private static String[] ipInfos;

    private static MainActivity _instance;

    // Used to load the 'native-lib' library on application startup.
    static {
        System.loadLibrary("arrowlib");
    }

    static MainActivity getInstance() {
        return _instance;
    }

    public static void updateIpInfo(final String ipv4Addr) {
        Handler mainHandler = new Handler(Looper.getMainLooper());
        mainHandler.post(new Runnable() {
            @Override
            public void run() {
                MainActivity mainActivity = getInstance();
                TextView ipv4AddrView =  mainActivity.findViewById(R.id.ipv4AddrView);
                ipv4AddrView.setText(ipv4Addr);
            }
        });
    }

    public static void updateStatistics(final int connectTime, final int uploadSpeed, final int downloadSpeed,
                                            final int uploadPackets, final int downloadPackets, final int uploadFlow, final int downloadFlow) {
        final String connectTimeStr = connectTime + " s";
        final String uploadSpeedStr = uploadSpeed + " Byte/s", downloadSpeedStr = downloadSpeed + " Byte/s";
        final String uploadFlowStr = uploadFlow + " Bytes", downloadFlowStr = downloadFlow + " Bytes";
        Handler mainHandler = new Handler(Looper.getMainLooper());
        mainHandler.post(new Runnable() {
            @Override
            public void run() {
                MainActivity mainActivity = getInstance();
                ((TextView) mainActivity.findViewById(R.id.connectTimeView)).setText(connectTimeStr);
                ((TextView) mainActivity.findViewById(R.id.uploadSpeedView)).setText(uploadSpeedStr);
                ((TextView) mainActivity.findViewById(R.id.downloadSpeedView)).setText(downloadSpeedStr);
                ((TextView) mainActivity.findViewById(R.id.uploadPacketsView)).setText(Integer.toString(uploadPackets));
                ((TextView) mainActivity.findViewById(R.id.downloadPacketsView)).setText(Integer.toString(downloadSpeed));
                ((TextView) mainActivity.findViewById(R.id.uploadFlowView)).setText(uploadFlowStr);
                ((TextView) mainActivity.findViewById(R.id.downloadFlowView)).setText(downloadFlowStr);
            }
        });
    }

    public static void clearStatistics() {
        Handler mainHandler = new Handler(Looper.getMainLooper());
        mainHandler.post(new Runnable() {
            @Override
            public void run() {
                MainActivity mainActivity = getInstance();
                ((TextView) mainActivity.findViewById(R.id.ipv4AddrView)).setText("");
                ((TextView) mainActivity.findViewById(R.id.connectTimeView)).setText("");
                ((TextView) mainActivity.findViewById(R.id.uploadSpeedView)).setText("");
                ((TextView) mainActivity.findViewById(R.id.downloadSpeedView)).setText("");
                ((TextView) mainActivity.findViewById(R.id.uploadPacketsView)).setText("");
                ((TextView) mainActivity.findViewById(R.id.downloadPacketsView)).setText("");
                ((TextView) mainActivity.findViewById(R.id.uploadFlowView)).setText("");
                ((TextView) mainActivity.findViewById(R.id.downloadFlowView)).setText("");
//                ((Button) mainActivity.findViewById(R.id.connectButton)).setText("连接");
//                connectButtonState = 0;
            }
        });
    }

    public static void establishVpn(final String[] ipInfos) {
        MainActivity.ipInfos = ipInfos;
        Handler mainHandler = new Handler(Looper.getMainLooper());
        mainHandler.post(new Runnable() {
            @Override
            public void run() {
                ArLog.i("Start to establish Vpn");

                Intent intent = VpnService.prepare(MainActivity.getInstance());
                if(intent != null) {
                    getInstance().startActivityForResult(intent, Constants.VPN_REQUEST_CODE);

                }
                else {

                    getInstance().onActivityResult(Constants.VPN_REQUEST_CODE, RESULT_OK, null);
                }
            }
        });

    }

    @Override
    protected void onActivityResult(int requestCode, int resultCode, @Nullable Intent data) {
        super.onActivityResult(requestCode, resultCode, data);
        ArLog.i("onActivityResult begin");
        if(requestCode == Constants.VPN_REQUEST_CODE && resultCode == RESULT_OK) {
            Intent startVpnIntent = new Intent(this, ArrowVpnService.class);
            startVpnIntent.putExtra("ipInfos", ipInfos);
            startService(startVpnIntent);
        }
        else {
            ArLog.w("requestCode = " + requestCode + " resultCode = " + resultCode);
        }
    }

    private void testPipe() {
        new Thread(new Runnable() {
            @Override
            public void run() {
                PipeFrontEnd.waitUntilInitialized();
                int receiveInt = PipeFrontEnd.getInstance().readInt();
                ArLog.i("Read int from backend: " + receiveInt);
                int sendInt = 102;
                PipeFrontEnd.getInstance().writeInt(sendInt);
            }
        }).start();

    }

    private void connectVpn() {
        String ipv6AddrStr = ((EditText)findViewById(R.id.ipv6AddrText)).getText().toString();
        int port = Integer.parseInt(((EditText)findViewById(R.id.portText)).getText().toString());
        ArFrontFramework.getInstance().establishConnect(ipv6AddrStr, port);
    }

    private void stopConnect() {
        PipeFrontEnd.getInstance().writeInt(103);
        MainActivity.clearStatistics();
    }

    private void setConnectButton() {
        final Button connectButton = (Button) findViewById(R.id.connectButton);
        ArLog.i("connectButton clicked");
        connectButton.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                if(connectButtonState == 0) {
                    ArLog.i("turn into connecting state");
                    connectButton.setText("断开");
                    connectButtonState = 1;
                    connectVpn();
                }
                else if(connectButtonState == 1) {
                    ArLog.i("turn into disconnected state");
                    connectButton.setText("连接");
                    connectButtonState = 0;
                    stopConnect();
                }
            }
        });
    }

    private void initView() {
        EditText ipv6AddrText = (EditText) findViewById(R.id.ipv6AddrText);
        ipv6AddrText.setText(Constants.INIT_IPV6_ADDRESS);
        EditText portText = (EditText) findViewById(R.id.portText);
        portText.setText(Constants.INIT_PORT);
        setConnectButton();
    }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        _instance = this;
        // Example of a call to a native method
        int backendRet = startBackend(getFilesDir().getAbsolutePath());
        PipeFrontEnd.setPipeDirPathAndBuild(getFilesDir().getAbsolutePath());
        ArLog.i("After set pipe dir");
//        PipeFrontEnd.constructInstance();
//        testPipe();
        initView();
        ArLog.i("haha");
    }

    /**
     * A native method that is implemented by the 'native-lib' native library,
     * which is packaged with this application.
     */
    public native int startBackend(String appDir);
}
