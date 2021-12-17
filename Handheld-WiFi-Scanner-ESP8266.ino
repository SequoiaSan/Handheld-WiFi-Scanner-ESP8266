#include "ESP8266WiFi.h"
#include "AwesomeClickButton.h"
#include "ScannerDisplay.h"
#include "DebugHelpers.h"

enum EContext
{
  INVALID = -1,
  ERROR = 0,
  SCAN_ANIMATION,
  MONITOR_ANIMATION,
  SCAN_PAGE ,
  MONITOR_PAGE
} g_context = INVALID;

#define SCAN_BUTTON_PIN D10
#define SELECT_BUTTON_PIN D2

CScannerDisplay scannerDisplay;

bool pageChanged = true;
int g_totalAp = 0;
int g_currentNetworkIndex = 0;

bool g_apLocked = false;
String g_bssid;
String g_ssid;

bool IsAnyAPLocked()
{
  return g_apLocked;
}

void LockAP()
{
  g_bssid = WiFi.BSSIDstr(g_currentNetworkIndex); 
  g_ssid = WiFi.SSID(g_currentNetworkIndex);
  g_apLocked = true;
}

void UnlockAP()
{
  g_apLocked = false;
}

bool FindLockedAP(int& foundAPIndex)
{
  if(IsAnyAPLocked())
  {
    for(int i = 0; i < g_totalAp; ++i)
    {
      if(strcmp(g_bssid.c_str(), WiFi.BSSIDstr(i).c_str()) == 0)
      {
        foundAPIndex = i;
        return true;
      }
    }
  }
  return false;
}

AwesomeClickButton scanButton(SCAN_BUTTON_PIN);
void onScanButtonClicked() 
{
  DEBUG_LOG_LN(F("Scan Button Click"));

  if(g_context != EContext::SCAN_PAGE)
  {
    return;
  }
  
  g_currentNetworkIndex = --g_currentNetworkIndex < 0 ? g_totalAp - 1 : g_currentNetworkIndex;
  pageChanged = true;

  DEBUG_LOG(F("Scan Button Processing Index: "));
  DEBUG_LOG_LN(g_currentNetworkIndex);
}

void onScanLongClickListener(int duration) 
{
  DEBUG_LOG_LN(F("Scan Button Long Click"));

  if(g_context < EContext::SCAN_PAGE)
  {
    return;
  }
  
  DEBUG_LOG_LN(F("Scan Button Long Click Processing"));
    
  UnlockAP();
  ScanNetworks();
}

AwesomeClickButton selectButton(SELECT_BUTTON_PIN);
void onSelectButtonClicked() 
{
  DEBUG_LOG_LN(F("Select Button Click"));

  if(g_context != EContext::SCAN_PAGE)
  {
    return;
  }

  DEBUG_LOG_LN(F("Select Button Click Processing"));
  
  g_currentNetworkIndex = ++g_currentNetworkIndex % g_totalAp;
  pageChanged = true;

  DEBUG_LOG(F("Select Button Click Processing "));
  DEBUG_LOG_LN(g_currentNetworkIndex);
}

void onSelectLongClickListener(int duration) 
{
  DEBUG_LOG_LN(F("Select Button Long Click"));

  if(g_context < EContext::SCAN_PAGE)
  {
    return;
  }
  
  DEBUG_LOG_LN(F("Select Button Long Click processing"));
  
  switch(g_context)
  {
    case SCAN_PAGE:
      LockAP();
      MonitorNetwork();
      g_context = EContext::MONITOR_ANIMATION;
      break;
    case MONITOR_PAGE:
      UnlockAP();
      ScanNetworks();
      break;    
  }
}

void setup() {
  Serial.begin(9600);

  WiFi.mode(WIFI_STA);
  WiFi.disconnect();

  scanButton.setOnClickListener(onScanButtonClicked);
  scanButton.setOnLongClickListener(onScanLongClickListener);
  scanButton.setSingleClickTime(65);
  scanButton.setDebounceTime(25);
  
  selectButton.setOnClickListener(onSelectButtonClicked);
  selectButton.setOnLongClickListener(onSelectLongClickListener);
  selectButton.setSingleClickTime(65);
  selectButton.setDebounceTime(25);

  scannerDisplay.Init();

  ScanNetworks();
}

