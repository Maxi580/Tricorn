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

### Latency Throughput (tricorn_V3)

### Latency Throughput (tricorn_V4)

### MutliThreading (Hauptimplementierung)


## Benchmarking-Methodik (Zeitmessung)

  

## Testumgebung (Korrektheit)







