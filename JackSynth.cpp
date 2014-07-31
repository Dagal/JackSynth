// Pour le client Jack et l'audio
#include <jack/jack.h>
// Pour la partie Midi
#include <jack/midiport.h>

// iostream pour le cout
#include <iostream>
// unistd.h pour le usleep
#include <unistd.h>
// math.h pour sin
#include <math.h>
// list pour la liste de sons
#include <list>

#include "SyntheSound.hpp"

using namespace std;

jack_client_t* client;
jack_port_t* inPort;
jack_port_t* outPortLeft;
jack_port_t* outPortRight;
jack_default_audio_sample_t currentAngle;
jack_default_audio_sample_t tuneFrequence;
bool continuousSound;
unsigned char canal;
unsigned char volumeGeneral;
unsigned char pan;
jack_nframes_t sampleRate;

unsigned char maxSound;
list<SyntheSound*> syntheSounds;

int process(jack_nframes_t nframes, void* arg)
{
  // Récupération du buffer d'entrée midi
  void* inBuffer = jack_port_get_buffer(inPort, nframes);
  // Combien d'événement midi dans le buffer
  jack_nframes_t eventCount = jack_midi_get_event_count(inBuffer);
  // Position courante dans les événements
  jack_nframes_t eventIndex = 0;
  // L'événement en cours de traitement
  jack_midi_event_t inEvent;
  // Récupération des buffers de sortie
  jack_default_audio_sample_t* outBufferLeft =
    (jack_default_audio_sample_t*)jack_port_get_buffer(outPortLeft, nframes);
  jack_default_audio_sample_t* outBufferRight =
    (jack_default_audio_sample_t*)jack_port_get_buffer(outPortRight, nframes);
  // Variable de la boucle
  jack_nframes_t i;
  // Boucle de remplissage du buffer
  for (i = 0; i < nframes; ++i)
  {
    // Est ce qu'il y a encore des événements Midi?
    if (eventIndex < eventCount)
    {
      // On prend le prochain événement dans la liste d'attente.
      jack_midi_event_get(&inEvent, inBuffer, eventIndex);
      // On sépare la commande et le canal
      unsigned char commandeEvent = inEvent.buffer[0] & 0xf0;
      unsigned char canalEvent = 1 + (inEvent.buffer[0] & 0x0f);
      // Un petit switch vaut mieux qu'un long ifelseif
      switch (commandeEvent)
      {
      case 0x80:
        // NoteOff
        // On teste le canal ainsi que l'existance d'au moins un son en cours
        if (((canal == 0) ||
             (canal == canalEvent)) &&
            (!syntheSounds.empty()))
        {
          // On a pas le choix, on va devoir scanner les sons pour connaître leur note.
          cout << "On a un son à arrêter... Il faut bien le choisir!" << endl;
          for (list<SyntheSound*>::iterator it = syntheSounds.begin(); it != syntheSounds.end(); ++it)
          {
            // Test pour voir si la note est la même.
            if ((*it)->getNote() == inEvent.buffer[1])
            {
              cout << "On a trouvé une note à désactiver." << endl;
              // On lui demande de s'arrêter
              (*it)->demandeArret();
              // On sort directement de la boucle, notre travail est fait.
              break;
            }
          }
        }
        break;
      case 0x90:
        // NoteOn
        // On teste pour savoir si le canal est correcte
        if ((canal == 0) || (canalEvent == canal))
        {
          // La norme midi dit que la commande NoteOn est suivie de 2 octets
          // Le premier pour la note et le deuxième pour la vélocité
          cout << "La note " << (int)inEvent.buffer[1]
               << " est jouée sur le canal " << (int)canalEvent
               << " avec un volume de " << (int)inEvent.buffer[2] << endl;
          // Avant d'ajouter un son,
          // il faut savoir si on a pas atteint le nombre limite de son.
          if (syntheSounds.size() < maxSound)
          {
            // Il faut aussi passer tous les sons en revue pour savoir s'il existe déjà.
            if (!syntheSounds.empty())
            {
              // La liste est vide, on peut ajouter un son sans se soucier.
              syntheSounds.push_back(new SyntheSound(inEvent.buffer[1], inEvent.buffer[2]));
            }
            else
            {
              // Sinon il faut vérifier si ce son existe.
              bool noteExiste = false;
              // Regarde tous les sons ou presque.
              for (list<SyntheSound*>::iterator it = syntheSounds.begin(); it != syntheSounds.end(); ++it)
              {
                // Si la note est trouvée
                if ((*it)->getNote() == inEvent.buffer[1])
                {
                  // Alors on adapte la valeur du volume
                  noteExiste = true;
                  (*it)->setVelocite(inEvent.buffer[2]);
                  // et on quitte la boucle directement.
                  break;
                }
              }
              // Si le son n'est pas trouvé alors on le crée.
              if (!noteExiste) syntheSounds.push_back(new SyntheSound(inEvent.buffer[1], inEvent.buffer[2]));
            }
          }
        }
        break;
      case 0xb0:
        // Control Change
        // On teste pour savoir si le canal est correcte
        if ((canal == 0) || (canalEvent == canal))
        {
          // La norme midi dit que la commande Control Change est suivie de
          // deux octets.
          // Le premier pour le numéro du contrôleur et le second pour sa valeur
          unsigned char controller = inEvent.buffer[1];
          unsigned char valeur = inEvent.buffer[2];
          cout << "Control Change " << (int)controller;
          cout << " Value " << (int)valeur << endl;
          // Action en fonction du numéro de contrôleur.
          switch (controller)
          {
          case 07:
            // Gestion du volume général du canal du son
            volumeGeneral = valeur;
            break;
          case 10:
            // Gestion du pan.
            pan = valeur;
            break;
          default:
            cout << "Control Change non traité." << endl;
            break;
          }
        }
        break;
      default:
        cout << "Le contrôle " << (int)inEvent.buffer[1] << " n'est pas pris en charge." << endl;
        break;
      }
      // On oublie pas de pointer vers l'événement Midi suivant.
      eventIndex++;
    }

    if (!syntheSounds.empty())
    {
        list<SyntheSound*>::iterator it = syntheSounds.begin();
        while (it != syntheSounds.end())
        {
            if ((*it)->estArrete())
            {
                delete (*it);
                it = syntheSounds.erase(it);
            }
            else
            {
                ++it;
            }
        }
    }

    if (!syntheSounds.empty())
    {
      jack_default_audio_sample_t baseSound = 0;

      list<SyntheSound*>::iterator it = syntheSounds.begin();
      while (it != syntheSounds.end())
      {
        baseSound += (*it)->getBaseSound();
        ++it;
      }

      baseSound = baseSound * volumeGeneral / 127.0;
      // Mise à jour des buffers de sortie.
      outBufferLeft[i] = baseSound * ((pan>=64)?((127-pan)/63.0):1);
      outBufferRight[i] = baseSound * ((pan<=64)?(pan/64.0):1);
    }
    else
    {
      outBufferLeft[i] = outBufferRight[i] = 0.0;
    }
  }
  return 0;
}

