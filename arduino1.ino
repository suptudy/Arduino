#define A0  14
#define A1  15
#define A2  16
#define A3  17
#define A4  18
#define A5  19

#include <Keypad.h> //키패드
#include <SoftwareSerial.h> //소프트웨어 Serial 통신 라이브러리 사용

// 엘리베이터 키패드
const byte ROWS = 4;
const byte COLS = 4;
char keys[ROWS][COLS] = {
  {'1','2','3','A'},
  {'4','5','6','B'},
  {'7','8','9','C'},
  {'*','0','#','D'},
};
byte rowPins[ROWS] = {19,18,17,16};
byte colPins[COLS] = {15,14,4,5};
Keypad keypad = Keypad(makeKeymap(keys),rowPins,colPins,ROWS,COLS);
  
// 음성인식 모듈
SoftwareSerial mySerial(2,3); // Serial 통신핀으로 d2번핀 rx, d3핀 tx로 선언
// UART 통신
SoftwareSerial s(6, 17); //test 

byte n; 
int voice = 0; 
int now = 1; // 현재 엘리베이터 위치
int keyN; // 층 int화

// 공동현관에 연결
char door;

byte to7seg[9][7] = {    // 캐소드 방식... 애노드는 1/0 반대로
  //{1, 1, 1, 1, 1, 1, 0}, // 0
  {0, 1, 1, 0, 0, 0, 0}, // 1
  {1, 1, 0, 1, 1, 0, 1}, // 2    
  {1, 1, 1, 1, 0, 0, 1}, // 3  
  {0, 1, 1, 0, 0, 1, 1}, // 4
  {1, 0, 1, 1, 0, 1, 1}, // 5
  {1, 0, 1, 1, 1, 1, 1}, // 6
  {1, 1, 1, 0, 0, 0, 0}, // 7
  {1, 1, 1, 1, 1, 1, 1}, // 8
  {1, 1, 1, 1, 0, 1, 1}  // 9
};

void setup() {
  Serial.begin(9600); //통신 속도 9600bps로 pc와 시리얼 통신 시작
  mySerial.begin(9600); //음성인식 모듈 통신 속도 9600bps로 모듈과 시리얼 통신 시작
  
  //음성인식모듈 통신설정 
  Serial.println("wait settings are in progress");
  delay(1000);
  mySerial.write(0xAA); // compact mode 사용
  mySerial.write(0x37);
  delay(1000);
  mySerial.write(0xAA); // 그룹1 음성 명령어 imported
  mySerial.write(0x21);
  delay(1000);
  Serial.println("The settings are complete");

  // 엘리베이터 키패드
  pinMode(A0, INPUT);
  pinMode(A1, INPUT);
  pinMode(A2, INPUT);
  pinMode(A3, INPUT);
  pinMode(A4, INPUT);
  pinMode(A5, INPUT);

  // 7세그먼트 연결 핀을 출력용으로 설정  
  for (byte pinNum = 7; pinNum < 14; pinNum++){
    pinMode(pinNum, OUTPUT);          
  }
  
  // 엘리베이터 초기상태 1층 설정
  for (byte i = 0; i < 7; i++) 
    digitalWrite(i+7, to7seg[0][i]); 
  
  Serial.print("현재 엘리베이터의 위치는 ");
  Serial.print(now);
  Serial.println("층 입니다.");
}

void writeSegment(byte digit){ // 지정한 숫자를 7세그먼트에 출력
//  clearSegment();
  for (byte i = 0; i < 7; i++) 
    digitalWrite(i+7, to7seg[digit][i]);    
}

