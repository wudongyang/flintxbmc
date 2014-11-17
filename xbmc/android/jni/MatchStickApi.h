#pragma once
/*
 *      Copyright (C) 2013 Team XBMC
 *      http://xbmc.org
 *
 *  This Program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2, or (at your option)
 *  any later version.
 *
 *  This Program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with XBMC; see the file COPYING.  If not, see
 *  <http://www.gnu.org/licenses/>.
 *
 */
#include "JNIBase.h"

class CJNIMatchStickApi : public CJNIBase
{
public:
	static jclass clazz;

	//current device
	static std::string device_name;
	static std::string device_macaddr;
	static int device_state; //0,idle 1,connected 2,playing

	//remote player status
	static std::string play_status_xml;
	static std::string content_id;
	static int current_time;
	static int duration;
	static int event_sequence;
	static bool muted;
	static int state;
	static bool time_progress;
	static std::string title;
	static int volume;

public:
	CJNIMatchStickApi(const jni::jhobject &object) : CJNIBase(object) {};
	CJNIMatchStickApi(const std::string &className);
	~CJNIMatchStickApi() {};

	static std::string getDeviceList();
	static std::string connectDevice(const std::string &deviceName,const std::string &deviceMacAddr);
	static std::string disconnect();
	static std::string playFile(const std::string &videoName,const std::string &videoUrl,int position=0);
	static std::string play();
	static std::string seekTime(int time);
	static std::string setVolume(int time);

	static void _onCallback(JNIEnv *env, jobject context, jobject intent);
};
