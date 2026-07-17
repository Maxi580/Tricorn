# Projektbericht



## Einleitung














## Optimierungen 

### Naive (tricorn_V1)


### SIMD (tricorn_V2)

### Symmetry (Nicht genutzt)

Trikorn-Fraktale sind an der X-Achse symmetrisch, weswegen man nur die Hälfte des Bilds berechnen muss.
Um dies auszunutzen, berechnen wir das Zentrum des Bildes, also die y-Koordinate an dem der Imaginärteil 0 ist.
So können wir für jede zu zeichnende Zeile, die potenzielle Partner-Zeile finden (y_gespiegelt = 2 * center - y) 
und berechnen, ob der Imaginärteil der beiden Kandidaten exakt das inverse ist. Da die Imaginärteile Floats sind und ein Vergleich deswegen
nie zu 100% korrekt ist, wurde diese Optimierung in der finalen Version weggelassen. 

### Circle (tricorn_V3)

### Latency Throughput (tricorn_V4)

Laut den Intel-Intrinsics ist die Latency von z.B. _mm_add_ps_ 8 mal so hoch wie der Throughput. Dadurch, dass
wir in Escape Time eine Dependency Chain haben, da eine Instruktion fast immer das Ergebnis der vorherigen braucht ist 
das sehr ineffizient. Deswegen berechnen wir gleich 2 mal 4er lanes (also insgesamt 8 Punkte gleichzeitig). Da wir so
z.B. zwei add Operationen hintereinander machen können, wobei die zweite add operation nicht von der ersten abhängt, 
also nicht darauf gewartet werden muss bis das Ergebnis im Speicher ist. Aufgrund limitierter Anzahl von xmm Registern 
haben wir es auf bei 2x belassen.

### MutliThreading (Hauptimplementierung)


## Benchmarking-Methodik (Zeitmessung)

  

## Testumgebung (Korrektheit)







