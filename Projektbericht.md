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

Man kann in das Trikorn_Fraktal einen Kreis mit Radius 0,25 zeichnen, in dem garantiert kein Punkt konvergiert.
(Skript zur Findung des Radius ist unter Circle/findCircleRad.c zu finden)

### Latency Throughput (tricorn_V4)

Laut den Intel-Intrinsics ist die Latency von z.B. _mm_add_ps achtmal so hoch wie der Throughput. 
In Escape Time haben wir eine Dependency Chain, da fast jede Instruktion das Ergebnis der vorherigen benötigt. 
Das ist ineffizient, weil dann immer darauf gewartet werden muss, bis das Ergebnis im Register zur Verfügung steht, 
obwohl die CPU in dieser Zeit rechnen könnte. Deshalb berechnen wir zwei 4er-Lanes gleichzeitig, also insgesamt 8 Punkte.
So können wir bspw. zwei add-Operationen hintereinander ausführen, ohne dass die zweite von dem Ergebnis der ersten
abhängig ist und warten muss. Es werden nur zwei 4er-Lanes genutzt, weil es begrenzte xmm-Register gibt.

### MutliThreading (Hauptimplementierung)

Wir multithreaden entlang der y-Achse mit so vielen Threads, wie das System logische Prozessoren bereitstellt. 
Nur bei sehr kleinen Bildern nutzen wir einen einzigen Thread, da sich der Thread-Overhead sonst nicht lohnt. 
Weil die meiste Rechenlast in der Bildmitte liegt, teilen wir die Threads gleichmäßig über das gesamte Bild auf.

## Benchmarking-Methodik (Zeitmessung)

  

## Testumgebung (Korrektheit)







