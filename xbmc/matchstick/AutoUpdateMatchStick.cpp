#include "AutoUpdateMatchStick.h"
#if defined(TARGET_ANDROID)
#include "../android/jni/MatchStickApi.h"
#endif
#include "utils/log.h"

AutoUpdateMatchstick g_AutoUpdateMatchStick;

AutoUpdateMatchstick::AutoUpdateMatchstick(): CThread("AutoUpdateMatchstick")
{
  m_bStop     = false;
  m_running   = false;
  m_hasDevice = false;
}

AutoUpdateMatchstick::~AutoUpdateMatchstick()
{
  m_bStop = true;
}

void AutoUpdateMatchstick::Process()
{
  while (!m_bStop)
  {
    FindCastDevice();

    m_startTime.StartZero();
    while (!m_bStop && m_startTime.GetElapsedMilliseconds() < 1000*5) 
    {
      Sleep(100);
    }
    if (m_bStop) 
    {
      return;
    }
  }
}

void AutoUpdateMatchstick::Run()
{   
    if(!m_running)
    {
        this->Create();
    m_running = true;
    }
}

void AutoUpdateMatchstick::FindCastDevice()
{
#if defined(TARGET_ANDROID)
  m_CastInfo = CJNIMatchStickApi::getDeviceList();
#else
  //m_CastInfo = "";
#endif
  CLog::Log(LOGDEBUG, "AutoUpdateMatchstick::FindCastDevice() m_CastInfo:%s",m_CastInfo.c_str());
  m_hasDevice = !m_CastInfo.empty();
}

bool AutoUpdateMatchstick::HasDevice()
{
  CLog::Log(LOGDEBUG, "AutoUpdateMatchstick::HasDevice() m_hasDevice:%d",m_hasDevice);
  return m_hasDevice;
}

std::string AutoUpdateMatchstick::GetCastInfo()
{
  return m_CastInfo;
}

