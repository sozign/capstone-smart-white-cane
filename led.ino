#define TRIG_1 9 //TRIG 핀 설정 (초음파 보내는 핀)
#define ECHO_1 8 //ECHO 핀 설정 (초음파 받는 핀)
#define TRIG_2 11 //TRIG 핀 설정 (초음파 보내는 핀)
#define ECHO_2 10 //ECHO 핀 설정 (초음파 받는 핀)
#define VIBRATION 13 // 진동모터 핀 설정 
#define sw1 12 //SW1 12번 핀 연결
#define sw2 7 //SW2 7번 핀 연결
#define led 6 //led 6번 핀 연결 
#define cds A0 //조도센서 A0핀 연결

int reading1 = HIGH; //SW1 현재 상태
int previous1 = HIGH; //SW1 이전 상태

int reading2 = HIGH; //SW2 현재 상태
int previous2 = HIGH; //SW2 이전 상태

int state = HIGH; // led on/off 상태 

long time1 = 0; //SW1이 처음 눌리는 시간
long time2 = 0; //SW2이 처음 눌리는 시간 
long debounce1 = 100; //SW1 Debounce 타임 설정
long debounce2 = 100; //SW2 Debounce 타임 설정 

unsigned long curr1=0;
unsigned long curr2=0;
unsigned long prev1=0;
unsigned long prev2=0;


void setup() {
  Serial.begin(9600); 

  pinMode(TRIG_1, OUTPUT);
  pinMode(ECHO_1, INPUT);
  pinMode(TRIG_2, OUTPUT);
  pinMode(ECHO_2, INPUT);
  pinMode(VIBRATION, OUTPUT);
  pinMode(cds, INPUT);
  pinMode(led, OUTPUT); // LED 설정
  pinMode(sw1, INPUT_PULLUP);
  pinMode(sw2, INPUT_PULLUP); // SW2를 설정, 아두이노 풀업저항 사용
  
}

void loop()

{
  reading1 = digitalRead(sw1);  //SW 누름 상태 여부
  reading2 = digitalRead(sw2);
  
  
  long duration_1, distance_1; // 초음파 1의 왕복 시간 및 편도 거리
  long duration_2, distance_2; // 초음파 2의 왕복 시간 및 편도 거리

  digitalWrite(TRIG_1, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_1, HIGH);
  delayMicroseconds(2);
  digitalWrite(TRIG_1, LOW);
  duration_1 = pulseIn(ECHO_1, HIGH);

  digitalWrite(TRIG_2, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_2, HIGH);
  delayMicroseconds(2);
  digitalWrite(TRIG_2, LOW);
  duration_2 = pulseIn(ECHO_2, HIGH);

  delay(100);

  distance_1 = duration_1 * 17 / 1000; // 초음파 1의 편도 거리 계산
  distance_2 = duration_2 * 17 / 1000; // 초음파 2의 편도 거리 계산


  //------------------------------기능 1------------------------------//
  if (reading1 == LOW && previous1 == HIGH && millis()-time1 > debounce1) { // 버튼 on
    Serial.println("1st!!!!!!!!!");
    previous1 = LOW;
    time1 = millis();
    delay(100);
  }

  else if (reading1 == HIGH && previous1 == LOW) { // 버튼 이전 상태 유지 
    Serial.println("2nd!!!!!!!!!!!!");
    curr1 = millis();
    
      if (distance_1 < 100 || distance_2 < 100){ 
        if (curr1 - prev1 < 200){ 
          digitalWrite(VIBRATION, HIGH);
          
        }
        else if (curr1 - prev1 > 200 && curr1 - prev1 < 700){
          digitalWrite(VIBRATION, LOW);
        }
        else if (curr1 - prev1 > 700) {
          prev1 = curr1;
          digitalWrite(VIBRATION, LOW);
        }
      }
    
      else{ // 장애물 없는 경우 
        digitalWrite(VIBRATION, LOW);
      }
  }
  

  else if (reading1 == LOW && previous1 == LOW){
    Serial.println("3rd!!!!!!!!!!!!!!");
    //time1 = millis();
    digitalWrite(VIBRATION, LOW);
    previous1 = HIGH;
    reading1 = HIGH;
    delay(100);
    
  }

  //------------------------------기능 2------------------------------//
  if (reading2 == LOW && previous2 == HIGH && millis()-time2 > debounce2) {
    Serial.println("1st");
    time2 = millis();
    previous2 = LOW;
    delay(100);
  }

  else if (reading2 == HIGH && previous2 == LOW) {
    Serial.println("2nd");
    curr2 = millis(); //2nd step이 시작하는 시간

    // 광조도센서가 주변 밝기 측정 시작
    int cdsValue = analogRead(cds);
    Serial.print("Cds = ");
    Serial.println(cdsValue);

    // 주변 밝기 측정값이 100 보다 크다면, LED ON
    if(cdsValue > 250)
      state = LOW;

    // 주변 밝기 측정값이 100 이하라면, LED OFF
    else 
     state = HIGH;

    if(curr2 - prev2 < 100) {
      digitalWrite(led, HIGH);
      prev2 = curr2; //prev2는 2nd step에서 조도센서가 led에 넘어갈 따까지 시간
    }
    else {
      digitalWrite(led, state);
    }
  }

  else if (reading2 == LOW && previous2 == LOW){ 
    Serial.println("3rd");
    state = HIGH;
    previous2 = HIGH;
    reading2 = HIGH;
    
    if(curr2 - prev2 < 100) {
      digitalWrite(led, HIGH);
      prev2 = curr2;
    }
    else {
      digitalWrite(led, state);
    }
    delay(100);
  } 

  else {
    state = HIGH;
  }
}