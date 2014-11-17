#ifndef AUTOUPDATEMATCHSTICK_H_
#define AUTOUPDATEMATCHSTICK_H_

#include <string>
#include "threads/Thread.h"
#include "utils/Stopwatch.h"

class AutoUpdateMatchstick : public CThread
{
public:
	AutoUpdateMatchstick();
	~AutoUpdateMatchstick();

	void Run();
  bool HasDevice();
	virtual void Process();
  std::string GetCastInfo();
protected:
  void FindCastDevice();
private:
  bool        m_running;
  bool        m_hasDevice;
	CStopWatch  m_startTime;
  bool        m_bStop;
  std::string m_CastInfo;
};

extern AutoUpdateMatchstick g_AutoUpdateMatchStick;

#endif//AUTOUPDATEAPP_H_
