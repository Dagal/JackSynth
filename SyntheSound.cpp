
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
    //m_volumeChange = 0.0;
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
    m_angle = (m_angle >= 2 * M_PI)?(m_angle - (2 * M_PI)):m_angle;
    // Et on renvoie la nouvelle valeur
    return m_angle;
}

/** @brief getBaseSound
  *
  * @todo: document this function
  */
const float& SyntheSound::getBaseSound()
{
    // Si le son n'est pas terminé
    if (!m_sonFini)
    {
        // alors on calcule la valeur de base de l'échantillon
        m_baseSound = sin(getAngle());

        // Si la fin du son est demandée
        if (m_debutFin)
        {
            // alors on tente d'arrêter le son en douceur
            // Si le volume est à 0
            if (m_volume == 0)
            {
                // alors on peut arrêter le son complètement
                m_sonFini = true;
            }
            else
            {
                // sinon on demande pour mettre le volume à 0 avant l'arrêt définitif
                m_volumeChange = 0; //m_volume / 2;
            }
            cout << "Ça sent la fin avec un volume de " << (unsigned int)m_volume << endl;
        }
        // Ensuite on vérifie le volume et on calcule l'échantillon final

        // Si le volume à changé et que l'échantillon est suffisemment
        // proche de la valeur de 0 alors
        if ((m_volumeChange != m_volume) && (m_baseSound <= 0.01) && (m_baseSound >= -0.01))
        {
            // on peut modifier le volume.
            m_volume = m_volumeChange;
        }
        m_baseSound = m_baseSound * m_volume / 127.0;
    }
    else
    {
        m_baseSound = 0.0;
        cout << "Le son est terminé avec un volume de " << (int)m_volume << endl;
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
