// vim: filetype=arduino

extern float currentSpeed;
extern unsigned int totalDistance;
extern unsigned long effectiveTime;
extern volatile boolean paused;

#include <stdlib.h>

#define changeSecondLine 3
#define numberOfSecondLine 3
#define pause_str "P"
#define speed_str "|KMH: "
#define distance_str "|M: "
#define time_str "|T: "

unsigned long lastSecondLineChange = 0;
int currentSecondLine = 0;
float avgSpeed = 0;

String floatToString(float float_val)
{
    char number[6];
    if ((int) float_val > 99) {
        return dtostrf(float_val, 5, 1, number);
    } else {
        return dtostrf(float_val, 5, 2, number);
    }
}

String getCurrentSpeedLine()
{
    String line;
    line.reserve(16);
    line += String(speed_str);
    line += floatToString(currentSpeed);
    return line;
}

String getDistanceLine()
{
    String line;
    line += String(distance_str);
    line += totalDistance;
    return line;
}

float getAverageSpeed() {
    if (effectiveTime == 0) {
        return 0;
    } else {
        return (float) ((float) totalDistance / effectiveTime) * 3600;
    }
}

String prettyDigits(int digits){
    // utility function for digital clock display: prints preceding colon and leading 0
    String output = ":";
    if(digits < 10)
        output += '0';
    output += digits;
    return output;
}

String elaspedTime(unsigned long time){
    time = time / 1000UL;
    int hours = hour(time);
    int minutes = minute(time);
    int seconds = second(time);

    String timeStr = String(time);
    return timeStr;
}

String getTimeLine() {
    String time;
    time = String(time_str + elaspedTime(effectiveTime));
    return time;
}

String getAvgSpeedLine() {
    String line;
    line = String("|KMA:");
    line += floatToString(avgSpeed);
    return line;
}

void displayInfo() {

    String finalString = "";
    
    finalString += String("PRJW");
    if (paused) {
        finalString += String("|KMH: 0");
    } else {
        finalString += getCurrentSpeedLine(); //KM/H
    }
    if ( (millis() - lastSecondLineChange) > ((unsigned long) changeSecondLine * 1000UL)) {
        currentSecondLine = (currentSecondLine + 1) % numberOfSecondLine;
        lastSecondLineChange = millis();
        avgSpeed = getAverageSpeed();
    }
    finalString += getDistanceLine(); //Distance (KM)
    finalString += getAvgSpeedLine(); //Average KM/H
    finalString += getTimeLine();     //Time (in seconds)
    Serial.println(finalString);
}
