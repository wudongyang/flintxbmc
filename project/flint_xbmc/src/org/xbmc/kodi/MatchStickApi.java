package org.xbmc.kodi;

import android.content.Intent;
import android.util.Log;

import tv.matchstick.flint.FlintDevice;

public class MatchStickApi {
	
	private final static String TAG = "MS/MatchStickApi";
	
	public static XBMCApplication application;
	public static String ACTION_BROADCAST_CMD="android.intent.action.MATCHSTICK_FLING_CMD";
	public static String ACTION_PLAYER_STATUS="ACTION_PLAYER_STATUS";
	public static String ACTION_CONNECTDEVICE_RESULT="ACTION_CONNECTDEVICE_RESULT";
	static native void _onCallback(Intent intent);
	
	/**
	 * getDeviceList
	 */
	public static String getDeviceList(){
		String devices="";
		devices=application.getDeviceList();
		return devices;
	}
	
	/**
	 * getDevice
	 */
	private static FlintDevice getDevice(String deviceName,String deviceMacAddr){
		FlintDevice castDevice=application.getDevice(deviceName, deviceMacAddr);
		if(castDevice==null){
			getDeviceList();
			sleep(500);
			castDevice=application.getDevice(deviceName, deviceMacAddr);
		}
		
		return castDevice;
	}
	
	/**
	 * connectDevice
	 */
	public static String connectDevice(String deviceName,String deviceMacAddr){
		String rtMsg="";
		FlintDevice castDevice=getDevice(deviceName, deviceMacAddr);
		if(castDevice!=null){
			application.connectDevice(castDevice);
		}
		else{
			rtMsg="<rtCode>-1</rtCode><rtMsg>Not Device</rtMsg>";
		}
		
		return rtMsg;
	}
	
	/**
	 * disconnect
	 */
	public static String disconnect(){
		application.disconnect();
		return "";
	}
	
	/**
	 * playFile
	 */
	public static String playFile(String videoName,String videoUrl,int position){
		String rtMsg="";
		application.playFile(videoName, videoUrl, position);
		return rtMsg;
	}
	
	/**
	 * play/pause
	 */
	public static String play(){
		String rtMsg="";
		application.play();
		return rtMsg;
	}
	
	/**
	 * seekTo
	 * @param time seconds
	 */
	public static String seekTime(int time){
		String rtMsg="";
		application.seekTime(time);
		
		return rtMsg;
	}
	
	/**
	 * setVolume
	 * @param volume
	 */
	public static String setVolume(int volume){
		String rtMsg="";
		application.setVolume(volume);
		
		return rtMsg;
	}
	
	/**
	 * sleep
	 */
	private static void sleep(long time){
		try {
			Thread.sleep(time);
		} catch (InterruptedException e) {
			Log.e(TAG,"test:",e);
		}
	}
	
}
