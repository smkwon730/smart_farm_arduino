/*
  서보 모터는 LED가 전류가 공급되어 있을 때 버튼을 눌러야 작동하는 점을 감안해서
  전류를 공급한 후, 버튼을 눌러주기 위하여 넣었음. 다른 LED를 사욯 한다면 필요가 없을 것으로
  생각됨.

  시간을 time 라이브러리를 사용하지 못하고, millis()로 했다는 점에서 시간 제한이 발생 할
  것이라고 생각함. 대략 47일 정도마다 아두이노를 리셋시켜줘야 한다는 단점이 있음. 이는
  time 라이브러리를 사용함으로써 해결할 수 있을 것으로 보임.
 */

#include <Servo.h> // 서보 라이브러리 사용
#include <Wire.h> // 아두이노와 LCD간의 통신을 위한 라이브러리 사용
#include <LiquidCrystal_I2C.h> // LCD 모니터 라이브러리 사용
LiquidCrystal_I2C lcd(0x27, 16, 2); // 사용할 LCD를 정의
Servo myservo; // 서보 모터를 정의

int first = false; // 서보 모터 제어 변수
int relay = 13; // 릴레이 핀이 13번에 연결되어 있음
int sensorPin = A0; // 가변 저항 핀이 A0에 연결되어 있음
int sensorValue = 0; // 가변 저항 값을 담아 둘 변수 선언
int buttonPin = 8; // 버튼 핀이 8번에 연결되어 있음
int buttonState = 0; // 버튼을 계속 누를 때 값의 빠른 전환을 막기 위한 변수
int buttont = false; // 버튼의 상태를 토글 상태로 설정하기 위한 변수
unsigned long t = 28800000; // 아두이노가 켜진 후 부터의 시간을 담기 위한 변수 (대략 47일?)
int stat = true; // 릴레이의 상태를 담아두기 위한 변수
int temp = false; // 서보 모터를 작동시키기 위한 변수
unsigned long tt = 0; // 서보 모터가 버튼을 누른 후 몇 초 후에 돌아오게 하기 위한 변수
int sensor = A1; // 토양 수분 센서의 핀이 A1에 연결되어 있음
int IN1 = 7; // 모터 드라이버 핀이 7에 연결되어 있음
int IN2 = 6; // 모터 드라이버 핀이 6에 연결되어 있음
unsigned long pt = 0; // 시간 간격을 두고 물을 분사하기 위한 변수
unsigned long ptt = 0; // 물을 특정 시간동안 분사하기 위한 변수
int pum = false; // 모터 드라이버가 작동하는지 확인하기 위한 변수
unsigned long ltime = 6; // LED를 작동시킬 시간을 설정하는 변수
int lser = true; // 릴레이가 항상 작동하는 경우와 반대의 경우를 설정하기 위한 변수
unsigned long hour = 3600000; // 밀리초를 시간으로 환산하는 변수
int wait = false; // 물을 간격에 나눠서 분사하는 경우 때 사용되는 변수
int che = false; // 물을 뿌리지 않고 대기하는 시간을 나타내는 변수
int count = 0; // 물을 나눠서 뿌릴 때 몇 번 뿌렸는지 감지하는 변수
unsigned long ppap = 86400000; // 호수를 뿌린 후 대기시간을 설정하기 위한 변수
unsigned long ppapt = 0; // 호수를 뿌린 후 대기시간을 감지하기 위한 변수

void setup() { // 무한 루프 문을 돌기 전 미리 설정하는 코드
  pinMode(relay, OUTPUT); // 릴레이 핀을 출력 모드로 사용
  pinMode(buttonPin, OUTPUT); // 버튼 핀을 출력 모드로 사용
  myservo.attach(9); // 서보 모터의 핀이 9번에 연결되어 있음을 정의
  
  lcd.begin(); // LCD 모니터 사용
  lcd.backlight(); // LCD 모니터 라이트 사용(어두우면 글자가 안보임, 즉 출력 X)
}

