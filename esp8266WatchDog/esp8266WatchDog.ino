#include <Ticker.h>

Ticker secondTick;

int watchDogCount =0;

void IsrWatchdogHandler()
{
   watchDogCount++;
   if(watchDogCount >5)
   {
       Serial.println ();
       Serial.println("Watchdog byte occured !!!!!!");
       ESP.reset();
   }
}

void setup() {
  Serial.begin(115200);
  secondTick.attach(1, IsrWatchdogHandler);
}

void loop() {
  
  Serial.printf("Watch dog counter : %d \r\n", watchDogCount);
  watchDogCount =0;
  delay (1000);
}
