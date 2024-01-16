#include <SoftwareSerial.h>
#include "Arduino.h"
#include "SoftwareSerial.h"
#include "DFRobotDFPlayerMini.h"

#define sw3 8 //SW3 8번 연결
int reading3 = HIGH; //SW3의 press 유무
int previous3 = HIGH;
  

SoftwareSerial mySerial(2, 3); // RX, TX
SoftwareSerial MP3Module(4, 5);
DFRobotDFPlayerMini myDFPlayer;

String temp = "TEST1"; //TEST STRING

byte data; //RECEIVE MYSERIAL'S DATA

byte Buffer[700]; //SAVE DATAS 

int i = 0; //USING BUFFER'S INDEX
int iminus = 0;
int full_scan = 0;

int k = 0;

char beaconA[2]; //SAVE BEACON A RSSI IN CHAR
char beaconB[2]; //SAVE BEACON B RSSI IN CHAR
char beaconC[2]; //SAVE BEACON C RSSI IN CHAR
char beaconD[2]; //SAVE BEACON D RSSI IN CHAR

int rssi_final[4]; //SAVE BEACONS RSSI IN INT

int musicOneFlag = 0;
int musicTwoFlag = 0;
int musicThreeFlag = 0;
int musicFourFlag = 0;

int min_Rssi;

int closestBusIndex = -1;

void mySerialListen();
void SerialWrite();
void timeCheck();

unsigned long prev = 0; //FOR TIME CHECK

//-----------------------------------------------------------
//  [1] SETUP 
//-----------------------------------------------------------
void setup()
{
  pinMode(8, INPUT_PULLUP);

  Serial.begin(9600);
  mySerial.begin(9600);
  MP3Module.begin(9600);
  
  Serial.println();
  Serial.println(F("DFRobot DFplayer Mini Demo"));
  Serial.println(F("Initializing DFplater ... (May take 3~5 seconds)"));

  if(!myDFPlayer.begin(MP3Module), false) { //객체 초기화
    Serial.println(F("Unable to begin:"));
    Serial.println(F("1.Please recheck the connection!"));
    Serial.println(F("2.Please insert the SD card!"));
    while(true);
  }

  Serial.println(F("DFplayer Mini online"));

  myDFPlayer.setTimeOut(500);  //시리얼 통신용 타임아웃 시간 저장

  //------볼륨조절--------
  myDFPlayer.volume(30); //0-30사이의 값을 인수로 입력

  //----------------이퀄라이즈 모드 지정 시 사용-------------
  myDFPlayer.EQ(DFPLAYER_EQ_NORMAL);  //일반모드

}  //END OF SETUP

//—————————————————————————————
//  [2] LOOP
//—————————————————————————————
void loop() // START LOOP
{ 
  reading3 = digitalRead(sw3);
  if(reading3 == LOW && previous3 == HIGH) {
    for(int i=0; i<700; i++){
      Buffer[i]=0;  
    }
    
    mySerial.listen();
    mySerial.write("AT+DISC?");
    Serial.println("----------------AT+DISC?----------------");

    i = 0;
    k = 0;
    iminus = 0;
    full_scan = 0; 
    previous3 = LOW;
  }

  else if(reading3 == HIGH && previous3 == LOW) {
    //Buffer 정보 저장 시작
    if(mySerial.available()){
      data = mySerial.read();
      Buffer[i++] = data;
      iminus = i-4;  //0부터 9까지 10개의 데이터 입력 //끝에서부터 5글자는 5번째
    }
    
    //스캔 종료 감지 필요
    for(int g=0; g<iminus; g++){
      if(Buffer[g]=='D' && Buffer[g+1]=='I' && Buffer[g+2]=='S' && Buffer[g+3]=='C' && Buffer[g+4]=='E') {
        Serial.println("Scan End");
        full_scan = 1;
      }
    }
    
    if(full_scan == 1){
      for(int i=0; i<700; i++){
        Serial.write(Buffer[i]);  
      }  

      int buffer_address = (int*)Buffer;//= Buffer[0]

      //TYPE CASTING BYTE TO STRING
      String test = String((char*)Buffer);

      //GET MAC ADDRESS'S POSITION
      // (char*)Buffer == Buffer라는 자료가 저장된 메모리 위치를 반환한다
      int positions = strstr((char*)Buffer, "E415F6604134"); //11111111
      int positions2= strstr((char*)Buffer, "E415F660237F"); //22222222
      int positions3= strstr((char*)Buffer, "FC45C3E0A622"); //33333333
      int positions4= strstr((char*)Buffer, "5051A9FEF618");


      //GET RSSI'S POSITION
      int rssi_A = (positions - buffer_address) + 22;
      int rssi_B = (positions2 - buffer_address) + 22;
      int rssi_C = (positions3 - buffer_address) + 22;
      int rssi_D = (positions4 - buffer_address) + 22;

      //OK+DISC:00000000:00000000000000000000000000000000:0000000000:C6158A2D1D25:-042

      //ASSIGN CHAR ARRAY WITH RSSI
      beaconA[0] = test[rssi_A];
      beaconA[1] = test[rssi_A+1];
      beaconB[0] = test[rssi_B];
      beaconB[1] = test[rssi_B+1];
      beaconC[0] = test[rssi_C];
      beaconC[1] = test[rssi_C+1];
      beaconD[0] = test[rssi_D];
      beaconD[1] = test[rssi_D+1];

      //TYPE CASTING CHAR TO INT
      //charToInt();
      rssi_final[0] = (beaconA[0]-'0')*10 + (beaconA[1]-'0');
      rssi_final[1] = (beaconB[0]-'0')*10 + (beaconB[1]-'0');
      rssi_final[2] = (beaconC[0]-'0')*10 + (beaconC[1]-'0');
      rssi_final[3] = (beaconD[0]-'0')*10 + (beaconD[1]-'0');


      //PRINT RSSI_FINAL ARRAY
      for(int i=0; i<3; i++){
        if(rssi_final[i] == -528){
          rssi_final[i] = 100; //rssi값은 99보다 클 수 없음.
        }
      }

      Serial.print("\n");
      Serial.print("BeaconA: ");
      Serial.print(rssi_final[0]); //BEACON A
      Serial.print("  ");
      Serial.print(" BeaconB: ");
      Serial.print(rssi_final[1]); //BEACON B
      Serial.print("  ");
      Serial.print(" BeaconC: ");
      Serial.print(rssi_final[2]); //BEACON C
      Serial.print("\n");

      //  RSSI 최소값 구하기
      min_Rssi = rssi_final[0];
      closestBusIndex = 0;
      for(k=1; k<3; k++){
        if(rssi_final[k] < min_Rssi){
          min_Rssi = rssi_final[k];
          closestBusIndex = k ;
        }
      }
      if (min_Rssi == 100) closestBusIndex = -1;
    
      Serial.print("min Rssi: ");
      Serial.print(min_Rssi);
      Serial.print(",   closestBusIndex= ");
      Serial.println(closestBusIndex);
      //END OF PRINTRSSI()

      //PLAY MUSIC
      myDFPlayer.playMp3Folder(closestBusIndex+1);
      previous3 = HIGH;
    }
  }
} //END OF LOOP