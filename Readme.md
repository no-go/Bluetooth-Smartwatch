# DIY Smartwatch - Plan B

Diese Version ist ein total anderes Konzept. Trinket Pro und OLED Display
wachen auf, sobald der PIN3 (Interrupt 1) auf GND geht. Eine Tilde wird
dann an das Handy gesendet, was dann alle gesammelten Nachrichten (Buffer
Größe ist über App Einstellungen anpassbar) der Smartwatch zusendet. Zu
aller erst wird aber die Zeit bzw. Datum (auch über App-Einstellungen
anpassbar) zugesandt.

## Details

Die Nachrichten scrollen durch die Uhr und können daher etwas länger als 120
Zeichen sein. Eine analoge Uhr ist als Darstellung weggefallen. Beim drücken
des Buttons wird ein kleines Aufwachen Symbol gezeigt.

Durch Delays kann es passieren, dass die Smartwatch schläft, obwohl
gerade eine Nachricht eingeht. In den Punkt weißt Plan B noch schwachstellen
auf. Durch den wirklich massiven Schlummerbetrieb wird auf der Smartwatch
nicht mehr angezeigt, wenn auf den Handy eine neue Nachricht eingeht. Das
ist nicht ganz so schlimmt, da man eh auf den Knopf drückt, um die Uhrzeit
zu sehen. Da reicht es aus, wenn man mit der Uhrzeit auch die neuen Nachrichten
sehen kann.

## Features

- Die Batterie Anzeige ist wieder da (durch die 3,3 V aber weiterhin nur 
  für die letzten 30 Min interessant)
- enorm Energiesparend
- nicht nur 1 Nachricht
- Auto-Scrollen
- App Einstellungen für Zeichenpuffer und Datumsformatierung

## Power

- über 50 mA beim Starten
- 5.5 mA im Schlummerbetrieb
- 25 mA beim Aufwachen und Scrollen

Mit einem 170mAh Akku komme ich doch auf 30h (PWR_DOWN)! Mit IDLE Modus
wacht Smartwatch bei UART auf, braucht aber dann 7mA was ca 21h Akkulaufzeit
wäre. 

## Schaltung

Im Code ist gut zu erkennen, an welchen Pins was angebracht ist. Wie gesagt,
der Pin 3 wird via Taster auf GND getastet. TX am Trinket Pro geht übrigends
an RXT des UART Bluefruit Moduls.

## Hülle

Ein Kombination aus Brillenputztuch Schlauch (mit Patex geklebt und damit auch
das rechteckige Loch fürs Display verstärkt) und ein Uhrenlederband haben sich
als wiederverwendbare Zwischenlösung bewährt. Man muss dann nicht so viel
Silikon abfummeln und neu dran machen, falls mal was spinnt.

## Analoge Uhr

Ist die Uhrzeit zu anfang mit einem # Symbol und HH:mm formatiert, zeigt die
Smartwatch nun wieder eine Analoge Uhr an. Außerdem steht 12,3,6 und 9
auf der Uhr in sehr kleinen Zahlen.
