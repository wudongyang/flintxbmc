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
#include "MatchStickApi.h"
#include "JNIBase.h"
#include "Intent.h"
#include "jutils/jutils-details.hpp"

#include "android/activity/XBMCApp.h"
#include "system.h"
#include "utils/StringUtils.h"

using namespace jni;

jclass CJNIMatchStickApi::clazz;

//current device
std::string CJNIMatchStickApi::device_name;
std::string CJNIMatchStickApi::device_macaddr;
int CJNIMatchStickApi::device_state;

//remote player status
std::string CJNIMatchStickApi::play_status_xml;
std::string CJNIMatchStickApi::content_id;
int CJNIMatchStickApi::current_time;
int CJNIMatchStickApi::duration;
int CJNIMatchStickApi::event_sequence;
bool CJNIMatchStickApi::muted=false;
int CJNIMatchStickApi::state;
bool CJNIMatchStickApi::time_progress=true;
std::string CJNIMatchStickApi::title;
int CJNIMatchStickApi::volume;

std::string CJNIMatchStickApi::getDeviceList()
{
	return jcast<std::string>(call_static_method<jhstring>(jhclass(clazz),"getDeviceList",
		"()Ljava/lang/String;"));
}

std::string CJNIMatchStickApi::connectDevice(const std::string &deviceName,const std::string &deviceMacAddr)
{
	CXBMCApp::matchstickMsg="";
	CXBMCApp::SendBroadcastInBg("android.intent.action.MATCHSTICK_FLING_CMD","CMD","matchstick://fling/?params=<cmdName>connectDevice</cmdName><deviceName>"+deviceName+"</deviceName><deviceMacAddr>"+deviceMacAddr+"</deviceMacAddr>");
	for(int i=0;i<100;i++){
		Sleep(500);
		if(CXBMCApp::matchstickMsg!=""){
			return CXBMCApp::matchstickMsg;
		}
	}
	return "";
}

std::string CJNIMatchStickApi::disconnect()
{
	//return jcast<std::string>(call_static_method<jhstring>(jhclass(clazz),"disconnect",
	//	"()Ljava/lang/String;"));
	CXBMCApp::SendBroadcastInBg("android.intent.action.MATCHSTICK_FLING_CMD","CMD","matchstick://fling/?params=<cmdName>disconnect</cmdName>");
	return "";
}

std::string CJNIMatchStickApi::playFile(const std::string &videoName,const std::string &videoUrl,int position/*=0*/)
{
	return jcast<std::string>(call_static_method<jhstring>(jhclass(clazz),"playFile",
		"(Ljava/lang/String;Ljava/lang/String;I)Ljava/lang/String;",jcast<jhstring>(videoName),jcast<jhstring>(videoUrl),position ));
}

std::string CJNIMatchStickApi::play()
{
	return jcast<std::string>(call_static_method<jhstring>(jhclass(clazz),"play",
		"()Ljava/lang/String;"));
}

std::string CJNIMatchStickApi::seekTime(const int time)
{
	//return jcast<std::string>(call_static_method<jhstring>(jhclass(clazz),"seekTime",
	//	"(I)Ljava/lang/String;",time));
	CStdString cmdstr;
	cmdstr = StringUtils::Format("matchstick://fling/?params=<cmdName>seekTime</cmdName><time>%i</time>",time);
	CXBMCApp::SendBroadcastInBg("android.intent.action.MATCHSTICK_FLING_CMD","CMD",cmdstr);
	return "";
}

std::string CJNIMatchStickApi::setVolume(const int time)
{
	//return jcast<std::string>(call_static_method<jhstring>(jhclass(clazz),"setVolume",
	//	"(I)Ljava/lang/String;",time));
	CStdString cmdstr;
	cmdstr = StringUtils::Format("matchstick://fling/?params=<cmdName>setVolume</cmdName><time>%i</time>",time);
	CXBMCApp::SendBroadcastInBg("android.intent.action.MATCHSTICK_FLING_CMD","CMD",cmdstr);
	return "";
}