void loop() {
  // 공동현관관련
  if(Serial.available()) {
    // 공동현관 비밀번호 맞으면 아두이노2에서 신호 'a'
    if(s.available()) {
      door = s.read();
    }
    Serial.println("공동현관 비밀번호 인식");
    if(door == 'a') {  
      if(now > 1) { 
        // 엘리베이터가 다른 층에 있는 경우 (엘리베이터 1층으로)
        for (n = now; n >= keyN; n--) {
          writeSegment(n-1);  
          delay(1000);
        }
        now = n+1;
        
      } 
      // 엘리베이터가 1층에 있는 경우 

      delay(1000);
      Serial.print("도착 현재 ");
      Serial.println(now);
      // 엘리베이터 문 자동으로 열어줘야함 -- 신호 보냄
      digitalWrite(17, 1); //test
    }
  }

  // 엘리베이터 키패드
  char key = keypad.getKey();
  if(key) {
    if(key == 'A') {
      //test
      s.write('o'); //test
      Serial.println("UART 통신 test 열림");
      delay(1000);
      Serial.println("문이 열립니다.");
    } else if(key == 'B') {
      //test
      s.write('c'); //test
      Serial.println("UART 통신 test 닫힘");
      delay(1000);
      Serial.println("문이 닫힙니다.");
    } else {
      //Serial.print("현재 ");
      //Serial.print(key);
      //Serial.println("층 입니다");
      keyN = key - 48;
      if(keyN > now) {
          for (n = now; n <= keyN; n++) {
             writeSegment(n-1);  
             delay(1000);
          } 
          now = n-1;
        } else {
          for (n = now; n >= keyN; n--) {
            writeSegment(n-1);  
            delay(1000);
          }
          now = n+1;
        }
        Serial.print("도착 현재 ");
        Serial.println(now);
    }
    // 도착하면 자동으로 문 열어주는 신호
    
    delay(1000);
    Serial.println("문이 열립니다");
    delay(1000);
  }

  
  while(mySerial.available()) {
    Serial.println("voice input");
    voice = mySerial.read();
    //Serial.print("입력: ");
    //Serial.println(voice);
    
    switch(voice) {
      case 0x11:
        voice = voice - 16;
        Serial.println("1층");
        // 엘리베이터 문 닫히는 신호 -- 아두이노2에 신호  !!!!!!! 각 케이스마다 넣어줘야함
        s.write('c'); //test
        delay(1000);
        Serial.println("문이 닫힙니다");
        delay(1000);
        
        if(voice > now) {
           for (n = now; n <= 1; n++) {
            writeSegment(n-1);   
            delay(1000);
            now = n-1;
          } 
        } else {
          for (n = now; n >= 1; n--) {
            writeSegment(n-1);  
            delay(1000);
          }
          now = n+1;
        }
        Serial.print("도착 현재 ");
        Serial.println(now);
        break;
      case 0x12:
        voice = voice - 16;
        Serial.println("2층");
        // 엘리베이터 문 닫히는 신호 -- 아두이노2
        s.write('c'); //test
        
        if(voice > now) {
           for (n = now; n <= 2; n++) {
            writeSegment(n-1);   
            delay(1000);
          }
          now = n-1; 
        } else {
          for (n = now; n >= 2; n--) {
            writeSegment(n-1);   
            delay(1000);
          }
          now = n+1;
        }
        Serial.print("도착 현재 ");
        Serial.println(now);
        break;
      case 0x13:
        voice = voice - 16;
        Serial.println("3층");
        // 엘리베이터 문 닫히는 신호 -- 아두이노2
        s.write('c'); //test
        
        if(voice > now) {
           for (n = now; n <= 3; n++) {
            writeSegment(n-1);   
            delay(1000);
          } 
          now = n-1;
        } else {
          for (n = now; n >= 3; n--) {
            writeSegment(n-1);  
            delay(1000);
          }
          now = n+1;
        }
        Serial.print("도착 현재 ");
        Serial.println(now);
        break;
      case 0x14:
        voice = voice - 16;
        Serial.println("4층");
        // 엘리베이터 문 닫히는 신호 -- 아두이노2
        s.write('c'); //test
        
        if(voice > now) {
           for (n = now; n <= 4; n++) {
            writeSegment(n-1);   
            delay(1000);
          } 
          now = n-1;
        } else {
          for (n = now; n >= 4; n--) {
            writeSegment(n-1);  
            delay(1000);
          }
          now = n+1;
        }
        Serial.print("도착 현재 ");
        Serial.println(now);
        break;
      case 0x15:
        voice = voice - 16;
        Serial.println("5층");
        // 엘리베이터 문 닫히는 신호 -- 아두이노2
        s.write('c'); //test
        
        if(voice > now) {
           for (n = now; n <= 5; n++) {
            writeSegment(n-1);   
            delay(1000);
          } 
          now = n-1;
        } else {
          for (n = now; n >= 5; n--) {
            writeSegment(n-1);  
            delay(1000);
          }
          now = n+1;
        }
        Serial.print("도착 현재 ");
        Serial.println(now);
        break;  
    }
    // 도착하면 자동으로 문 열어주는 신호 -- 아두이노2에 신호 보냄
    s.write('o');
    delay(1000);
    Serial.println("문이 열립니다");
    delay(1000);
  }
  
  /*delay(1000); 
  for (byte n = 0; n < 9; n++) {       
    writeSegment(n);   // 1~9까지 숫자를 번갈아 출력 
    delay(1000);   
  }   */
}
