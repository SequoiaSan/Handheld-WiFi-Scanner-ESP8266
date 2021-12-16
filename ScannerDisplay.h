#include <U8g2lib.h>

#ifdef U8X8_HAVE_HW_SPI
#include <SPI.h>
#endif
#ifdef U8X8_HAVE_HW_I2C
#include <Wire.h>
#endif

#define WEATHER_DISPLAY_W 128
#define WEATHER_DISPLAY_H 64

#define NA 0

class CScannerDisplay
{
  public:

  struct SScanInfoDisplay
  {
    String m_ssid = "N/A";
    uint8_t m_encryptionType = NA;
    int32_t m_rssi = NA;
    String m_bssid = "N/A";
    int32_t m_channel = NA;
    bool m_isHidden = false;

    uint8_t m_currentAp;
    uint8_t m_totalAp;
  };
  
  CScannerDisplay();
  void Init();

  void ScanAnimation();
  void ToMonitorModeTransition();
  void DisplayScanListInfo(SScanInfoDisplay& scanInfo);
  void DisplayMonitorInfo(SScanInfoDisplay& scanInfo);
  void DisplayError(const char* error);

  void ResetAnimations();

  private:
    void DrawSignalStrengthBar(int rssi, int x_lt, int y_lt, int x_rb, int y_rb);
    String EncryptionTypeToString(short encType);
    int dBmtoPercentage(int dBm);

    int m_animtaionCounter = 0;
    unsigned long m_animationTime = 350;
    unsigned long m_prevAnimationTime = 0;
  
};
