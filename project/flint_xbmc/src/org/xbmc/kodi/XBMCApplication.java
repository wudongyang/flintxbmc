package org.xbmc.kodi;

import java.util.ArrayList;

import android.app.Application;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.net.wifi.WifiManager;
import android.os.Handler;
import android.util.Log;
import android.widget.Toast;
import android.support.v7.media.MediaRouter;
import android.support.v7.media.MediaRouter.RouteInfo;
import android.support.v7.media.MediaRouteSelector;

import tv.matchstick.flint.Flint;
import tv.matchstick.flint.FlintDevice;
import tv.matchstick.flint.MediaInfo;
import tv.matchstick.flint.MediaMetadata;
import tv.matchstick.flint.MediaStatus;

public class XBMCApplication extends Application implements
		IRemotePlayController {

	private final String TAG = "XBMC/XBMCApplication";

	private WifiManager mWifiManager;
	public boolean isSearching = true;
	public String devices = "";
	public ArrayList<FlintDevice> deviceslist = new ArrayList<FlintDevice>();
	private FlintDevice mDevice;
	private String appname;
	private Handler mHandler = new Handler();
	private Thread mThread = null;
	private QueryRunnable mRunnable = null;
	public boolean queryFlag = false;
	Intent intent = new Intent();
	StringBuilder statusBuf = new StringBuilder();

	protected MediaRouter mMediaRouter;
	protected MediaRouteSelector mMediaRouteSelector;
	private static String APPLICATION_ID;
	private static FlingHelper mFlingHelper = null;

	public void onCreate() {
		super.onCreate();
		Log.d(TAG, "XBMCApplication onCreate...");
		mWifiManager = (WifiManager) this
				.getSystemService(Context.WIFI_SERVICE);
		WifiStateReceiver wifiReceiver = new WifiStateReceiver();
		IntentFilter filter = new IntentFilter();
		filter.addAction(WifiManager.RSSI_CHANGED_ACTION);
		filter.addAction(WifiManager.NETWORK_STATE_CHANGED_ACTION);
		filter.addAction(WifiManager.WIFI_STATE_CHANGED_ACTION);
		registerReceiver(wifiReceiver, filter);
		MatchStickBroadcastReceiver broadcastReceiver = new MatchStickBroadcastReceiver();
		filter = new IntentFilter();
		filter.addAction(MatchStickApi.ACTION_BROADCAST_CMD);
		registerReceiver(broadcastReceiver, filter);
		MatchStickApi.application = this;
		FlingHelper.mRemotePlayController = this;
		APPLICATION_ID = "~flintplayer";
		Flint.FlintApi.setApplicationId(APPLICATION_ID);
	}

	public static FlingHelper getCastManager(Context context) {
		if (null == mFlingHelper) {
			mFlingHelper = new FlingHelper(context);
		}
		return mFlingHelper;
	}

	/**
	 * getDeviceList
	 */
	public String getDeviceList() {
		devices = "";
		if (isConnectedToWifi()) {
			StringBuilder sbuf = new StringBuilder();
			for (RouteInfo routeInfo : mFlingHelper.getRouteList()) {
				FlintDevice device = FlintDevice.getFromBundle(routeInfo
						.getExtras());
				sbuf.append("<device");
				sbuf.append(" macAddr=\"" + device.getDeviceId() + "\"");
				sbuf.append(">");
				sbuf.append(device.getFriendlyName());
				sbuf.append("</device>");
			}
			devices = sbuf.toString();
			Log.d(TAG, "getDeviceList: devices -- " + devices);
		} else {
			alert("Please enable and connect WiFi!");
		}

		return devices;
	}

	/**
	 * getDevice
	 */
	public FlintDevice getDevice(String deviceName, String deviceMacAddr) {
		for (RouteInfo routeInfo : mFlingHelper.getRouteList()) {
			FlintDevice device = FlintDevice.getFromBundle(routeInfo
					.getExtras());
			if (device.getFriendlyName().equals(deviceName)
					&& device.getDeviceId().equals(deviceMacAddr)) {
				return device;
			}
		}

		return null;
	}

	/**
	 * alert
	 */
	public void alert(String msg) {
		Toast.makeText(this, msg, Toast.LENGTH_SHORT).show();
	}

	private class WifiStateReceiver extends BroadcastReceiver {
		@Override
		public void onReceive(Context context, Intent intent) {
			if (intent.getAction()
					.equals(WifiManager.WIFI_STATE_CHANGED_ACTION)) {
				int curState = intent.getIntExtra(WifiManager.EXTRA_WIFI_STATE,
						-1);
				switch (curState) {
				case WifiManager.WIFI_STATE_DISABLED:
					Toast.makeText(getApplicationContext(),
							R.string.wifi_enable_toast_note, Toast.LENGTH_SHORT)
							.show();
					break;
				case WifiManager.WIFI_STATE_DISABLING:
					clearData();
					break;
				case WifiManager.WIFI_STATE_ENABLED:
					break;
				case WifiManager.WIFI_STATE_ENABLING:
					break;
				case WifiManager.WIFI_STATE_UNKNOWN:
					break;
				default:
					break;
				}
			} else if (intent.getAction().equals(
					WifiManager.NETWORK_STATE_CHANGED_ACTION)) {
				Log.d(TAG, "network state changed!");
			} else {
			}
		}
	}

	/**
	 * isConnectedToWifi
	 */
	public boolean isConnectedToWifi() {
		if (!mWifiManager.isWifiEnabled()) {
			return false;
		}
		if (mWifiManager.getConnectionInfo() != null
				&& mWifiManager.getConnectionInfo().getIpAddress() != 0) {
			return true;
		} else {
			return false;
		}
	}

	/**
	 * connectDevice
	 */
	public boolean connectDevice(FlintDevice castDevice) {
		for (RouteInfo routeInfo : mFlingHelper.getRouteList()) {
			FlintDevice device = FlintDevice.getFromBundle(routeInfo
					.getExtras());
			String deviceName = castDevice.getFriendlyName();
			String deviceMacAddr = castDevice.getDeviceId();
			if (device.getFriendlyName().equals(deviceName)
					&& device.getDeviceId().equals(deviceMacAddr)) {
				mFlingHelper.selectRoute(routeInfo);
				setDevice(castDevice);
			}
		}
		return true;
	}

	/**
	 * playFile
	 */
	public void playFile(String videoName, String videoUrl, int position) {
		mFlingHelper.loadMedia(videoName, videoUrl, "video/mp4");
	}

	public void play() {
		mFlingHelper.togglePlayback();
	}

	/**
	 * seekTime
	 */
	public void seekTime(int time) {
		long cur_position = mFlingHelper.getCurrentMediaPosition();
		long go_to = cur_position + time * 1000;
		if (go_to < 0) {
			go_to = 0;
		}
		if (go_to > mFlingHelper.getMediaDuration()) {
			go_to = mFlingHelper.getMediaDuration();
		}
		mFlingHelper.play((int) go_to);
	}

	/**
	 * setVolume
	 */
	public void setVolume(int volume) {
		if (mFlingHelper == null) {
			return;
		}
		try {
			double delta = (double) volume * 0.1;
			mFlingHelper.incrementVolume(delta);
		} catch (Exception e) {
			Log.e(TAG, "setVolume() Failed to change volume", e);
		}
	}

	Handler handler = new Handler();

	public void startGetInfo() {
		queryFlag = true;
		if (mThread == null) {
			mRunnable = new QueryRunnable();
			mThread = new Thread(mRunnable);
			mThread.start();
		}
	}

	public void stopGetInfo() {
		if (mThread != null) {
			queryFlag = false;
		}
	}

	public class QueryRunnable implements Runnable {
		@Override
		public void run() {
			while (true) {
				sleep(200);
				if (queryFlag) {
					mHandler.post(new Runnable() {
						@Override
						public void run() {
							statusBuf.delete(0, statusBuf.length());
							Log.d(TAG,
									"UpdatePlayStatus() reached, current_time="
											+ mFlingHelper
													.getCurrentMediaPosition());
							MediaInfo info = mFlingHelper
									.getRemoteMediaInformation();
							if (info != null) {
								MediaMetadata metaData = info.getMetadata();
								statusBuf.append("<content_id>"
										+ info.getContentId() + "</content_id>");
								statusBuf.append("<current_time>"
										+ mFlingHelper
												.getCurrentMediaPosition()
										/ 1000 + "</current_time>");
								statusBuf.append("<duration>"
										+ mFlingHelper.getMediaDuration()
										/ 1000 + "</duration>");
								statusBuf.append("<muted>"
										+ mFlingHelper.isMute() + "</muted>");
								statusBuf.append("<state>"
										+ mFlingHelper.getPlaybackStatus()
										+ "</state>");
								statusBuf.append("<title>"
										+ metaData
												.getString(MediaMetadata.KEY_TITLE)
										+ "</title>");
								statusBuf.append("<volume>"
										+ mFlingHelper.getVolume()
										+ "</volume>");
								intent.setAction(MatchStickApi.ACTION_PLAYER_STATUS);
								intent.putExtra("STATUS", statusBuf.toString());
								MatchStickApi._onCallback(intent);
							}
						}
					});
				}
			}
		}
	}

	/**
	 * sleep
	 */
	private void sleep(long time) {
		try {
			Thread.sleep(time);
		} catch (InterruptedException e) {
			Log.e(TAG, "Matchstick:", e);
		}
	}

	/**
	 * disconnect
	 */
	public void disconnect() {
		mFlingHelper.disconnect();
	}

	/**
	 * clearData
	 */
	private void clearData() {
		setDevice(null);
		setAppname(null);
	}

	public FlintDevice getDevice() {
		return mDevice;
	}

	public void setDevice(FlintDevice device) {
		this.mDevice = device;
	}

	public String getAppname() {
		return appname;
	}

	public void setAppname(String appname) {
		this.appname = appname;
	}

	@Override
	public void UpdatePlayStatus(int playerState) {
		// TODO Auto-generated method stub
		Log.d(TAG, "UpdatePlayStatus() reached, playerState=" + playerState);
		switch (playerState) {
		case MediaStatus.PLAYER_STATE_PLAYING:
		case MediaStatus.PLAYER_STATE_PAUSED:
		case MediaStatus.PLAYER_STATE_BUFFERING:
			startGetInfo();
			break;
		default:
			stopGetInfo();
			break;
		}
	}

}
