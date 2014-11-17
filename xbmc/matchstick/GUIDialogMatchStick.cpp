#include "GUIDialogMatchStick.h"
#include "guilib/GUISliderControl.h"
#include "guilib/GUIWindowManager.h"
#include "guilib/Key.h"
#include "guilib/LocalizeStrings.h"
#include "utils/log.h"
#include "../dialogs/GUIDialogSeekBar.h"
#include "../dialogs/GUIDialogSelect.h"
#include "../dialogs/GUIDialogKaiToast.h"
#include "../Application.h"

#if defined(TARGET_ANDROID)
#include "../android/jni/MatchStickApi.h"
#endif
#include "../utils/StringUtils.h"
#include "../dialogs/GUIDialogYesNo.h"
#include "../dialogs/GUIDialogOK.h"
#include "../dialogs/GUIDialogBusy.h"
#include "utils/XMLUtils.h"
#include "AutoUpdateMatchStick.h"
#include "../interfaces/Builtins.h"
#include "../network/upnp/UPnPServer.h"
#include "../network/upnp/UPnP.h"
#include "PltUtilities.h"
#include "PltMediaController.h"
#include "URL.h"
#include "settings/Settings.h"
using namespace UPNP;
#include "video/VideoDatabase.h"
#include "ApplicationMessenger.h"
#include "FileItem.h"
#include "utils/StringUtils.h"
#include "utils/URIUtils.h"

#define MATCHSTICK_BTNID_BACK        101
#define MATCHSTICK_BTNID_DISCONNECT  102
#define MATCHSTICK_SLIDERID_PLAY     301
#define MATCHSTICK_BTNID_REWIND      401
#define MATCHSTICK_BTNID_FORWARD     402
#define MATCHSTICK_BTNID_VOLUME_DOWN 403
#define MATCHSTICK_BTNID_VOLUME_UP   404
#define MATCHSTICK_BTNID_PLAY_PAUSE  405

CGUIMatchStick::CGUIMatchStick()
  : CGUIDialog(WINDOW_DIALOG_MATCHSTICK, "DialogMatchStick.xml")
  , m_progressTrackingVideoResumeBookmark(*new CBookmark)
  , m_item(new CFileItem)
{
  m_loadType    = KEEP_IN_MEMORY;
  m_isConnected = false;
  m_CurrentDeviceName = "";
  m_isMatchStickPlayerRunning = false;

  InitPlayerState();
  
  ClearDeviceInfo();
}

CGUIMatchStick::~CGUIMatchStick()
{
  delete &m_progressTrackingVideoResumeBookmark;
}

void CGUIMatchStick::FrameMove()
{
#if defined(TARGET_ANDROID)
  CGUISliderControl *pSlider = (CGUISliderControl*)GetControl(MATCHSTICK_SLIDERID_PLAY);
  if (pSlider)
  {
    m_currentTime = CJNIMatchStickApi::current_time;
    m_duration    = CJNIMatchStickApi::duration;
    if (m_currentTime < 0) m_currentTime = 0;
    if (m_duration    < 0) m_duration    = 0;

    //check whether play is over.
    if ( (m_duration != 0) && (m_currentTime == m_duration) )
    {
      m_currentTime = 0;
      m_isPlaying   = false;
    }

    m_percentage = (float)m_currentTime/(float)m_duration * 100;
  }
#endif

   CGUIDialog::FrameMove();
}

bool CGUIMatchStick::OnAction(const CAction &action)
{
  return CGUIDialog::OnAction(action);
}

EVENT_RESULT CGUIMatchStick::OnMouseEvent(const CPoint &point, const CMouseEvent &event)
{
  return CGUIDialog::OnMouseEvent(point, event);
}