void CJNIMatchStickApi::_onCallback(JNIEnv *env, jobject context, jobject jintent)
{
	(void)env;
	(void)context;
	CJNIIntent intent;
	intent=CJNIIntent(jhobject(jintent));
	std::string action = intent.getAction();
	CXBMCApp::android_printf("CJNIMatchStickApi::_onCallback Action: %s", action.c_str());
	if(action=="ACTION_CONNECTDEVICE_RESULT"){
		std::string result=intent.getStringExtra("RESULT");
		if(result!=""){
			CXBMCApp::matchstickMsg=result;
		}
	}
	else if (action=="ACTION_PLAYER_STATUS"){
		CStdString statusStr=intent.getStringExtra("STATUS");
		if(statusStr != ""){
			play_status_xml=statusStr;
			CStdString tmpStr;
			int iStart=0;
			int iEnd=0;
			if (statusStr.find("<content_id>")!=std::string::npos){
				iStart=statusStr.find("<content_id>");
				iEnd=statusStr.find("</content_id>");
				content_id = statusStr.substr(iStart+12,iEnd-iStart-12);
			}
			if (statusStr.find("<current_time>")!=std::string::npos){
				iStart=statusStr.find("<current_time>");
				iEnd=statusStr.find("</current_time>");
				tmpStr = statusStr.substr(iStart+14,iEnd-iStart-14);
				if(StringUtils::IsInteger(tmpStr)){
					current_time=atoi(tmpStr);
				}
			}
			if (statusStr.find("<duration>")!=std::string::npos){
				iStart=statusStr.find("<duration>");
				iEnd=statusStr.find("</duration>");
				tmpStr = statusStr.substr(iStart+10,iEnd-iStart-10);
				if(StringUtils::IsInteger(tmpStr)){
					duration=atoi(tmpStr);
				}
			}
			if (statusStr.find("<event_sequence>")!=std::string::npos){
				iStart=statusStr.find("<event_sequence>");
				iEnd=statusStr.find("</event_sequence>");
				tmpStr = statusStr.substr(iStart+16,iEnd-iStart-16);
				if(StringUtils::IsInteger(tmpStr)){
					event_sequence=atoi(tmpStr);
				}
			}
			if (statusStr.find("<muted>")!=std::string::npos){
				iStart=statusStr.find("<muted>");
				iEnd=statusStr.find("</muted>");
				tmpStr = statusStr.substr(iStart+7,iEnd-iStart-7);
				muted=tmpStr.Equals("true");
			}
			if (statusStr.find("<state>")!=std::string::npos){
				iStart=statusStr.find("<state>");
				iEnd=statusStr.find("</state>");
				tmpStr = statusStr.substr(iStart+7,iEnd-iStart-7);
				if(StringUtils::IsInteger(tmpStr)){
					state=atoi(tmpStr);
				}
			}
			if (statusStr.find("<time_progress>")!=std::string::npos){
				iStart=statusStr.find("<time_progress>");
				iEnd=statusStr.find("</time_progress>");
				tmpStr = statusStr.substr(iStart+15,iEnd-iStart-15);
				time_progress=tmpStr.Equals("true");
			}
			if (statusStr.find("<title>")!=std::string::npos){
				iStart=statusStr.find("<title>");
				iEnd=statusStr.find("</title>");
				tmpStr = statusStr.substr(iStart+7,iEnd-iStart-7);
			}
			if (statusStr.find("<volume>")!=std::string::npos){
				iStart=statusStr.find("<volume>");
				iEnd=statusStr.find("</volume>");
				tmpStr = statusStr.substr(iStart+8,iEnd-iStart-8);
				if(StringUtils::IsInteger(tmpStr)){
					volume=atoi(tmpStr);
				}
			}
		}
	}
}
