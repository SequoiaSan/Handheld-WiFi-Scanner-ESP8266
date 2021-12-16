#include "ScannerDisplay.h"

U8G2_SH1106_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, /* reset=*/ U8X8_PIN_NONE);

CScannerDisplay::CScannerDisplay()
{
}

void CScannerDisplay::Init()
{
  u8g2.begin();
  u8g2.setContrast(255);
  u8g2.clearBuffer();
  u8g2.setDrawColor(1);
  u8g2.setFont(u8g2_font_5x7_tr);
  u8g2.sendBuffer();  
}

void CScannerDisplay::DisplayScanListInfo(SScanInfoDisplay& scanInfo)
{
  uint8_t offsetY = 0;
  uint8_t offsetX = 0;
  u8g2.clearBuffer();
  u8g2.drawFrame(offsetX, offsetY, WEATHER_DISPLAY_W, WEATHER_DISPLAY_H);

  u8g2.setFont(u8g2_font_7x13B_tr);
  uint8_t fontHeight = u8g2.getMaxCharHeight(); 
  uint8_t fontWidth = u8g2.getMaxCharWidth();
  
  offsetX += 5;
  offsetY += 1 + fontHeight;
  u8g2.drawStr(offsetX, offsetY, scanInfo.m_ssid.c_str());

  offsetY += fontHeight + 2;

  u8g2.drawStr(offsetX, offsetY, scanInfo.m_bssid.c_str());

  u8g2.setFont(u8g2_font_tinytim_tf);
  fontHeight = u8g2.getMaxCharHeight(); 
  fontWidth = u8g2.getMaxCharWidth();
  
  offsetY += fontHeight + 5;

  u8g2.drawStr(offsetX, offsetY, (String("SGN: ") + String(scanInfo.m_rssi)).c_str());
  offsetY += fontHeight + 2;
  u8g2.drawStr(offsetX, offsetY, (String("CHN: ") + String(scanInfo.m_channel)).c_str());
  offsetY += fontHeight + 2;
  u8g2.drawStr(offsetX, offsetY, (String("ENC: ") + EncryptionTypeToString(scanInfo.m_encryptionType)).c_str());
  offsetY += fontHeight + 2;
  u8g2.drawStr(offsetX, offsetY, scanInfo.m_isHidden ? "<<Hidden>>" : ""); // Where it should go?

  offsetY -= fontHeight + 2;

  u8g2.setFont(u8g2_font_courB08_tn);
  String pageOutOf = String(scanInfo.m_currentAp) + '/' + String(scanInfo.m_totalAp);
  offsetX = u8g2.getDisplayWidth() - u8g2.getStrWidth((String(scanInfo.m_totalAp) + '/' + String(scanInfo.m_totalAp)).c_str()) - 5;
  u8g2.drawStr(offsetX, offsetY, pageOutOf.c_str());

  u8g2.drawFrame(offsetX - 6, offsetY - u8g2.getMaxCharHeight()- 3, WEATHER_DISPLAY_W, WEATHER_DISPLAY_H);

  u8g2.setFont(u8g2_font_open_iconic_arrow_2x_t);
  if(scanInfo.m_currentAp != scanInfo.m_totalAp)
  {
    u8g2.drawStr(offsetX - 5 - 20, offsetY + 5, "\x4F");
  }

  if(scanInfo.m_currentAp != 1)
  {
    u8g2.drawStr(offsetX - 6 - 35, offsetY + 5, "\x4C");
  }
  

  u8g2.sendBuffer();
}


void CScannerDisplay::ScanAnimation()
{
  unsigned long currentTime = millis();
  if(currentTime - m_prevAnimationTime > m_animationTime)
  {
    m_prevAnimationTime = currentTime;

    const short offsetX = 10;
    const short mutliplier = 2;
    
    u8g2.clearBuffer();
    for(int i = 0; i < 3; ++i)
    {
      u8g2.drawDisc(offsetX + 30 + 25 * i, WEATHER_DISPLAY_H/2 - 7, 5 * (m_animtaionCounter == i ? mutliplier : 1));
    }
  
    m_animtaionCounter = ++m_animtaionCounter % 3;
    
    u8g2.setFont(u8g2_font_7x13B_tr);
    const char* message = "Scanning";
    u8g2.drawStr(WEATHER_DISPLAY_W/2 - u8g2.getStrWidth(message)/2, 55, message);
    u8g2.sendBuffer();
  }
}

