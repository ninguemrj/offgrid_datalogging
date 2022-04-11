# offgrid_datalogging
  
  I'm using a Arduino mega+ESP01+RTC DS1307+SD Card shield, to implement data 
  logging and automation project.
  
  This code uses RS232 Serial commands to acquire energy data from my offgrid solar inverter.
  
  In the future I will add 16 temperature sensors for monitoring the temperature of each lead acid battery from my battery bank (16 x 100Ah -- 24v x 800Ah).
  
  My plans consider:
  1. Arduino MEGA
     - Concentrates all data acquisition (from inverter and other sensors);
     - Stores all data on SD CARD;
     - Sends all data to a webserver (checking in the SD CARD for non uploaded contents);
  2. Raspberry pi 3 or Cloud Server (running a webserver+PHP+mariaDB)
     - Receives the information from Arduino for storing on MariaDB;
     - Provides a webpage with dashboard information from MariaDB data.

        
STATUS: pre-alfa: testing communication with solar inverter, response time, parsing inversor feedback into meaningful variables

Special THANKS for the thread below, on Arduino forum, for all shared knowledge on communicating with solar Inverter, specialy for the user "athersaleem".
https://forum.arduino.cc/t/rs232-read-data-from-mpp-solar-inverter/600960/36

Athersaleem´s youtube list with valuable information (Arduino Talking with Solar Inverter):
https://www.youtube.com/watch?v=dlhsc_KjIM4&list=PLDW1zKN_tjaJDp-Ukz8KApchBgA60AFC_


TO DO LIST:
  1. function "askInverter": 
     - Test the feedback string before returning the QPIGS response string (CRC check and size check -- create a "correct size" constant for each command)
     - Implement other commands between Arduino and inverter (check OUTPUT PRIORITY, and other useful info) 
  2. create rules to activate OUTPUT PRIORITY as SOLAR or SBU (time rules, AC OUT LOAD rules and etc...)
  3. start to use SD Card for data logging (define data structure, not uploaded info and etc.)
  4. start to use wifi (when available) for data logging in the cloud (checking in the SD CARD for non uploaded contents)
  5. add 16 temperature sensors with ADS1115 ADC (15/16bits)