void OnScanComplete(int totalAps)
{
  g_totalAp = totalAps;
  g_currentNetworkIndex = 0;
  pageChanged = true;
  if(g_totalAp > 0)
  {
    g_context = EContext::SCAN_PAGE;
  }
  else
  {
    DEBUG_LOG_LN(F("No APs found. Scan again."));
    scannerDisplay.DisplayError("No APs found. Scan again.");
    g_context = EContext::ERROR;
  }  
}

void OnMonScanComplete(int totalAps)
{
  DEBUG_LOG_LN(F("DisplayMonitorInfo()"));
  
  g_totalAp = totalAps;
  
  if(g_totalAp > 0)
  {
    CScannerDisplay::SScanInfoDisplay scanInfo;
    scanInfo.m_bssid = g_bssid;
    scanInfo.m_ssid = g_ssid;
    
    int foundAp = 0;
    if(FindLockedAP(foundAp))
    {
      scanInfo.m_currentAp = foundAp;
      
      uint8_t* bssid_fake;
      if(WiFi.getNetworkInfo(scanInfo.m_currentAp, scanInfo.m_ssid, scanInfo.m_encryptionType, scanInfo.m_rssi, bssid_fake, scanInfo.m_channel, scanInfo.m_isHidden))
      {
        scannerDisplay.DisplayMonitorInfo(scanInfo);
      }
      else
      {
        DEBUG_LOG_LN(F("WiFi.getNetworkInfo() - Failed"));
        scanInfo.m_rssi = NA;
        scannerDisplay.DisplayMonitorInfo(scanInfo);    
      }
    }
    else
    {
      DEBUG_LOG_LN(F("AP LOST"));
      scanInfo.m_rssi = NA;
      scannerDisplay.DisplayMonitorInfo(scanInfo);    
    }
  }
  else
  {
    DEBUG_LOG_LN(F("Monitor scan failed"));
    scannerDisplay.DisplayError("Monitor scan failed");
  }

  if(g_context == EContext::MONITOR_ANIMATION)
  {
    g_context = EContext::MONITOR_PAGE;
  }
}

void ScanNetworks()
{
  DEBUG_LOG_LN(F("ScanNetworks()"));
  
  WiFi.scanDelete();
  const bool showHidden = true;
  WiFi.scanNetworksAsync(OnScanComplete, showHidden);

  g_context = EContext::SCAN_ANIMATION;
}

bool MonitorNetwork()
{
  if(WiFi.scanComplete() >= 0)
  {
    DEBUG_LOG_LN(F("MonitorNetwork()"));
    
    WiFi.scanDelete();
    const bool showHidden = true;
    WiFi.scanNetworksAsync(OnMonScanComplete, showHidden);
  }
}

void DisplayScannedAP()
{
  if(pageChanged)
  {
    DEBUG_LOG_LN(F("DisplayScannedAP() - pageChanged"));
    pageChanged = false;
    
    CScannerDisplay::SScanInfoDisplay scanInfo;
    scanInfo.m_currentAp = g_currentNetworkIndex + 1;
    scanInfo.m_totalAp = g_totalAp;
    scanInfo.m_bssid = WiFi.BSSIDstr(scanInfo.m_currentAp);
    
    uint8_t* bssid_fake;
    if(WiFi.getNetworkInfo(g_currentNetworkIndex, scanInfo.m_ssid, scanInfo.m_encryptionType, scanInfo.m_rssi, bssid_fake, scanInfo.m_channel, scanInfo.m_isHidden))
    {
      scannerDisplay.DisplayScanListInfo(scanInfo);
    }  
    else
    {
      DEBUG_LOG_LN(F("FAILED to retrieve scan result info."));
      scannerDisplay.DisplayError("FAILED to retrieve scan result info.");
    }
  }
}

void loop() 
{
  scanButton.update();
  selectButton.update();
  
  switch(g_context)
  {
    case EContext::SCAN_PAGE:
      DisplayScannedAP();
      break;
    case EContext::MONITOR_PAGE:
      MonitorNetwork();
      break;
    case EContext::SCAN_ANIMATION:
      scannerDisplay.ScanAnimation();
      break;
    case EContext::MONITOR_ANIMATION:
      scannerDisplay.ToMonitorModeTransition();
      break;
    case EContext::ERROR:
      break;
    default:
      break;
  }
}
