// MOSFET pin tanımları
#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>

// test kodlar
#define A_H 3         // Faz A üst MOSFET (PWM)
#define A_L 2         // Faz A alt MOSFET
#define B_H 9         // Faz B üst MOSFET (PWM)
#define B_L 8         // Faz B alt MOSFET
#define C_H 10        // Faz C üst MOSFET (PWM)
#define C_L 12        // Faz C alt MOSFET
#define BEMF_A 1      // A1 pin
#define BEMF_B 2      // A2 pin
#define BEMF_C 3      // A3 pin
int pwmDuty = 20;     // PWM duty değeri (maks 255)
int currentStep = 1;  // Mevcut komütasyon adımı

void setup() {
  // MOSFET pinlerini çıkış olarak ayarla
  DDRD |= (1 << PD3);  // D3 - A_H (OC2B)
  DDRD |= (1 << PD2);  // D2 - A_L
  DDRB |= (1 << PB1);  // D9 - B_H (OC1A)
  DDRB |= (1 << PB0);  // D8 - B_L
  DDRB |= (1 << PB2);  // D10 - C_H (OC1B)
  DDRB |= (1 << PB4);  // D12 - C_L


  TCCR1A = (1 << COM1A1) | (1 << COM1B1) | (1 << WGM10);
  TCCR1B = (1 << WGM12) | (1 << CS10) | (1 << CS11);

  // === TIMER2 AYARI (A_H için) ===
  // Fast PWM, non-inverting mode, prescaler 1
  TCCR2A = (1 << COM2B1) | (1 << WGM21) | (1 << WGM20);
  TCCR2B = (1 << CS22);

  digitalWrite(B_L, HIGH);
  digitalWrite(A_L, HIGH);
  digitalWrite(C_L, HIGH);
  ADCSRA = (0 << ADEN);  // ADC'yi devre dışı bırak
  ADCSRB = (1 << ACME);  // Comparator'ın negatif girişi için MUX seçimi
  BEMF_A_RISING();
  step1();

  // Global interrupt'leri etkinleştir
  //sei();
}

void loop() {
  // Sürekli aynı yönde 6 adımlı komütasyon döngüsü
  step1();
  if(analogRead(A1)>10){
    step2();
    
  }
  delayMicroseconds(1);
  step2();
  delayMicroseconds(1);
  step3();
  delayMicroseconds(1);
  step4();
  delayMicroseconds(1);
  step5();
  delayMicroseconds(1);
  step6();
  delayMicroseconds(1);
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
  BEMF_A_RISING();
}

void step2() {           // A+ C-
  OCR2B = pwmDuty;       //analogWrite(A_H, pwmDuty);
  PORTB |= (1 << PB4);   //digitalWrite(C_L, HIGH);
  PORTD |= (1 << PD2);   //digitalWrite(A_L, HIGH);
  PORTB &= ~(1 << PB0);  //digitalWrite(B_L, LOW);
  OCR1A = 0;             //analogWrite(B_H, 0);
  OCR1B = 0;             //analogWrite(C_H, 0);
  
}

void step3() {  // B+ C-
  OCR1A = pwmDuty;
  PORTB |= (1 << PB4);
  PORTD &= ~(1 << PD2);
  PORTB |= (1 << PB0);
  OCR2B = 0;
  OCR1B = 0;
  
}

void step4() {           // B+ A-
  OCR1A = pwmDuty;       //analogWrite(B_H, pwmDuty);
  PORTD |= (1 << PD2);   //digitalWrite(A_L, HIGH);
  PORTB |= (1 << PB0);   //digitalWrite(B_L, HIGH);
  PORTB &= ~(1 << PB4);  //digitalWrite(C_L, LOW);
  OCR2B = 0;             //analogWrite(A_H, 0);
  OCR1B = 0;             //analogWrite(C_H, 0);
  
}

void step5() {           // C+ A-
  OCR1B = pwmDuty;       //analogWrite(C_H, pwmDuty);
  PORTD |= (1 << PD2);   //digitalWrite(A_L, HIGH);
  PORTB &= ~(1 << PB0);  //digitalWrite(B_L, LOW);
  PORTB |= (1 << PB4);   //digitalWrite(C_L, HIGH);
  OCR2B = 0;             //analogWrite(A_H, 0);
  OCR1A = 0;             //analogWrite(B_H, 0);
  
}

void step6() {           // C+ B-
  OCR1B = pwmDuty;       //analogWrite(C_H, pwmDuty);
  PORTB |= (1 << PB0);   //digitalWrite(B_L, HIGH);
  PORTD &= ~(1 << PD2);  //digitalWrite(A_L, LOW);
  PORTB |= (1 << PB4);   //digitalWrite(C_L, HIGH);
  OCR2B = 0;             //analogWrite(A_H, 0);
  OCR1A = 0;             //analogWrite(B_H, 0);
  
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


//notes
/*
ISR(ANALOG_COMP_vect) {
  int lastStep = 0;

  // Yeni bir sıfır geçiş tespit edildiğinde
  if (currentStep != lastStep) {
    lastStep = currentStep;

    // Mevcut adımda göre sonraki adımı seç
    switch (currentStep) {
      case 1: currentStep = 2; break;
      case 2: currentStep = 3; break;
      case 3: currentStep = 4; break;
      case 4: currentStep = 5; break;
      case 5: currentStep = 6; break;
      case 6: currentStep = 1; break;
    }

    // Yeni adıma geçiş yap
    switch (currentStep) {
      case 1: step1(); break;
      case 2: step2(); break;
      case 3: step3(); break;
      case 4: step4(); break;
      case 5: step5(); break;
      case 6: step6(); break;
    }
  }
}
*/