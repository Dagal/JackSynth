#include <math.h>

#include "SyntheSound.hpp"

jack_default_audio_sample_t SyntheSound::deltaAngles[128];

SyntheSound::SyntheSound(jack_midi_data_t note, jack_midi_data_t velocite)
{
	m_note = note;
	m_velocite = velocite;
	m_angle = 0;
}

SyntheSound::~SyntheSound()
{

}

jack_midi_data_t SyntheSound::note() const
{
	return m_note;
}

jack_midi_data_t SyntheSound::velocite() const
{
	return m_velocite;
}

void SyntheSound::setVelocite(jack_midi_data_t velocite)
{
  m_velocite = velocite;
}

jack_default_audio_sample_t SyntheSound::getAngle()
{
	jack_default_audio_sample_t angle = m_angle;
	m_angle += deltaAngles[m_note];
	if (m_angle >= 2 * 3.1415926353) m_angle -= 2 * 3.1415926353;
	return angle;
}

void SyntheSound::initialise(jack_nframes_t sampleRate)
{
	int i = 0;
	for (;i <= 127;++i)
	{
		jack_default_audio_sample_t frequence = 440 * pow(2.0,((i - 69.0) / 12.0));
		SyntheSound::deltaAngles[i] = 2 * 3.1415926353 * frequence / sampleRate;
	}
}
