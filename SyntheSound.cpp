
#include <math.h>
#include <iostream>

#include "SyntheSound.hpp"

using namespace std;

float SyntheSound::deltaAngles[128];
unsigned int SyntheSound::pitchbend = 8192;
unsigned char SyntheSound::pitchbendsemitone = 2;

SyntheSound::SyntheSound(jack_midi_data_t note, jack_midi_data_t velocite)
{
  m_note = note;
  m_velocite = velocite;
  m_volumeChange = velocite;
  m_volume = 0;
  m_angle = 0;
  m_debutFin = false;
  m_sonFini = false;
}

SyntheSound::~SyntheSound()
{

}

const jack_midi_data_t& SyntheSound::getNote() const
{
  return m_note;
}

const jack_midi_data_t& SyntheSound::getVelocite() const
{
  return m_velocite;
}

void SyntheSound::setVelocite(const jack_midi_data_t& velocite)
{
  m_velocite = velocite;
  m_volumeChange = velocite;
}

void SyntheSound::setPitchBend(const unsigned int& pitch)
{
    SyntheSound::pitchbend = pitch;
}

/** @brief Teste si le son a terminé de jouer
  *
  * Teste si le son a terminé de jouer
  *
  * Renvoie un booléen
  *
  */
const bool& SyntheSound::estArrete()
{
  return m_sonFini;
}

/** @brief Demande l'arrêt du son
  *
  * Demande l'arrêt du son
  *
  * Le son sera arrêté plus tard quand sa sortie sera a 0.
  *
  */
void SyntheSound::demandeArret()
{
    // On commence par dire à la classe que c'est le début de la fin
    m_debutFin = true;
    // Ensuite on demande à la classe d'adapter le volume à une valeur de 0
    m_volumeChange = 0.0;
}

/** @brief getAngle
  *
  * Calcule et renvoie le nouvel angle
  *
  */
const float& SyntheSound::getAngle()
{
    // Récupération du delta de l'angle
    float deltaAngle = deltaAngles[m_note];
    // Application du pitch bend
    deltaAngle *= pow(2.0,((pitchbend - 8192.0)
                            * pitchbendsemitone
                            / 8192.0
                            / 12.0
                          )
                     );
    // On calcule le nouvel angle
    m_angle += deltaAngle;
    // Détection d'une valeur supérieure à 2PI
    m_angle = (m_angle >= 2 * M_PI)?m_angle - (2 * M_PI):m_angle;
    // Et on renvoie la nouvelle valeur
    return m_angle;
}

/** @brief getBaseSound
  *
  * @todo: document this function
  */
const float& SyntheSound::getBaseSound()
{
    // On calcule l'échantillon de base à condition que le son ne soit pas déjà fini
    if (!m_sonFini) m_baseSound = sin(getAngle());
    else m_baseSound = 0.0;
    // Il faut ajuster la valeur en fonction du volume,
    // mais avant, il faut vérifier que le son doit encore être joué.
    if ((m_debutFin) && (abs(m_baseSound) <= 0.000001))
    {
        // L'échantillon est suffisemment proche de 0,
        // on peut arrêter le son.
        m_sonFini = true;
    }
    else
    {
        if ((m_volumeChange != m_volume) && (abs(m_baseSound) <= 0.000001))
        {
            // Par contre, ici c'est le volume qui a changé
            m_volume = m_volumeChange;
        }
        m_baseSound = m_baseSound * m_volume / 127.0;
    }
    return m_baseSound;
}

void SyntheSound::initialise(const jack_nframes_t& sampleRate)
{
  int i = 0;
  for (; i <= 127; ++i)
  {
    jack_default_audio_sample_t frequence = 440 * pow(2.0,((i - 69.0) / 12.0));
    SyntheSound::deltaAngles[i] = 2 * M_PI * frequence / sampleRate;
  }
}