void error(const char* description)
{
  cout << "Jack Error: " << description << endl;
}

int sampleRateChanged(jack_nframes_t nframes, void* arg)
{
  sampleRate = nframes;

  SyntheSound::initialise(sampleRate);

  cout << "Jack Server : Sample Rate changed to " << nframes << endl;
  return 0;
}

void jackShutdown(void* arg)
{
  // Désolé, je quitte ...
  cout << "Le serveur à lamentablement quitter les lieux sans prévenir." << endl;
  exit(0);
}

int main()
{
  // Jack Status utilisé seulement à la création du client
  jack_status_t jackStatus;
  // Position angulaire du son
  currentAngle = 0;
  // Fréquence du son de référence
  tuneFrequence = 440;
  // Le son est il soutenu en permanence?
  continuousSound = false;
  // Numéro du canal d'écoute où 0 pour tous les canaux
  canal = 1;
  // Initialisation du volume général du synthétiseur
  volumeGeneral = 100;
  // Initialisation du pan au centre.
  pan = 64;
  // Nombre de son maximum joué en même temps
  maxSound = 128;

  // Déclaration de la fonction de gestion des erreurs
  jack_set_error_function(error);
  // Création du client
  cout << "On commence par créer le client lui même." << endl;
  client = jack_client_open("MonClientJack",
                            JackServerName,
                            &jackStatus,
                            "default");
  cout << "Ensuite, il faut créer le port audio gauche de sortie." << endl;
  outPortLeft = jack_port_register(client,
                                   "SortieAudio1",
                                   JACK_DEFAULT_AUDIO_TYPE,
                                   JackPortIsOutput|
                                   JackPortIsTerminal,
                                   0);
  cout << "Ensuite, il faut créer le port audio droit de sortie." << endl;
  outPortRight = jack_port_register(client,
                                    "SortieAudio2",
                                    JACK_DEFAULT_AUDIO_TYPE,
                                    JackPortIsOutput|
                                    JackPortIsTerminal,
                                    0);
  cout << "Creation du port Midi d'entree." << endl;
  inPort = jack_port_register(client,
                              "EntreeMidi1",
                              JACK_DEFAULT_MIDI_TYPE,
                              JackPortIsInput|
                              JackPortIsTerminal,
                              0);
  // Récupération du nombre de sample par seconde pour la première fois
  // Après, c'est la fonction callback qui s'en occupe
  sampleRate = jack_get_sample_rate(client);

  SyntheSound::initialise(sampleRate);
  cout << "Initialisation de SynthSound" << endl;

  jack_set_sample_rate_callback(client, sampleRateChanged, 0);
  jack_on_shutdown(client, jackShutdown, 0);
  cout << "Référencement de la fonction process" << endl;
  jack_set_process_callback(client, process, 0);
  cout << "On active notre client fraichement créé." << endl;
  jack_activate(client);

  cout << "Maintenant on rentre dans une boucle infinie." << endl;
  while (true)
  {
    // On souffle un peu.
    usleep(1);
  }
}
