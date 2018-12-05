#include <EnableInterrupt.h>
#include <kEncoder.h>
using namespace kEncoder;

RelativeEncoder rel_encoder;

void rel_encoder_isr(){
  rel_encoder.interputHandler();
}

void setup() {
  rel_encoder.setPins(A12, A13);
  rel_encoder.setPort(&PORTK, 0b00110000);
  rel_encoder.setup(&rel_encoder_isr);
}

void loop() {
  Serial.print("Steps "); Serial.print(rel_encoder.steps);
  Serial.print(" & Rel Direction "); Serial.print(rel_encoder.direction);
  Serial.println("");
}
