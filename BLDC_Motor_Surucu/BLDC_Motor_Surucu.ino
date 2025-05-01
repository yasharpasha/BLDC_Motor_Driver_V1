// MOSFET pin tanımları
int u;
#define A_H 3   // Faz A üst MOSFET (PWM)
#define A_L 2   // Faz A alt MOSFET
#define B_H 9   // Faz B üst MOSFET (PWM)
#define B_L 8   // Faz B alt MOSFET
#define C_H 10  // Faz C üst MOSFET (PWM)
#define C_L 12  // Faz C alt MOSFET

int pwmDuty = 10;  // PWM duty değeri (maks 255)

void setup() {
  // MOSFET pinlerini çıkış olarak ayarla
  DDRD |= (1 << PD3);  // D3 - A_H (OC2B)
  DDRD |= (1 << PD2);  // D2 - A_L
  DDRB |= (1 << PB1);  // D9 - B_H (OC1A)
  DDRB |= (1 << PB0);  // D8 - B_L
  DDRB |= (1 << PB2);  // D10 - C_H (OC1B)
  DDRB |= (1 << PB4);  // D12 - C_L

  OCR1A = pwmDuty;  // B_H → D9
  OCR1B = pwmDuty;  // C_H → D10
  OCR2B = pwmDuty;  // A_H → D3

  TCCR1A = (1 << COM1A1) | (1 << COM1B1) | (1 << WGM10);
  TCCR1B = (1 << WGM12) | (1 << CS10) | (1 << CS11);

  // === TIMER2 AYARI (A_H için) ===
  // Fast PWM, non-inverting mode, prescaler 1
  TCCR2A = (1 << COM2B1) | (1 << WGM21) | (1 << WGM20);
  TCCR2B = (1 << CS22);

  digitalWrite(B_L, HIGH);
  digitalWrite(A_L, HIGH);
  digitalWrite(C_L, HIGH);
}

void loop() {
  // Sürekli aynı yönde 6 adımlı komütasyon döngüsü
  step1();
  delay(1000);
  step2();
  delay(1000);
  step3();
  delay(1000);
  step4();
  delay(1000);
  step5();
  delay(1000);
  step6();
  delay(1000);
}

// Tüm MOSFET'leri kapatma fonksiyonu
void disableAll() {
  PORTD |= (1 << PD2);
  OCR2B = 0;            //analogWrite(A_H, 0);
  PORTB |= (1 << PB0);  //digitalWrite(B_L, HIGH);
  OCR1A = 0;            //analogWrite(B_H, 0);
  PORTB |= (1 << PB4);  //digitalWrite(C_L, HIGH);
  OCR1B = 0;            //analogWrite(C_H, 0);
}

// Her bir komütasyon adımı:
void step1() {           // A+ B-
  OCR2B = pwmDuty;       //analogWrite(A_H, pwmDuty);
  PORTB |= (1 << PB0);   //digitalWrite(B_L, HIGH);
  PORTD |= (1 << PD2);   //digitalWrite(A_L, HIGH);
  PORTB &= ~(1 << PB4);  //digitalWrite(C_L, LOW);
  OCR1A = 0;             //analogWrite(B_H, 0);
  OCR1B = 0;             //analogWrite(C_H, 0);
}

void step2() {  // A+ C-
  analogWrite(A_H, pwmDuty);
  digitalWrite(C_L, HIGH);
  digitalWrite(A_L, HIGH);
  digitalWrite(B_L, LOW);
  analogWrite(B_H, 0);
  analogWrite(C_H, 0);
}

void step3() {  // B+ C-
  OCR1A = pwmDuty;
  PORTB |= (1 << PB4);
  PORTD &= ~(1 << PD2);
  PORTB |= (1 << PB0);

  OCR2B = 0;
  OCR1B = 0;
}

void step4() {  // B+ A-
  analogWrite(B_H, pwmDuty);
  digitalWrite(A_L, HIGH);
  digitalWrite(B_L, HIGH);
  digitalWrite(C_L, LOW);
  analogWrite(A_H, 0);
  analogWrite(C_H, 0);
}

void step5() {  // C+ A-
  analogWrite(C_H, pwmDuty);
  digitalWrite(A_L, HIGH);
  digitalWrite(B_L, LOW);
  digitalWrite(C_L, HIGH);
  analogWrite(A_H, 0);
  analogWrite(B_H, 0);
}

void step6() {  // C+ B-
  analogWrite(C_H, pwmDuty);
  digitalWrite(B_L, HIGH);
  digitalWrite(A_L, LOW);
  digitalWrite(C_L, HIGH);
  analogWrite(A_H, 0);
  analogWrite(B_H, 0);
}
/* On each step we know that the next 0 cross will be rising or falling and if it will be
   on coil A, B or C. With these functions we select that according to the step of the sequence */
void BEMF_A_RISING() {
  ADCSRA = (0 << ADEN);  // Disable the ADC module
  ADCSRB = (1 << ACME);  // MUX select for negative input of comparator
  ADMUX = 3;             // Changed: select A3 as comparator negative input (was A2)
  ACSR |= 0x03;          // Set interrupt on rising edge
}

void BEMF_A_FALLING() {
  ADCSRA = (0 << ADEN);  // Disable the ADC module
  ADCSRB = (1 << ACME);  // MUX select for negative input of comparator
  ADMUX = 3;             // Changed: select A3 as comparator negative input (was A2)
  ACSR &= ~0x01;         // Set interrupt on falling edge
}

void BEMF_B_RISING() {
  ADCSRA = (0 << ADEN);  // Disable the ADC module
  ADCSRB = (1 << ACME);  // MUX select for negative input of comparator
  ADMUX = 2;             // Changed: select A2 as comparator negative input (was A1)
  ACSR |= 0x03;          // Set interrupt on rising edge
}

void BEMF_B_FALLING() {
  ADCSRA = (0 << ADEN);  // Disable the ADC module
  ADCSRB = (1 << ACME);  // MUX select for negative input of comparator
  ADMUX = 2;             // Changed: select A2 as comparator negative input (was A1)
  ACSR &= ~0x01;         // Set interrupt on falling edge
}

void BEMF_C_RISING() {
  ADCSRA = (0 << ADEN);  // Disable the ADC module
  ADCSRB = (1 << ACME);  // MUX select for negative input of comparator
  ADMUX = 1;             // Changed: select A1 as comparator negative input (was A0)
  ACSR |= 0x03;          // Set interrupt on rising edge
}

void BEMF_C_FALLING() {
  ADCSRA = (0 << ADEN);  // Disable the ADC module
  ADCSRB = (1 << ACME);  // MUX select for negative input of comparator
  ADMUX = 1;             // Changed: select A1 as comparator negative input (was A0)
  ACSR &= ~0x01;         // Set interrupt on falling edge
}
