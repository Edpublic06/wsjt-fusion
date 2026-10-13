// Personal WSJT-Z extension, 2026-10-13. GPL-3.0-or-later.
#ifndef WSJTZ_AUTOMATION_SETTINGS_HPP
#define WSJTZ_AUTOMATION_SETTINGS_HPP
#include <QSettings>
namespace wsjtz {
inline bool readCallerPreference (QSettings& settings,bool legacy) {
  return settings.value ("Edpublic06/PreferIncomingCalls",
    settings.value ("PreferIncomingCalls",legacy)).toBool ();
}
inline int readWdCounter (QSettings& settings) {
  return qBound (0,settings.value ("Edpublic06/WDCounter",
    settings.value ("QueuedStationMaxCalls",0)).toInt (),999);
}
inline void writeAutomationSettings (QSettings& settings,bool prefer,int counter) {
  settings.setValue ("Edpublic06/PreferIncomingCalls",prefer);
  settings.setValue ("Edpublic06/WDCounter",qBound (0,counter,999));
}
}
#endif
