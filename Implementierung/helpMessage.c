
#include <stdio.h>

void helpMessage() {
    printf("Help Message: Tricorn-Fraktal Generator\n\n");

    printf("Beschreibung:\n");
    printf("  Erstellt ein Tricorn-Fraktal anhand der gesetzten Flags. Dabei kann\n"
           "  zwischen verschiedenen Implementationen gewählt werden. Wenn ein\n"
           "  Flag fehlt, wird dieses mit Standardwerten vervollständigt.\n\n");

    printf("Verwendung:\n");
    printf("  ./main [-V <Versionnummer>] [-B <Anzahl Wiederholungen>]\n"
           "         [-s <Realteil>,<Imaginärteil>] [-d <Breite>,<Höhe>]\n"
           "         [-n <Maximale Anzahl Iterationen>] [-r <Schrittweite>]\n"
           "         [-o <Dateinamen>] [-C|--color] [-h|--help] [-t|--test]\n"
           "         [-graph] [-correct]\n\n");

    printf("Optionen:\n");
    printf("  Sollte eines der Flags --help, --test, -graph oder -correct\n"
           "  gesetzt sein, werden alle Eingaben davor und danach ignoriert.\n\n");

    printf("  -V <Versionnummer>           Versionnummer der Implementierung von 0 bis 5.\n"
           "                               (Standard: 0)\n\n");

    printf("  -B <Anzahl Wiederholungen>   Laufzeitmessung (Benchmarking) der angegebenen Implementierung.\n"
           "                               Sollte die Gesamtlaufzeit unter 1 Sekunde sein,\n"
           "                               wird eine Fehlermeldung mit ausgegeben.\n\n");

    printf("  -s <Realteil>,<Imaginärteil> Startwert der Berechnung in float.\n"
           "                               (Standard: 0,0)\n\n");

    printf("  -d <Breite>,<Höhe>           Anzahl der Pixel des Bildes. Die Breite muss positiv\n"
           "                               und ganzzahlig sein. Die Höhe kann auch negativ sein.\n"
           "                               (Standard: 4000,-4000)\n\n");

    printf("  -n <Maximale Iterationen>    Gibt die Anzahl an maximalen Iterationen pro\n"
           "                               Bildpixel aus. Muss ganzzahlig und positiv sein.\n"
           "                               (Standard: 256)\n\n");

    printf("  -r <Schrittweite pro Pixel>  Resolution gibt die Schrittweite pro Bildpixel an.\n"
           "                               Muss positiv und in float sein. Wenn nicht angegeben, \n"
           "                               wird vier geteilt durch die Bildbreite berechnet,\n"
           "                               sodass das Fraktal bei einem passenden Start komplett zu sehen ist.\n\n");

    printf("  -o <Dateinamen>              Gibt den Dateinamen der Ausgabedatei an.\n"
           "                               (Standardname: test)\n\n");

    printf("  -C | --color                 Wenn gesetzt, wird das Bild mit einer Farbpalette\n"
           "                               bunt gefärbt, sonst durch eine vordefinierte Formel\n"
           "                               in Grautöne eingefärbt. (Standard: Grauton)\n\n");

    printf("  -h | --help                  Gibt diese Help Message im Terminal aus.\n\n");

    printf("  -t | --test                  Es wird eine vordefinierte Testsuite ausgeführt\n"
           "                               und ausgegeben. Diese beinhaltet sämtliche\n"
           "                               Benchmarking-Szenarien mit Beschreibungen und gibt die\n"
           "                               Messungen in gut lesbarer Form aus.\n\n");

    printf("  -graph                       Es wird eine vordefinierte Testsuite ausgeführt,\n"
           "                               welche Werte in Tabellenform zum Erstellen\n"
           "                               von Graphen ausgibt.\n\n");

    printf("  -correct                     Flag zum Testen der Korrektheit aller\n"
           "                               Implementierungen gegenüber der Hauptimplementation.\n\n");


    printf("Beispiele:\n");

    printf("  Erstellung des Fraktals ausschließlich mit Standardwerten:\n"
           "         ./main\n\n");

    printf("  Erstellung des Fraktals mit den angegebenen Parametern:\n"
           "         ./main -s 2,-2 -d 800,800 -n 100 -r 0.005 -o test -V 4\n\n");

    printf("  Mit Benchmarking:\n"
           "         ./main -s 2,-2 -d 800,800 -n 100 -r 0.005 -o test -V 4 -B 100\n\n");

    printf("  Testsuite ausführen:\n"
           "         ./main --test\n");
}