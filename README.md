# Anteckningar 2022-02-01
Implementering av diverse aritmetiska och logiska instruktioner samt villkorliga hoppinstruktioner för egenskapad CPU.
Även implementering av instruktioner CLR, LSL samt LSR.

I efterföljande del ska interrupt-implementering läggas till.

Samtliga .c- och .h-filer utgörs av processorn med färdig ALU samt att programkoden har uppdaterats för att testa ALU:n via bitvis AND samt bitvis OR.

Filen "alu.png" utgör en bild som demonstrerar ALU:ns arbetssätt visuellt samt via text.

Filen "alu_emulator.zip" utgör en ALU-emulator, som kan användas för att testa ALU:ns funktion från en terminal. Operation samt operander kan matas in från terminalen, följt av att resultatet skrivs ut både decimalt och binärt, tillsammans med statusbitar SNZVC. Fem exempelfall skrivas ut vid start, som demonstrerar när de olika statusbitarna ettställs.

Filen "led_blink.asm" demonstrerar den assemblerkod som placeras i programminnet för att testa ALU:n, skrivet i AVR assembler.