#ifndef __MFRAVCTRL_H__
#define __MFRAVCTRL_H__

#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

/***
  * @brief      : Enable/Disable Video outputs(HDMI)
  * @param1[in] : bool; true - enable; false - disable.
  */
void enableVideoOutput(bool);

/***
  * @brief      : Enable/Disable Audio outputs(Mutes all ALSA Mute Controls)
  * @param1[in] : bool; true - enable; false - disable.
  */
void enableAudioOutput(bool);

#endif /* __MFRAVCTRL_H__ */
