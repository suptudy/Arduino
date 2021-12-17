#include <Keypad.h>
#include <SoftwareSerial.h> //소프트웨어 Serial 통신 라이브러리 사용

SoftwareSerial s(9,8);


//          3   //C4
//          4   //C3
#define A0 14   //C2
#define A1 15   //C1
#define A2 16   //R4
#define A3 17   //R3
#define A4 18   //R2
#define A5 19   //R1

//엘리베이터 DC모터
int ENA = 5;    
int IN1 = 6;
int IN2 = 7;

//공동현관문 DC모터
int ENB = 11;
int IN3 = 12;
int IN4 = 13;

char secretCode[5] = {'1','2','3','4','*'};  // 비밀번호를 설정(여기선 1234)
int tr = 0;   // 비밀번호 비교 확인
int count = 0;
/*
char data;  //아두이노1에서 받는 신호(엘리베이터)
*/

// 엘리베이터 키패드
const byte ROWS = 4;
const byte COLS = 4;
char keys[ROWS][COLS] = {
  {'1','2','3','A'},
  {'4','5','6','B'},
  {'7','8','9','C'},
  {'*','0','#','D'},
};
byte rowPins[ROWS] = {19,18,17,16}; //R1, R2, R3, R4
byte colPins[COLS] = {15,14,4,3};   //C1, C2, C3, C4

Keypad keypad = Keypad(makeKeymap(keys),rowPins,colPins,ROWS,COLS);

void setup() {
  Serial.begin(9600);
  s.begin(9600);
  

  //엘리베이터 문
  pinMode(ENA, OUTPUT); //ENA
  pinMode(IN1, OUTPUT); //IN1
  pinMode(IN2, OUTPUT); //IN2

  //공동현관문
  pinMode(ENB, OUTPUT); //ENB
  pinMode(IN3, OUTPUT); //IN3
  pinMode(IN4, OUTPUT); //IN4

  Serial.println("setup complete");
}

/*엘리베이터 여는 함수*/
void open_ev(){
  analogWrite(ENA, 45);
  digitalWrite(IN1,HIGH);
  digitalWrite(IN2,LOW);
  delay(2000);

  analogWrite(ENA, LOW);
  digitalWrite(IN1,HIGH);
  digitalWrite(IN2,HIGH);
  delay(2000);
    
  Serial.println("엘리베이터 열림");
}

/*엘리베이터 닫는 함수*/
void close_ev() {
  analogWrite(ENA, 45);
  digitalWrite(IN1,LOW);
  digitalWrite(IN2,HIGH);
  delay(2000);
    
  digitalWrite(ENA,LOW);
  digitalWrite(IN1,HIGH);
  digitalWrite(IN2,HIGH);
  delay(2000);  
  
  Serial.println("엘리베이터 닫힘");
}

/*공동현관문 여닫는 함수*/
void door_ud(){    
  //정방향  문열림
  analogWrite(ENB, 30); //ENA
  digitalWrite(IN3, HIGH);
  digitalWrite(IN4, LOW);
  delay(2000);
          
  //문고정
  digitalWrite(ENB, LOW);
  digitalWrite(IN3, HIGH);
  digitalWrite(IN4, HIGH);
  delay(2000);
  
  Serial.println("공동현관 열림");

  // 역방향 회전 문닫힘
  analogWrite(ENB, 30); //ENB
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, HIGH);
  delay(2000);
          
  //문고정
  digitalWrite(ENB, LOW);
  digitalWrite(IN3, HIGH);
  digitalWrite(IN4, HIGH);
  delay(2000);
  
  Serial.println("공동현관 닫힘");
}

void loop() {
  //엘리베이터
  if(s.available()) {
    /*data = s.read();*/      //아두이노1과 통신(엘리베이터) 
    Serial.println("문확인");
    Serial.println(s.read()); // 아두이노1에서 넘어오는지 확인 !!!!!!!!!!!!!!!!!!!!!
    if(s.read() == 'o') {   //열림
       open_ev();
    }else if(s.read() == 'c') {   //닫힘
      close_ev();
    }
  }
 
  

  char key = keypad.getKey(); // 키패드에서 입력된 값을 가져옵니다.
  
  //공동현관문
  if((key >= '0' && key <= '9') || (key >= 'A' && key <='D') || (key == '*' || key == '#')){
      
    
    Serial.println(key);
    if(key == secretCode[count]) {        //입력번호와 비밀번호가 맞을시 
       count++;
       tr++;
    }
    else if(key != secretCode[count]) {   //입력번호와 비밀번호가 불일치
       count++;
    }
    
    if(key == '#') {                      //누른 번호 초기화
       count =0;
       tr=0;
       Serial.println("초기화");
    }
    if(count ==5) {                       //누른 번호가 5개인 경우
       if(tr == 5) {                     //일치 번호가 5개인 경우
         Serial.println("잠금 해제");        
         
         s.write('a');
         
         //정방향  문열림
         door_ud();
       }
       else {                            //아닌 경우
         Serial.println("실패");
       }
       count = 0;
       tr=0;
    }   
  }
}