bool CGUIMatchStick::OnMessage(CGUIMessage& message)
{
  //check MatchStick player
  if (m_isMatchStickPlayerRunning)
  {
    switch ( message.GetMessage() )
    {
    case GUI_MSG_WINDOW_INIT:
      InitPlayerState();
      break;
    case GUI_MSG_CLICKED:
      {
        int iControl = message.GetSenderId();
        if (MATCHSTICK_BTNID_DISCONNECT == iControl)
        {
          CStdString strLabel;
          strLabel = StringUtils::Format(g_localizeStrings.Get(70159).c_str(),this->GetProperty("deviceName").c_str());
          if (CGUIDialogYesNo::ShowAndGetInput(g_localizeStrings.Get(70072), "", strLabel, ""))
          {
            m_bResumeMatchStick = true;
            DisconnectCast();
            Close();//close MatchStick dialog 
          }
        } 
        else if (MATCHSTICK_BTNID_REWIND == iControl)
        {
          #if defined(TARGET_ANDROID)
          CJNIMatchStickApi::seekTime( /*m_currentTime*/ - 10 );
          #endif
        }
        else if (MATCHSTICK_BTNID_FORWARD == iControl)
        {
          #if defined(TARGET_ANDROID)
          CJNIMatchStickApi::seekTime(/* m_currentTime + */10 );
          #endif
        }
        else if (MATCHSTICK_BTNID_PLAY_PAUSE == iControl)
        {
          m_isPlaying = !m_isPlaying;
          #if defined(TARGET_ANDROID)
          CJNIMatchStickApi::play();
          #endif
        }
        else if (MATCHSTICK_BTNID_VOLUME_DOWN == iControl)
        {
          #if defined(TARGET_ANDROID)
          CJNIMatchStickApi::setVolume(-1);
          #endif
        }
        else if (MATCHSTICK_BTNID_VOLUME_UP == iControl)
        {
          #if defined(TARGET_ANDROID)
          CJNIMatchStickApi::setVolume(1);
          #endif
        } 
      }
      break;
    default:break;
    }
  }
  else //else upnp player
  {
    switch ( message.GetMessage() )
    {
    case GUI_MSG_WINDOW_INIT:
      m_isPlaying = true;
      break;
    case GUI_MSG_CLICKED:
      {
        int iControl = message.GetSenderId();
        if (MATCHSTICK_BTNID_DISCONNECT == iControl)
        {
          CBuiltins::Execute("PlayerControl(Stop)");
          m_isPlaying = false;
          Close();//close UPNP dialog 
        } 
        else if (MATCHSTICK_BTNID_REWIND == iControl)
        {
          CBuiltins::Execute("PlayerControl(Rewind)");
        }
        else if (MATCHSTICK_BTNID_FORWARD == iControl)
        {
          CBuiltins::Execute("PlayerControl(Forward)");
        }
        else if (MATCHSTICK_BTNID_PLAY_PAUSE == iControl)
        {
          m_isPlaying = !m_isPlaying;
          CBuiltins::Execute("PlayerControl(Play)");
        }
        else if (MATCHSTICK_BTNID_VOLUME_DOWN == iControl)
        {
          //g_application.OnAction(ACTION_VOLUME_DOWN);
        }
        else if (MATCHSTICK_BTNID_VOLUME_UP == iControl)
        {
          //g_application.OnAction(ACTION_VOLUME_UP);
        } 
      }
      break;
    default:break;
    }
  }
  return CGUIDialog::OnMessage(message);
}

bool CGUIMatchStick::IsPlaying() const
{
  return m_isPlaying;
}

string CGUIMatchStick::GetCurrentTime() const
{
#if defined(TARGET_ANDROID)
  return StringUtils::SecondsToTimeString(m_currentTime, TIME_FORMAT_HH_MM_SS);
#else 
  return "00:00:00";
#endif
}

string CGUIMatchStick::GetDurationTime() const
{
#if defined(TARGET_ANDROID)
  return StringUtils::SecondsToTimeString(m_duration, TIME_FORMAT_HH_MM_SS);
#else
  return "02:30:00";
#endif
}

float CGUIMatchStick::GetPercentage() const
{
  return m_percentage;
}