void loop() { // 무한 루프 문
  if (digitalRead(buttonPin) && millis()-buttonState >= 1000) { // 버튼을 누르고 있으며, 버튼이 작동하는 딜레이 시간이 지난 경우
    buttont = ~buttont; // 버튼의 상태를 변경
    buttonState = millis(); // 버튼 상태 변경 딜레이 설정
  }
  
  if ((analogRead(sensor) <= 650 && ppap >= hour/2) || ppap >= hour*24) 
  { // 토양 수분 센서 값이 계절에 따른 수치보다 적으며, 펌프 대기 시간이 지난 경우 (봄 : 500 여름 : 650 가을 : 400) 또는 물을 하루동안 주지 않은 경우
    ptt = millis(); // 뿌릴 시간 초기화
    pt = 0; // 대기 시간 초기화
    pum = true; // 펌프 상태를 작동으로 변경
    ppap = 0; // 호수를 뿌린 후 대기 시간을 0으로 설정
    ppapt = millis(); // 대기 시간 시작 시간을 현재 시간으로 설정
  } 
  ppap = millis()-ppapt; // 대기 시간 경과
  if (pum) { // 펌프 상태가 작동인 경우
    if (millis()-ptt <= 18000) { // 분사를 시작하고 18초가 지나지 않은 경우
      digitalWrite(IN1, HIGH); // 정방향 모터 작동
      digitalWrite(IN2, LOW); // 역방향 모터 비작동
    } else { // 분사를 시작하고 18초가 지난 경우
      digitalWrite(IN1, LOW); // 정방향 모터 비작동
      digitalWrite(IN2, LOW); // 역방향 모터 비작동
      pum = false; // 펌프 상태를 종료로 변경
      }
    }
  }
  if (wait) { // 나눠서 분사시키는 상태인 경우
    if (millis()-ptt <= 4500) { // 분사를 시작하고 4.5초가 지나지 않은 경우
        digitalWrite(IN1, HIGH); // 정방향 모터 작동
        digitalWrite(IN2, LOW); // 역방향 모터 비작동
    } else if (che != true) { // 분사를 시작하고 4.5초가 지났으며, 대기 상태가 아닌 경우
        digitalWrite(IN1, LOW); // 정방향 모터 비작동
        digitalWrite(IN2, LOW); // 역방향 모터 비작동
        count++; // 분사 횟수에 1 추가
        pt = millis(); // 대기 시간 설정
        che = true; // 대기 상태로 설정
        if (count >= 4) { // 만약 분사를 4번 이상 한 경우
          wait = false; // 분사 상태 종료
          che = false; // 대기 상태 종료
          pt = 0; // 대기 시간 초기화
        }
    }
    if (millis()-pt >= 120000 && che) { // 대기 상태이며, 2분이 지난 경우
      che = false; // 대기 상태 종료
      ptt = millis(); // 분사 시간 초기화
    }
  }
  if (map(analogRead(sensorPin), 0, 1023, 0, 25)>=24) { // 일조량이 24시간 이상인 경우
    if (sensorValue < 23) { // 저장된 센서 값이 23보다 작은 경우
      first = true; // 바뀐 상태 설정
      sensorValue = 24; // 가변저항의 값을 시간 단위 (0~24)로 환산하기 위한 코드 (25는 사실상 나오지 않으므로 24가 최대)
    }
  } else { // 일조량이 24시간이 아닌 경우
    sensorValue = map(analogRead(sensorPin), 0, 1023, 0, 25); // 서보모터의 값을 시간 단위 (0~24)로 환산하기 위한 코드 (25는 사실상 나오지 않으므로 24가 최대)
  }
  
  if (~stat) { // led가 작동하고 있지 않은 경우
    ltime = 24 - sensorValue; // 대기 시간을 24-작동시간으로 설정
  } else { // led가 작동하고 있는 경우
    ltime = sensorValue; // 대기 시간을 작동시간으로 설정
  }
  if (sensorValue == 0 || sensorValue >= 24) { // led 작동시간이 24시간 또는 0시간인 경우
    lser = false; // 릴레이 상태 변경 함수 사용 안함
  } else { // 아닌 경우
    lser = true; // 릴레이 상태 변경 함수 사용
  }
  if (millis()-t >= hour*ltime && lser) { // led의 상태가 변경된 후 부터의 시간이 대기 시간을 넘은 경우
    t = millis(); // 시간 초기화
    stat = ~stat; // led 상태 변경
    temp = stat; // 서보 모터 작동을 위한 변수 상태 변경
    // Serial.println(temp); 현재 상태를 확인하기 위한 프린트 함수 (실제로는 필요 없음)
  }
  
  if (temp || first) { // 릴레이가 꺼져있다가 작동하게 된 경우
    tt = millis(); // 서보 모터가 누르고 있을 시간을 재는 변수
    myservo.write(130); // 서보 모터 작동 (버튼 누르기)
    temp = false; // 변수 끄기 (해당 함수 종료)
    first = false; // 첫 서보 모터 제어 변수 중지
  }
  if (millis()-tt >= 500 && tt != 0) { // 서보 모터가 작동한 후, 0.5초가 지났으며, 서보 모터가 작동 상태인 경우
    myservo.write(60); // 서보 모터 원상복귀
    tt = 0; // 서보 모터 작동 X
  }
  if (tt == 0) { // 서보 모터가 작동하지 않는 경우
    myservo.write(myservo.read()); // 서보 모터 값을 현재 상태로 유지
  }
  if (sensorValue == 0) { // led 작동 시간이 0시간인 경우
    stat = false; // 릴레이의 상태를 작동안함으로 변경
  } else if (sensorValue >= 24) { // led 작동 시간이 24시간인 경우
    stat = true; // 릴레이의 상태를 작동함으로 변경
  } 
  if (stat) { // 릴레이의 상태가 작동함인 경우
    digitalWrite(relay, HIGH); // 릴레이 작동
  } else { // 릴레이의 상태가 작동안함인 경우
    digitalWrite(relay, LOW); // 릴레이 작동 안함
  }
  
  lcd.clear(); // LCD 모니터 초기화
  lcd.setCursor(0,0); // lcd 모니터의 커서를 0,0(왼쪽 위)로 설정
  lcd.print("Light hour : "); // 'Light hour : ' 출력
  lcd.print(sensorValue); // led 작동 시간 출력
  lcd.setCursor(0,1); // lcd 모니터의 커서를 0,1(왼쪽 아래)로 설정
  
  if (buttont) { // 버튼 상태가 작동 중인 경우
    lcd.print("Gradually"); // 'Gradually' 출력
  } else { // 버튼 상태가 작동 중이 아닌 경우
    lcd.print("Rapidly"); // 'Rapidly' 출력
  }
}
