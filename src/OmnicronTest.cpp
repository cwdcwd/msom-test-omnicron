/*
 * Copyright (c) 2024 Particle Industries, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "Particle.h"
#include "location.h"

SYSTEM_MODE(AUTOMATIC);
SYSTEM_THREAD(ENABLED);
SerialLogHandler logHandler(LOG_LEVEL_INFO);

void getCb(LocationResults results);

const double checkTime = 3000;

LocationPoint point = {};
bool ready = true;
double lastCheck = 0;
boolean blnGotLocation = false;
SystemSleepConfiguration sleepConfig;

void setup()
{
  waitFor(Serial.isConnected, 10000);
  Log.info("Starting up");
  LocationConfiguration config;
  config.enableAntennaPower(GNSS_ANT_PWR);
  Location.begin(config);

  
  sleepConfig.mode(SystemSleepMode::ULTRA_LOW_POWER).duration(60min);
}

void getCb(LocationResults results)
{
  Log.info("async callback returned %d", (int)results);

  if (results == LocationResults::Fixed) { 
    Log.info("async callback reporting fixed");
    blnGotLocation = true;
  }
}

void loop()
{
  if (millis() > (lastCheck + checkTime))
  {

    if(blnGotLocation) {
      auto fixed = (point.fix) ? true : false;
      if (fixed)
      {
        Log.info("Position fixed!");
        Log.info("Lat %0.5lf, lon %0.5lf", point.latitude, point.longitude);
        Log.info("Alt %0.1f m, speed %0.1f m/s, heading %0.1f deg", point.altitude, point.speed, point.heading);
        blnGotLocation = false;

        Log.info("got location so going to sleep for a bit...");
        SystemSleepResult result = System.sleep(sleepConfig);
        switch (result.wakeupReason())
        {

        case SystemSleepWakeupReason::BY_GPIO:
        {
          Log.info("was woken up by GPIO");
          break;
        }
        case SystemSleepWakeupReason::BY_RTC:
        {
          Log.info("was woken up by the RTC (after a specified number of seconds)");
          break;
        }
        case SystemSleepWakeupReason::BY_NETWORK:
        {
          Log.info("was woken up network)");
          break;
        }
        case SystemSleepWakeupReason::UNKNOWN:
        default:
        {
          Log.info("unknown reason tp wake up from sleep");
          break;
        }
        }
      }
      else
      {
        Log.info("Position not fixed. :(");
      }
    } else {
      Location.getLocation(point, getCb, true);
      Log.info("GNSS acquistion started");
    }

    lastCheck = millis();
  }

  
}