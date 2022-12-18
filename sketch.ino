/*
 * Bike
 *
 *  https://github.com/reefab/CyclingPusher
 *
 *  CyclingPusher is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  CyclingPusher is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *  You should have received a copy of the GNU General Public License
 *  along with CyclingPusher.  If not, see <http://www.gnu.org/licenses/>.
 *
 *  Created on: 25.02.2012
 *      Author: reefab@demenzia.net
 */

#include "Time.h"
#include <EEPROM.h>

#include "persistent.h"

// This needs to be pin 2 to use interrupt 0
#define reedPin 2
// Number of 'wheel' turns needed to recalculate speed/total distance
#define interval 5
// Minimal number of millisecond between reed switch changes to prevent bounce
#define reedRes 100
// Timeout in seconds, if the reed switch is not activated during this time, pause everything
#define timeOut 2
// Minimum distance for a valid activity (in meters)
#define minDistance 500
// least amount of effective time for a valid activity (in secs)
#define minTime 60
// Amount of inactive time before either automatic upload or discarding of current session
#define maxTime 60
// Save session data every X seconds
#define saveInterval 120
// Beep shortly every X meters
#define beepInterval 5000

// Global Vars
const float meterPerTurn = 6;

unsigned int nbRotation = 0;
unsigned int updateCount = 0;
unsigned int totalDistance = 0;
unsigned int lastBeep = 0;
unsigned int beepCount = 0;
unsigned long lastUpdate = 0;
unsigned long currentTime = 0;
unsigned long startTime = 0;
unsigned long enterLoop = 0;
unsigned long exitLoop = 0;
unsigned long time_elasped = 0;
unsigned long effectiveTime = 0;
unsigned long lastSave = 0;
volatile boolean paused = false;
volatile boolean resetRequested = false;
volatile boolean start = false;
volatile unsigned long lastReedPress = 0;
volatile unsigned int rotationCount = 0;
// For display during init and sending via API

float currentSpeed = 0;
boolean done = false;
boolean uploaded = false;

#include "display.h"

void setup() {
  Serial.begin(9600);

  // Disabling SD slot
  pinMode(4, OUTPUT);
  digitalWrite(4, HIGH);

  pinMode(reedPin, INPUT);

  // Reed switch handling by interrupt
  lastReedPress = millis();
  attachInterrupt(0, turnCounter, RISING);

  // Beeper init
  pinMode(A0, OUTPUT);
  digitalWrite(A0, HIGH);
}

void reset(boolean startNew=false)
{
  rotationCount = 0;
  updateCount = 0;
  totalDistance = 0;
  currentSpeed = 0;
  done = false;
  uploaded = false;
  lastReedPress = millis();
  lastUpdate = 0;
  currentTime = 0;
  startTime = 0;
  time_elasped = 0;
  effectiveTime = 0;
  if (startNew) {
    start = false;
  }
  paused = false;
}

void loop() {
  enterLoop = millis();
  delay(100);
  // Activity finished & api push
  
  // Activity in progres
  // Start a new session if requested
  // Update turns and distance
  nbRotation = rotationCount - updateCount;
  if (nbRotation >= interval)
  {
    currentTime = millis();
    time_elasped = currentTime - lastUpdate;
    float distance = nbRotation * meterPerTurn;
    currentSpeed = ((float) distance / (float) time_elasped) * 3600;
    totalDistance = rotationCount * meterPerTurn;
    updateCount = rotationCount;
    lastUpdate = currentTime;
  }
  // Update time
  if((millis() - lastReedPress < ((unsigned long) 1000 * timeOut)) && (rotationCount > 0))
  {
    exitLoop = millis();
    effectiveTime = effectiveTime + (exitLoop - enterLoop);
  } else if((millis() - lastReedPress) < ((unsigned long) 1000 * maxTime)) {
    // Automatic activity pause
    paused = true;
    currentSpeed = 0;
  }

  // Beep if needed
  if(totalDistance >= (beepCount * beepInterval)) {
    beepCount++;
  }
  displayInfo();
}

void turnCounter() {
  if (millis() - lastReedPress > reedRes)
  {
    // Start a new session at first pedal turn
    if (rotationCount == 0) {
      resetRequested = true;
      start = true;
    }
    if (paused) paused = false;

    rotationCount++;
  }
  lastReedPress = millis();
}

boolean isSessionValid() {
  return ((totalDistance > (unsigned int) minDistance) && (effectiveTime > ((unsigned long) minTime * 1000)));
}
