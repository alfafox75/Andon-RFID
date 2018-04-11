void relectrl() {
  if (state_id == '1') {
    digitalWrite(ledVerde, HIGH);
    digitalWrite(ledGiallo, LOW);
    digitalWrite(ledRosso, LOW);
    digitalWrite(rl1, HIGH);
    digitalWrite(rl2, LOW);
    digitalWrite(rl3, LOW);
    digitalWrite(rl4, LOW);
  }
  if (state_id == '2') {
    digitalWrite(ledVerde, LOW);
    digitalWrite(ledGiallo, HIGH);
    digitalWrite(ledRosso, LOW);
    digitalWrite(rl1, LOW);
    digitalWrite(rl2, HIGH);
    digitalWrite(rl3, LOW);
    digitalWrite(rl4, LOW);
  }
  if (state_id == '3') {
    digitalWrite(ledVerde, LOW);
    digitalWrite(ledGiallo, LOW);
    digitalWrite(ledRosso, HIGH);
    digitalWrite(rl1, LOW);
    digitalWrite(rl2, LOW);
    digitalWrite(rl3, HIGH);
    digitalWrite(rl4, LOW);
  }
}

