
// Décommenter cette ligne pour activer le mode debugging
//#define DEBUG

// Librairies
#include <SoftwareSerial.h>
#include <DFRobotDFPlayerMini.h>

// Pinout
const byte buttonCount = 10;
const byte buttonsPins[] = { 2, 3, 4, 5, 6, 7, 8, 9, 10, 11 };  // Liste des ports sur lesquels sont branchés les boutons
const byte MP3_PIN1 = A6;  // Le port de l'Ardunino branché sur le TX du DFPlayer
const byte MP3_PIN2 = A5;  // Le port de l'Ardunino branché sur le RX du DFPlayer

// Etat
int lastButtonPressedIndex = -1; // Numéro du dernier bouton appuyé
unsigned int buttonState[] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }; // Pour tracker le temps d'appui (debounce)
bool isADifferentButtonPressed = false;
bool isAButtonPressed = false;

// Constantes
const unsigned int pressedThreshold = 10000; // Seuil pour la routine de "debouncing".

// Lecteur
SoftwareSerial MP3Serial(MP3_PIN1, MP3_PIN2); // Emulation d'un port Série sur deux pins
DFRobotDFPlayerMini mp3Player;


/*
 * Initialisation
 */
void setup() {
  #ifdef DEBUG
  // Initialisation du port Série (pour debugging)
  Serial.begin(9600);
  #endif
  
  //Initialisation du lecteur MP3
  MP3Serial.begin(9600);
  mp3Player.begin(MP3Serial);
  mp3Player.volume(15);

  // Initialisation des ports des boutons en mode entrée avec pullup
  for (byte i = 0; i < buttonCount; i++) {
    pinMode(buttonsPins[i], INPUT_PULLUP);
  }
}

/*
 * Boucle principale
 */
void loop() {
  handleButtons();
}

/*
 * Fonction principale de gestion des boutons/lecture 
 */
void handleButtons() {
  for (byte i = 0; i < buttonCount; i++) {
    // Si un bouton est appuyé
    if (digitalRead(buttonsPins[i]) == LOW) {
      // Si le bouton est différent du bouton précedemment appuyé (ou aucun appuyé)
      if (lastButtonPressedIndex != i || lastButtonPressedIndex == -1) {
        // Si le bouton est appuyé depuis suffisamment longtemps, on valide réellement l'appui (filtrage des bounces)...
        if(buttonState[i] > pressedThreshold) {
          isADifferentButtonPressed = true;
          lastButtonPressedIndex = i;
          isAButtonPressed = true;
          
          #ifdef DEBUG
          Serial.print(i);
          Serial.println(" - PRESSED");
          #endif
        } else {
          // ...sinon on incrémente le tracker
          buttonState[i]++;
        }
      }
    } else {
      // Décrémentation du tracker si bouton pas appuyé
      if(buttonState[i] > 0) {
        buttonState[i]--;
      }
    }
  }

  // Si on a validé l'appui d'un bouton différent
  if (isADifferentButtonPressed) {
    // Si le lecteur jouait un son, on l'arrête
    if (-1 != mp3Player.readState()) {
      mp3Player.pause();
    }
    mp3Player.playMp3Folder(lastButtonPressedIndex);  // On joue le morçeau associé (entre 0 et 9)
    isADifferentButtonPressed = false;
  }

  // Si aucun bouton n'est actuellement appuyé
  for(byte i = 0; i < buttonCount; i++) {
    // On attends que le tracker de chaque bouton soit à zéro pour valider l'état, sinon on sort de la boucle
    if(buttonState[i] > 0) {
      break;
    }

    // Arrêt du player
    if (i == buttonCount - 1) {
      if(isAButtonPressed) {
        lastButtonPressedIndex = -1;
        isAButtonPressed = false;
        mp3Player.pause();
        
        #ifdef DEBUG
        Serial.println("NONE PRESSED");
        #endif
      }
    }
  }
}
