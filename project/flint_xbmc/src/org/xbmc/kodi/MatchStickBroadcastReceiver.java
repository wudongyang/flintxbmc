package org.xbmc.kodi;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.util.Log;

public class MatchStickBroadcastReceiver extends BroadcastReceiver {
	
	@Override
	public void onReceive(Context context, Intent intent) {
		Log.d("MatchStickBroadcastReceiver", "Received Intent");
		try {
			if (intent.getAction().equals(MatchStickApi.ACTION_BROADCAST_CMD)) {
				String cmdmsg = intent.getStringExtra("CMD");
				Log.d("Matchstick/", "receiver: " + cmdmsg);
				if (cmdmsg != null && cmdmsg.indexOf("params=") > 0) {
					String command = "", deviceName = "", deviceMacAddr = "";
					String params = cmdmsg.substring(cmdmsg.indexOf("params=") + 7);
					if (params.indexOf("<cmdName>") != -1) {
						command = params.substring(params.indexOf("<cmdName>") + 9,params.indexOf("</cmdName>"));
					}
					if (params.indexOf("<deviceName>") != -1) {
						deviceName = params.substring(params.indexOf("<deviceName>") + 12,params.indexOf("</deviceName>"));
					}
					if (params.indexOf("<deviceMacAddr>") != -1) {
						deviceMacAddr = params.substring(params.indexOf("<deviceMacAddr>") + 15,params.indexOf("</deviceMacAddr>"));
					}
					if (command.equals("connectDevice")) {
						String rtmsg = MatchStickApi.connectDevice(deviceName,deviceMacAddr);
						if (rtmsg.indexOf("-1") < 0) {
							rtmsg = "OK";
						}
						Intent result = new Intent();
						result.setAction(MatchStickApi.ACTION_CONNECTDEVICE_RESULT);
						result.putExtra("RESULT", rtmsg);
						MatchStickApi._onCallback(result);
					} else if (command.equals("disconnect")) {
						MatchStickApi.disconnect();
					} else if (command.equals("setVolume")) {
						if (params.indexOf("<time>") != -1) {
							String timeStr = params.substring(params.indexOf("<time>") + 6,params.indexOf("</time>"));
							MatchStickApi.setVolume(Integer.valueOf(timeStr));
						}
					} else if (command.equals("seekTime")) {
						if (params.indexOf("<time>") != -1) {
							String timeStr = params.substring(params.indexOf("<time>") + 6,params.indexOf("</time>"));
							MatchStickApi.seekTime(Integer.valueOf(timeStr));
						}
					}
				}
			}
		} catch (UnsatisfiedLinkError e) {
			Log.e("MatchStickBroadcastReceiver", "Native not registered");
		} catch (Exception e) {
			Log.e("MatchStickBroadcastReceiver", "error");
		}
	}
	
}