void CGUIMatchStick::SeekPercentage(float percentage)
{
  if (m_isMatchStickPlayerRunning)
  {
    m_percentage = percentage;
    #if defined(TARGET_ANDROID)
    CJNIMatchStickApi::seekTime( CJNIMatchStickApi::duration * m_percentage*0.01f);
    #endif
  }
  else
  {
    //g_application.SeekPercentage(percentage);
  }
}

int CGUIMatchStick::ShowMatchStickList()
{
  ClearDeviceInfo();

  if(!g_AutoUpdateMatchStick.HasDevice())
  {
    CGUIDialogOK::ShowAndGetInput("Notice", 0, "Not found MatchStick", 0);
    return -1;
  }
  std::string castInfo = g_AutoUpdateMatchStick.GetCastInfo();
  TiXmlDocument ReqDataInfoDoc;
  CStdString reqResults = castInfo;
  ReqDataInfoDoc.Parse(reqResults, 0, TIXML_ENCODING_UTF8);
  if(ReqDataInfoDoc.Error())
  {
    ReqDataInfoDoc.Parse(reqResults, 0, TIXML_DEFAULT_ENCODING);
    if(ReqDataInfoDoc.Error())
    {
      CLog::Log(LOGERROR, "Error loading results");
    }
  }

  TiXmlElement *pRootElement = ReqDataInfoDoc.RootElement();
  if (!pRootElement || strcmpi(pRootElement->Value(),"device") != 0)
  {
    CLog::Log(LOGERROR, "Error loading results, no <device> node");
  }

  TiXmlNode *pNode = pRootElement;
  TiXmlNode *pChild = NULL;
  while(pNode)
  {
    pChild = pNode->IterateChildren(pChild);
    if(pChild )
    {
      if (pChild->Type() == TiXmlNode::TINYXML_TEXT)
      {
        m_deviceInfo.nameList.push_back(pChild->Value());
        TiXmlElement *pElement = (TiXmlElement *)pNode;
        CStdString mac = pElement->Attribute("macAddr");
        m_deviceInfo.macList.push_back(mac);
      }
      pNode = pChild;
      pChild = NULL;
      continue;
    }
    pChild = pNode;
    pNode = pNode->Parent();
  }
  CGUIDialogSelect *pDlg = (CGUIDialogSelect*)g_windowManager.GetWindow(WINDOW_DIALOG_SELECT);
  if (pDlg)
  {
    pDlg->SetHeading(g_localizeStrings.Get(424));
    pDlg->Reset();  //clear expired data

    CStdString listLine;
    for(unsigned int i = 0; i < m_deviceInfo.nameList.size(); i++)
    {
      listLine = m_deviceInfo.nameList[i];
      if (listLine == m_CurrentDeviceName)
      {
        listLine += " ("+g_localizeStrings.Get(31961)+")";
      }
      
      pDlg->Add(listLine);
    }

    g_application.OnAction(ACTION_PAUSE);
    // and wait till user selects one
    pDlg->DoModal();

    int choice = pDlg->GetSelectedLabel();
    if (choice >= 0)
    {
      return choice;
    }
    else
    {
      g_application.OnAction(ACTION_PLAYER_PLAY);
      return -1;
    }
  }
  else return -1;
}
bool CGUIMatchStick::ConnectDevice(string deviceName,string deviceMacAddr)
{
#if defined(TARGET_ANDROID)
  string respond = CJNIMatchStickApi::connectDevice(deviceName,deviceMacAddr);

  //wait 5s for device ready
  CGUIDialogBusy* dialog = (CGUIDialogBusy*)g_windowManager.GetWindow(WINDOW_DIALOG_BUSY);
  if (dialog)
  {
    dialog->SetAutoClose(5000);
    dialog->DoModal();
  }

  if ("OK" == respond)
  {
    CGUIDialogKaiToast::QueueNotification(CGUIDialogKaiToast::Info, "Notice", deviceName+" connected");
    m_isConnected = true;
    m_CurrentDeviceName = deviceName;
    return true;
  }
  else
  {
    CGUIDialogKaiToast::QueueNotification(CGUIDialogKaiToast::Info, "Notice", deviceName+" failed");
    m_isConnected = false;
    return false;
  }
#else
  CGUIDialogBusy* dialog = (CGUIDialogBusy*)g_windowManager.GetWindow(WINDOW_DIALOG_BUSY);
  if (dialog)
  {
    dialog->SetAutoClose(5000);
    dialog->DoModal();
  }

  m_isConnected       = true;
  m_CurrentDeviceName = deviceName;
  CGUIDialogKaiToast::QueueNotification(CGUIDialogKaiToast::Info, "Notice", deviceName+" connected");
  return true;
#endif
}
bool CGUIMatchStick::ConnectDevice(int choice)
{
  if ( (choice < 0) || (choice > (int)m_deviceInfo.nameList.size()) )
  {
    return false;
  }
  // if connected,disconnect it.
  if (m_deviceInfo.nameList[choice] == m_CurrentDeviceName)
  {
    DisconnectCast();
    return false;
  }
  else // if another device is selected,disconnect current device
  {
    DisconnectCast();
    return ConnectDevice( m_deviceInfo.nameList[choice], m_deviceInfo.macList[choice] );
  }
}

