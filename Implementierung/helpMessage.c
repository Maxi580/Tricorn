
#include <stdio.h>
#include <stdlib.h>

void helpMessage() {
    printf("Help Message: Tricorn-Fraktal Generator\n\n");

    printf("Beschreibung:\n");
    printf("  Erstellt ein Tricorn-Fraktal anhand der gesetzten Flags. Dabei kann\n"
           "  zwischen verschiedenen Implementationen gewählt werden. Wenn ein\n"
           "  Flag fehlt, wird dieses mit Standardwerten vervollständigt.\n\n");

    printf("Verwendung:\n");
    printf("  ./main [-V <Versionnummer>] [-B <Anzahl Wiederholungen>]\n"
           "         [-s <Realteil> <Imaginärteil>] [-d <Breite> <Höhe>]\n"
           "         [-n <Maximale Anzahl Iterationen>] [-r Schrittweite]\n"
           "         [-o <Dateinamen>] [-C|--color] [-h|--help] [-t|--test]\n"
           "         [-graph] [-correct]\n\n");

    printf("Optionen:\n");
    printf("  Sollte eines der Flags -help, -test, -graph und -correct\n"
           "  gesetzt sein, werden alle Eingaben davor und danach ignoriert und\n"
           "  das zuerst gefundene Flag verwendet.\n\n");

    printf("  -V <Versionnummer>           Versionnummer der Implementierung von 0 bis 5.\n"
           "                               (Standard: 0)\n\n");
    printf("  -B <Anzahl Wiederholungen>   Laufzeitmessung der angegebenen Implementierung.\n"
           "                               Sollte die Laufzeit der Iterationen unter 1 Sekunde sein,\n"
           "                               bricht der Test ab.\n\n");
    printf("  -s <Realteil> <Imaginärteil> Startwert der Berechnung in float.\n"
           "                               (Standard: 0 + 0i)\n\n");
    printf("  -d <Breite> <Höhe>           Anzahl der Pixel des Bildes. Die Breite muss positiv\n"
           "                               und ganzzahlig sein. Die Höhe kann auch negativ sein.\n"
           "                               (Standard: 4000, -4000)\n\n");
    printf("  -n <Maximale Iterationen>    Gibt die Anzahl an maximalen Iterationen pro\n"
           "                               Bildpixel aus. Muss ganzzahlig und positiv sein.\n"
           "                               (Standard: 256)\n\n");
    printf("  -r <Schrittweite pro Pixel>  Resolution gibt die Schrittweite pro Bildpixel an.\n"
           "                               Muss positiv und ein float sein. Wenn nicht angegeben, \n"
           "                               wird 4 geteilt durch die Bildbreite berechnet,\n"
           "                               sodass das Bild beim passenden Start komplett zu sehen ist.\n\n");
    printf("  -o <Dateinamen>              Gibt den Dateinamen der Ausgabedatei an.\n"
           "                               Wenn die Endung nicht .bmp ist, wird diese vom Programm\n"
           "                               hinzugefügt. (Standardname: test.bmp)\n\n");
    printf("  -C | --color                 Wenn gesetzt, wird das Bild mit einer Farbpalette\n"
           "                               bunt gefärbt, sonst durch eine vordefinierte Formel\n"
           "                               in Grautöne eingefärbt. (Standard: Grauton)\n\n");
    printf("  -h | --help                  Zeigt dir diese Help Message an.\n\n");
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
    printf("  Nur die Standardwerte:\n"
           "         ./main\n\n");
    printf("  Normales Fraktal berechnen:\n"
           "         ./main -s 2 -2 -d 800 800 -n 100 -r 0.005 -o test -V 4\n\n");
    printf("  Mit Benchmarking:\n"
           "         ./main -s 2 -2 -d 800 800 -n 100 -r 0.005 -o test -V 4 -B 100\n\n");
    printf("  Testsuite ausführen:\n"
           "         ./main --test\n");

    exit(EXIT_SUCCESS);
}


