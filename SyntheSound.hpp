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
    // Le tableau des différences d'angle en fonction de la note
  static float deltaAngles[];
  // La variation de la fréquence (pitch bend)
  static unsigned int pitchbend;
  // Le nombre de demi ton utilisé par le pitch bend
  static unsigned char pitchbendsemitone;

  // Variables
private:
  bool m_sonFini;
  bool m_debutFin;
  unsigned char m_note;
  unsigned char m_velocite;

  unsigned char m_volume;
  unsigned char m_volumeChange;

  float m_angle;
  float m_baseSound;

  // Constructeurs/Destructeur
public:
  SyntheSound(unsigned char note, unsigned char velocite = 100.0);
  ~SyntheSound();

  // Accesseurs
public:
  const unsigned char& getNote() const;
  const unsigned char& getVelocite() const;
  void setVelocite(const unsigned char& velocite);
  static void setPitchBend(const unsigned int& pitch);

  // Méthodes
  const float& getAngle();
  const float& getBaseSound();

  static void initialise(const jack_nframes_t& sampleRate);

  void demandeArret();
  const bool& estArrete();
};
