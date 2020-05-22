package com.gf.arrow4over6;

import android.content.Intent;
import android.net.VpnService;
import android.os.ParcelFileDescriptor;

public class ArrowVpnService extends VpnService {

    @Override
    public int onStartCommand(Intent intent, int flags, int startId) {
        ArLog.i("begin ArrowVpnService");
        String[] ipInfos =  intent.getStringArrayExtra("ipInfos");
        assert ipInfos != null;
        int vpnFd = setVpn(ipInfos);
        sendVpnFd(vpnFd);
        return super.onStartCommand(intent, flags, startId);

    }

    private void sendVpnFd(final int vpnFd) {
        new Thread(new Runnable() {
            @Override
            public void run() {
                PipeFrontEnd.getInstance().writeInt(102);
                PipeFrontEnd.getInstance().writeInt(vpnFd);
            }
        }).start();
    }

    private int setVpn(String[] ipInfos) {
        int sockFd = 0;
        try {
            sockFd = Integer.parseInt(ipInfos[5]);
        } catch (NumberFormatException e) {
            ArLog.e("sockfd can not be interpreted as int" + ipInfos[5]);
            ArLog.e(e.getMessage(), e);
            return -1;
        }

        this.protect(sockFd);

        Builder builder = new Builder();
        builder.setSession(getString(R.string.app_name));
        builder.addAddress(ipInfos[0], 32);
        builder.addDnsServer(ipInfos[2]);
        builder.addDnsServer(ipInfos[3]);
        builder.addDnsServer(ipInfos[4]);
        builder.setMtu(1500);
        builder.addRoute("0.0.0.0", 0);

        ParcelFileDescriptor vpnFileDescriptor = builder.establish();
        assert vpnFileDescriptor != null;
        int vpnFd = vpnFileDescriptor.getFd();
        ArLog.i("vpn fd = " + vpnFd);
        return vpnFd;
    }
}
