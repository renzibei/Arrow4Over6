package com.gf.arrow4over6;

import android.content.Intent;
import android.net.VpnService;

import java.util.Arrays;

public enum ArFrontFramework {
    INSTANCE;

    public static ArFrontFramework getInstance() {
        return INSTANCE;
    }



    public void establishConnect(final String ipv6AddrStr, final int port) {
        new Thread(new Runnable() {
            @Override
            public void run() {
                ArLog.i("before establish Connect");
                PipeFrontEnd.waitUntilInitialized();
                //send connect signal, ipv6 address and port
                PipeFrontEnd.getInstance().writeInt(101);
                PipeFrontEnd.getInstance().writeBytes(ipv6AddrStr.getBytes());
                PipeFrontEnd.getInstance().writeInt(port);

                // receive ip info and sockfd
                int code = PipeFrontEnd.getInstance().readInt();
                if(code != 201) {
                    ArLog.e("read code not equals 201");
                    return;
                }
                byte[] data = PipeFrontEnd.getInstance().readBytes();
                int dataLen = data.length;
                String dataStr = new String(Arrays.copyOfRange(data, 0, dataLen));

                int sockFd = PipeFrontEnd.getInstance().readInt();
                ArLog.i("ip info " + dataStr);

                dataStr = dataStr + sockFd;
                String[] ipInfos = dataStr.split(" ");
                if(ipInfos.length != 6) {
                    ArLog.e("Error, ip info len != 6, ip info len = " + ipInfos.length);
                    for(int i = 0; i < ipInfos.length; ++i) {
                        ArLog.e(ipInfos[i]);
                    }
                    return;
                }
                ArLog.i("sockfd : " + ipInfos[5]);
                MainActivity.updateIpInfo(ipInfos[0]);

                MainActivity.establishVpn(ipInfos);
                readLoop();

            }
        }).start();
    }

    private void readLoop() {
        int readCode = 0;
        ArLog.i("frontend readloop start");
        while(true) {
            readCode = PipeFrontEnd.getInstance().readInt();
            PipeFrontEnd frontEnd = PipeFrontEnd.getInstance();
            if(readCode == 202) {
                int connectTime = frontEnd.readInt();
                int uploadSpeed = frontEnd.readInt();
                int downLoadSpeed = frontEnd.readInt();
                int uploadPackets = frontEnd.readInt();
                int downloadPackets = frontEnd.readInt();
                int uploadBandwidth = frontEnd.readInt();
                int downloadBandwidth = frontEnd.readInt();
                MainActivity.updateStatistics(connectTime, uploadSpeed, downLoadSpeed, uploadPackets, downloadPackets, uploadBandwidth, downloadBandwidth);
            }
            else if(readCode == 203) {
                ArLog.i("end front end readLoop");
                break;
            }
            else {
                ArLog.e("unknown readCode = " + readCode);
            }
        }
    }


}
