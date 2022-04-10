String acIn_volts="";
String acIn_freq="";
String acOut_volts="";
String acOut_freq="";
String acOut_va="";
String acOut_watts="";
String acOut_loadPerc="";
String dcBat_volts="";
String dcBat_chargAmp="";
String dcBat_capPerc="";
String inv_heatSink="";
String pvIn_amp="";
String pvIn_volts="";
String dcBat_voltsSCC="";
String dcBat_dischargAmp="";
String inv_status="";


    

void setup() {
  // Open serial communications and wait for port to open:
  Serial.begin(9600);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }

  // send an intro:
  Serial.println("\n\nString  substring():");
  Serial.println();
}

void loop() {
  // Set up a String:
  String stringOne = "(230.1 49.9 230.1 49.9 1035 0965 012 361 00.00 002 000 0024 00.";
  Serial.println(stringOne);
  Serial.println("");

  // substring(index) looks for the substring from the index position to the end:

   acIn_volts = stringOne.substring(1, 6);
   acIn_freq = stringOne.substring(7, 11 );
   acOut_volts = stringOne.substring(12, 17);
   acOut_freq = stringOne.substring(18, 22);
    acOut_va = stringOne.substring(23, 27);
    acOut_watts = stringOne.substring(28, 32);
    acOut_loadPerc = stringOne.substring(33, 36);
    dcBat_volts = stringOne.substring(41, 47);
    
    Serial.println("AC entrada tensao: " + acIn_volts);
    Serial.println("AC entrada frequencia: " + acIn_freq);
    Serial.println("AC saida tensao: " + acOut_volts);
    Serial.println("AC saida frequencia: " + acOut_freq);
    Serial.println("AC saida potencia aparente (va): " + acOut_va);
    Serial.println("AC saida potencial ativa (w): " + acOut_watts);
    Serial.println("AC saida percentual carga: " + acOut_loadPerc);
    Serial.println("DC bateria tensao: " + dcBat_volts);
    
    
    

  // do nothing while true:
  while (true);
}
