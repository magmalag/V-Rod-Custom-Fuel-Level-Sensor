#include <avr/pgmspace.h>

// ==================== РЕЖИМ НАСТРОЙКИ ПРИБОРКИ ====================
bool CALIBRATION_MODE = false;  // true - ручной подбор ШИМ, false - работа от датчика
int  TEST_PWM = 183;            // МЕНЯЙ ЭТО ЧИСЛО, чтобы двигать стрелку по рискам

// ==================== НАСТРОЙКА РИСОК ПРИБОРКИ ====================
const int G_POINTS = 9;        
const int gPWM[] = {
  183, // ШИМ для риски 0 (Пусто)
  189, // ШИМ для риски 1/8
  196, // ШИМ для риски 1/4
  203, // ШИМ для риски 3/8
  210, // ШИМ для риски 1/2 (Половина)
  216, // ШИМ для риски 5/8
  222, // ШИМ для риски 3/4
  227, // ШИМ для риски 7/8
  232  // ШИМ для риски 1/1 (Полный)
};

// Литры, соответствующие рискам на приборке
const float gLiters[] = {0, 2.5, 5.0, 7.5, 10.0, 12.5, 15.0, 17.5, 20.5}; 

// ==================== ПИНЫ И ЖЕЛЕЗО ====================
const int sensorPin = A1;  
const int batteryPin = A4; 
const int outputPin = 10;   

const int PWM_OFFSET = -32;  
const int SYS_REF = 455;     
const float COMP_FACTOR = 0.40; 
const int USE_VOLTAGE_COMP = 1;
const float FUEL_FILTER_K = 0.08; 

// ==================== ТВОЯ ИДЕАЛЬНАЯ ТАБЛИЦА (С ЛИСТОЧКА) ====================
// Всего 33 точки. Я перенес каждую цифру с твоей фотографии!
const int NUM_POINTS = 33;  

const int rawTable[] PROGMEM = { 
  222, 226, 237, 242, 248, 253, 260, 268, 272, 280, 
  287, 295, 305, 330, 341, 352, 365, 376, 390, 420, 
  437, 454, 496, 520, 545, 605, 642, 681, 726, 777, 
  837, 987, 1023 
};

const float litersTable[] PROGMEM = { 
  0.0,  2.0,  3.4,  4.0,  5.0,  5.7,  6.2,  7.0,  7.5,  8.3, 
  9.0,  9.4,  10.0, 11.0, 11.3, 11.7, 12.0, 12.2, 12.5, 13.2, 
  13.6, 13.8, 14.0, 14.4, 15.0, 16.1, 17.0, 17.4, 18.0, 18.7, 
  19.3, 20.1, 20.5 
};

float filteredInput = 0;

// ==================== ФУНКЦИИ ====================

// Медианный фильтр (отсекает резкие всплески от плескания бензина)
int getMedianRead(int pin) {
  int samples[5];
  for (int i = 0; i < 5; i++) samples[i] = analogRead(pin);
  for (int i = 0; i < 4; i++) {
    for (int j = i + 1; j < 5; j++) {
      if (samples[i] > samples[j]) {
        int temp = samples[i]; samples[i] = samples[j]; samples[j] = temp;
      }
    }
  }
  return samples[2]; 
}

// Преобразование "попугаев" АЦП в литры по твоей таблице
float getLitersFromRaw(int raw) {
  if (raw <= pgm_read_word(&rawTable[0])) return pgm_read_float(&litersTable[0]);
  if (raw >= pgm_read_word(&rawTable[NUM_POINTS-1])) return pgm_read_float(&litersTable[NUM_POINTS-1]);
  for (int i = 0; i < NUM_POINTS-1; i++) {
    int r1 = pgm_read_word(&rawTable[i]);
    int r2 = pgm_read_word(&rawTable[i+1]);
    if (raw >= r1 && raw <= r2) {
      float l1 = pgm_read_float(&litersTable[i]);
      float l2 = pgm_read_float(&litersTable[i+1]);
      return l1 + (float)(raw - r1) / (r2 - r1) * (l2 - l1);
    }
  }
  return pgm_read_float(&litersTable[NUM_POINTS-1]);
}

// Преобразование литров в ШИМ для приборки
int getPwmForLiters(float liters) {
  if (liters <= gLiters[0]) return gPWM[0];
  if (liters >= gLiters[G_POINTS-1]) return gPWM[G_POINTS-1];
  for (int i = 0; i < G_POINTS-1; i++) {
    if (liters >= gLiters[i] && liters <= gLiters[i+1]) {
      float ratio = (liters - gLiters[i]) / (gLiters[i+1] - gLiters[i]);
      return gPWM[i] + (int)(ratio * (gPWM[i+1] - gPWM[i]));
    }
  }
  return gPWM[G_POINTS-1];
}

// ==================== ИНИЦИАЛИЗАЦИЯ ====================
void setup() {
  Serial.begin(115200);
  pinMode(outputPin, OUTPUT);
  
  // При включении зажигания мгновенно считываем уровень топлива,
  // чтобы стрелка сразу встала куда нужно, а не ползла медленно с нуля.
  filteredInput = getMedianRead(sensorPin); 
}

// ==================== ОСНОВНОЙ ЦИКЛ ====================
void loop() {
  // 1. Мгновенный вольтметр
  int currentBattery = getMedianRead(batteryPin);

  // 2. Чтение датчика топлива (чистые данные)
  int rawInput = getMedianRead(sensorPin);
  
  // Плавный фильтр: стрелка не будет дергаться в поворотах
  filteredInput += (float)(rawInput - filteredInput) * FUEL_FILTER_K;   
  float currentLiters = getLitersFromRaw((int)filteredInput);

  // 3. Выбор режима ШИМ
  int currentPwm;
  if (CALIBRATION_MODE) {
    currentPwm = TEST_PWM; // Ручной подбор ШИМ (если true)
  } else {
    currentPwm = getPwmForLiters(currentLiters); // Автоматический расчет по таблице
  }

  // 4. Компенсация вольтажа бортсети (чтобы приборка не врала при просадках напряжения)
  if (USE_VOLTAGE_COMP == 1) {
    float sysDelta = (float)currentBattery - (float)SYS_REF; 
    currentPwm = currentPwm + (int)(sysDelta * COMP_FACTOR); 
  }

  // Применяем офсет (калибровку самой приборки) и выдаем на ножку
  int finalOutput = constrain(currentPwm + PWM_OFFSET, 120, 254);
  analogWrite(outputPin, finalOutput);

  // 5. Вывод в монитор порта (каждые 200мс)
  static uint32_t timer = 0;
  if (millis() - timer > 200) {
    timer = millis();
    Serial.print(F("Raw_Sensor:")); Serial.print(rawInput);
    Serial.print(F(" | Liters:")); Serial.print(currentLiters, 1);
    
    if (CALIBRATION_MODE) {
      Serial.print(F(" | TEST_PWM:")); Serial.print(TEST_PWM);
    } else {
      Serial.print(F(" | BASE_PWM:")); Serial.print(currentPwm);
    }
    
    Serial.print(F(" | BatRaw:")); Serial.print(currentBattery);
    Serial.print(F(" | FINAL_PWM:")); Serial.println(finalOutput);
  }
  delay(10); 
}