void CScannerDisplay::ToMonitorModeTransition()
{
  unsigned long currentTime = millis();
  if(currentTime - m_prevAnimationTime > m_animationTime)
  {
    m_prevAnimationTime = currentTime;
    
    const short offsetX = 10;
    const short mutliplier = 2;
    
    u8g2.clearBuffer();

    u8g2.drawDisc(offsetX + 30, WEATHER_DISPLAY_H/2 - 7, 5 * (m_animtaionCounter == 0 ? mutliplier : 1));
    u8g2.drawDisc(offsetX + 55, WEATHER_DISPLAY_H/2 - 7, 5 * (m_animtaionCounter == 1 ? mutliplier : 1));
    u8g2.drawDisc(offsetX + 80, WEATHER_DISPLAY_H/2 - 7, 5 * (m_animtaionCounter == 0 ? mutliplier : 1));
    
    m_animtaionCounter = ++m_animtaionCounter % 2;

    u8g2.setFont(u8g2_font_7x13B_tr);
    const char* message = "Monitor Mode";
    u8g2.drawStr(WEATHER_DISPLAY_W/2 - u8g2.getStrWidth(message)/2, 55, message);
    u8g2.sendBuffer();
  }
}

void CScannerDisplay::DisplayMonitorInfo(SScanInfoDisplay& scanInfo)
{
  uint8_t offsetY = 0;
  uint8_t offsetX = 0;
  u8g2.clearBuffer();
  u8g2.drawFrame(offsetX, offsetY, WEATHER_DISPLAY_W, WEATHER_DISPLAY_H);

  u8g2.setFont(u8g2_font_inb27_mr);
  uint8_t fontHeight = u8g2.getMaxCharHeight(); 
  uint8_t fontWidth = u8g2.getMaxCharWidth();

  if(scanInfo.m_rssi == NA)
  {
    offsetX += floor(WEATHER_DISPLAY_W/2) - fontWidth -10;
    offsetY += fontHeight - 5;
  
    u8g2.drawStr(offsetX, offsetY, "N/A");
  }
  else
  {
    offsetX += floor(WEATHER_DISPLAY_W/2) - 2*fontWidth;
    offsetY += fontHeight - 5;
  
    u8g2.drawStr(offsetX, offsetY, String(scanInfo.m_rssi).c_str());    
  }

  u8g2.setFont(u8g2_font_7x13B_tr);
  fontHeight = u8g2.getMaxCharHeight(); 
  fontWidth = u8g2.getMaxCharWidth();
  
  offsetX = 5;
  offsetY = WEATHER_DISPLAY_H - 7 - fontHeight;
  u8g2.drawStr(offsetX, offsetY, scanInfo.m_ssid.c_str());

  offsetY += fontHeight + 2;

  u8g2.drawStr(offsetX, offsetY, scanInfo.m_bssid.c_str());

  DrawSignalStrengthBar(scanInfo.m_rssi, 5, 5, 12, 25);
  DrawSignalStrengthBar(scanInfo.m_rssi, WEATHER_DISPLAY_W - 5 - 12, 5, 12, 25);
  
  u8g2.sendBuffer();
}

void CScannerDisplay::DrawSignalStrengthBar(int rssi, int x, int y, int width, int height)
{
  int percents = dBmtoPercentage(rssi);

  u8g2.drawHLine(x, y, width);
  u8g2.drawHLine(x, y + height, width);

  if(rssi != NA && height > 0)
  {
    int barHeight = floor((height / 100.f) * percents);
    u8g2.drawBox(x, y + height - barHeight, width, barHeight);
  }
}

void CScannerDisplay::DisplayError(const char* error)
{
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_7x13B_tr);
  u8g2.drawStr(16, 63, error);
  u8g2.sendBuffer();
}

String CScannerDisplay::EncryptionTypeToString(short encType)
{
  switch(encType)
  {
    case 5:
      return "WEP";
    case 2:
      return "TKIP";
    case 4:
      return "CCMP";
    case 7:
      return "NONE";
    case 8:
      return "AUTO";
    default:
      return "FAIL";
  }
}

int CScannerDisplay::dBmtoPercentage(int dBm)
{
  const int RSSI_MAX =-50;// define maximum strength of signal in dBm
  const int RSSI_MIN =-100;// define minimum strength of signal in dBm
  
  int quality;
    if(dBm <= RSSI_MIN)
    {
        quality = 0;
    }
    else if(dBm >= RSSI_MAX)
    {  
        quality = 100;
    }
    else
    {
        quality = 2 * (dBm + 100);
   }

     return quality;
}
