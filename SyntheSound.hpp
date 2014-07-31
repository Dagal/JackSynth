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
  bool m_sonFini;
  bool m_debutFin;
  jack_midi_data_t m_note;
  jack_midi_data_t m_velocite;

  jack_midi_data_t m_volume;
  jack_midi_data_t m_volumeChange;

  jack_default_audio_sample_t m_angle;
  jack_default_audio_sample_t m_baseSound;

  // Constructeurs/Destructeur
public:
  SyntheSound(jack_midi_data_t note, jack_midi_data_t velocite = 100.0);
  ~SyntheSound();

  // Accesseurs
public:
  const jack_midi_data_t& getNote() const;
  const jack_midi_data_t& getVelocite() const;
  void setVelocite(const jack_midi_data_t& velocite);

  // Méthodes
  const jack_default_audio_sample_t& getAngle();
  const jack_default_audio_sample_t& getBaseSound();

  static void initialise(const jack_nframes_t& sampleRate);

  void demandeArret();
  const bool& estArrete();
};