void CGUIMatchStick::ShowMatchStickDialog(string videoName,string videoUrl)
{
  if (!m_isConnected)
  {
    int choice = ShowMatchStickList();
    if (!ConnectDevice(choice)) return;
  }

  int position = (int)(g_application.GetTime()*1000);
  
  if (CheckIsLocalFile(videoUrl))
  {
    if (!CSettings::Get().GetBool("services.upnpserver"))
    {
      CGUIDialogOK::ShowAndGetInput(g_localizeStrings.Get(70072), "", g_localizeStrings.Get(31962), "");
      
      return;
    }
    else
    {
      videoUrl = GetLocalFileURL(videoName,videoUrl);
    }
  }

  g_application.StopPlaying();
  #if defined(TARGET_ANDROID)
  CJNIMatchStickApi::playFile(videoName,videoUrl,position);
  #endif
  m_isMatchStickPlayerRunning = true;
  SetProperty("deviceName",m_CurrentDeviceName);
  SetProperty("videoName" ,videoName );
  DoModal();
}

void CGUIMatchStick::ShowMatchStickDialogForUpnp(string deviceName)
{
  SetProperty("deviceName",deviceName);
  DoModal();
}

bool CGUIMatchStick::CheckIsLocalFile(string videoName)
{
  CURL url(videoName);
  return url.IsLocal() || url.IsSmb();
}

string CGUIMatchStick::GetLocalFileURL(string videoName,string videoUrl)
{
  CLog::Log(LOGDEBUG, "CGUIMatchStick::ShowMatchStickDialog() CheckIsLocalFile");
  string localPath = videoUrl;
  NPT_List<NPT_IpAddress> ips;
  CUPnPServer* upnp_server = CUPnP::GetServer();
  NPT_HttpUrl rooturi = NPT_HttpUrl("localhost", upnp_server->GetPort(), "/");
  if(NPT_SUCCESS == PLT_UPnPMessageHelper::GetIPAddresses(ips))
  {
    NPT_List<NPT_IpAddress>::Iterator ip = ips.GetFirstItem();
    NPT_String NPTStr = upnp_server->BuildSafeResourceUri(rooturi, (*ip).ToString(), localPath.c_str());
    
    return NPTStr.GetChars();
  }
  else return "";
}

bool CGUIMatchStick::IsConnected() const
{
  return m_isConnected;
}

void CGUIMatchStick::ClearDeviceInfo()
{
  m_deviceInfo.nameList.clear();
  m_deviceInfo.macList.clear();
}

bool CGUIMatchStick::IsCastPlayerRunning() const
{
  return m_isMatchStickPlayerRunning;
}

