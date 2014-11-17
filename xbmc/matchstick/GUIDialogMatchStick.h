#pragma once
#include "guilib/GUIDialog.h"
#include "video/Bookmark.h"
using namespace std;

class CGUIMatchStick : public CGUIDialog
{
public:
  struct DeviceInfo
  {
    vector<string> nameList;
    vector<string> macList;
  };
  CGUIMatchStick();
  virtual ~CGUIMatchStick(void);
  virtual void FrameMove();
  virtual bool OnMessage(CGUIMessage& message);
  virtual bool OnAction(const CAction &action);
  bool IsPlaying() const;
  bool IsConnected() const;
  bool IsCastPlayerRunning() const;
  string GetCurrentTime() const;
  string GetDurationTime() const;
  float GetPercentage() const;
  void SeekPercentage(float percentage);
  int ShowMatchStickList();
  bool ConnectDevice(int choice);
  bool ConnectDevice(string deviceName,string deviceMacAddr);
  void ShowMatchStickDialog(string videoName,string videoUrl);
  void ShowMatchStickDialogForUpnp(string deviceName);
  bool CheckIsLocalFile(string videoName);
  string GetLocalFileURL(string videoName,string videoUrl);
  void ShowMatchStickDialog(CFileItem item);
  void UpdateResumeBookmark(CBookmark bookmark);
  void ResetResumeBookmark();
  bool IsResumeMatchStick();
  void ResumeMatchStick(bool bResume);
protected:
  virtual EVENT_RESULT OnMouseEvent(const CPoint &point, const CMouseEvent &event);
private:
  bool  m_isPlaying;
  bool  m_isConnected;
  bool  m_isMatchStickPlayerRunning;
  float m_percentage;
  int   m_currentTime;
  int   m_duration;
  DeviceInfo m_deviceInfo;
  string m_CurrentDeviceName;
  CBookmark& m_progressTrackingVideoResumeBookmark;
  CFileItemPtr m_item;
  bool m_bResumeMatchStick;
private:
  void ClearDeviceInfo();
  void DisconnectCast();
  void InitPlayerState();
};
