// J'ai besoin d'inclure Jack au minimum pour ses types de variables
#include <jack/jack.h>
#include <jack/midiport.h>

class SyntheSound
{
  // Enumération
  enum NomNote
    {
      Do,Dod,Re,Red,Mi,Fa,Fad,Sol,Sold,La,Lad,Si,
      Reb = 1, Mib = 3, Solb = 6, Lab = 8, Sib = 10,
      C = 0,Cd,D,Dd,E,F,Fd,G,Gd,A,Ad,B,
      Db = 1, Eb = 3, Gb = 6, Ab = 8, Bb = 10
    };
  
  // Variables statiques
public:
  static jack_default_audio_sample_t deltaAngles[];
  
  // Variables
private:
  jack_midi_data_t m_note;
  jack_midi_data_t m_velocite;

  jack_midi_data_t m_volume;
  
  jack_default_audio_sample_t m_angle;
  
  // Constructeurs/Destructeur
public:
  SyntheSound(jack_midi_data_t note, jack_midi_data_t velocite);
  ~SyntheSound();
  
  // Accesseurs
  jack_midi_data_t note() const;
  jack_midi_data_t velocite() const;
  void setVelocite(jack_midi_data_t velocite);
  jack_midi_data_t volume() const;
  void setVolume(jack_midi_data_t volume);
  
  // Méthodes
  jack_default_audio_sample_t getAngle();
  static void initialise(jack_nframes_t sampleRate);
};