void CGUIMatchStick::DisconnectCast()
{
  if (m_CurrentDeviceName == "")
  {
    return;
  }
  
  CGUIDialogBusy* pBusyDialog = (CGUIDialogBusy*)g_windowManager.GetWindow(WINDOW_DIALOG_BUSY);
  if (pBusyDialog)
  {
    pBusyDialog->SetAutoClose(5000);
    pBusyDialog->DoModal();
  }
#if defined(TARGET_ANDROID)
  CJNIMatchStickApi::disconnect();
  
  if (m_bResumeMatchStick && m_currentTime > 0 && m_currentTime < m_duration)
  {
    m_progressTrackingVideoResumeBookmark.timeInSeconds = m_currentTime;

    CLog::Log(LOGDEBUG, "%s - Saving file state for video item %s", __FUNCTION__, m_item->GetPath().c_str());

    CVideoDatabase videodatabase;
    if (!videodatabase.Open())
    {
      CLog::Log(LOGWARNING, "%s - Unable to open video database. Can not save file state!", __FUNCTION__);
    }
    else
    {
      CStdString progressTrackingFile = m_item->GetPath();
      if (m_item->HasVideoInfoTag() && StringUtils::StartsWith(m_item->GetVideoInfoTag()->m_strFileNameAndPath, "removable://"))
      {
        progressTrackingFile = m_item->GetVideoInfoTag()->m_strFileNameAndPath;
      }
      else if (m_item->HasProperty("original_listitem_url") && URIUtils::IsPlugin(m_item->GetProperty("original_listitem_url").asString()))
      {
        progressTrackingFile = m_item->GetProperty("original_listitem_url").asString();
      }

      if (m_progressTrackingVideoResumeBookmark.timeInSeconds <= 0.0f)
        videodatabase.ClearBookMarksOfFile(progressTrackingFile, CBookmark::RESUME);
      else
        videodatabase.AddBookMarkToFile(progressTrackingFile, m_progressTrackingVideoResumeBookmark, CBookmark::RESUME);

      if (m_item->HasVideoInfoTag())
        m_item->GetVideoInfoTag()->m_resumePoint = m_progressTrackingVideoResumeBookmark;

      videodatabase.Close();
    }
    
    CApplicationMessenger::Get().PlayFile(*m_item);
  }
#endif
  //CGUIDialogKaiToast::QueueNotification(CGUIDialogKaiToast::Info, "Notice", m_CurrentDeviceName+" disconnected");
  
  m_CurrentDeviceName       = "";
  m_isMatchStickPlayerRunning  = false;
  m_isConnected                = false;
  m_isPlaying                  = false;
}
void CGUIMatchStick::InitPlayerState()
{
  m_isPlaying   = true;
  m_currentTime = 0;
  m_duration    = 0;
  m_percentage  = 0.0f;
}

void CGUIMatchStick::ShowMatchStickDialog(CFileItem item)
{
  *m_item = item;

  string videoName = item.GetProperty("title").asString();
  if (videoName == "")
  {
    videoName = item.GetLabel();
  }
  if (videoName == "")
  {
    videoName = "fireflytestvideo";
  }
  string videoUrl  = item.GetPath();

  ShowMatchStickDialog(videoName,videoUrl);
}

void CGUIMatchStick::ResetResumeBookmark()
{
  m_progressTrackingVideoResumeBookmark.Reset();
}

void CGUIMatchStick::UpdateResumeBookmark(CBookmark bookmark)
{
  m_progressTrackingVideoResumeBookmark.player = bookmark.player;
  m_progressTrackingVideoResumeBookmark.playerState = bookmark.playerState;
  m_progressTrackingVideoResumeBookmark.thumbNailImage= bookmark.thumbNailImage;
  m_progressTrackingVideoResumeBookmark.timeInSeconds = bookmark.timeInSeconds;
  m_progressTrackingVideoResumeBookmark.totalTimeInSeconds = bookmark.totalTimeInSeconds;
}

bool CGUIMatchStick::IsResumeMatchStick()
{
  return m_bResumeMatchStick;
}

void CGUIMatchStick::ResumeMatchStick(bool bResume)
{
  m_bResumeMatchStick = bResume;
}
