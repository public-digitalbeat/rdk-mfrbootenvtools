#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <tinyalsa/asoundlib.h>

/***
  * @brief      : Puts all ALSA based Sound cards to Mute/UnMute
  * @param[in]  : bool; true - mutes; false - unmutes.
  */
bool setAllSoundCardsToMute(bool mute)
{
    bool status = false;
    int card = 0;
    unsigned int i, j;
    unsigned int num_ctls, ctrlNumValues;
    const char *name, *typeName;
    enum mixer_ctl_type typeEnum;
    struct mixer *mixer;
    struct mixer_ctl *ctl;

    mixer = mixer_open(card);
    if (mixer) {
        /* Get list of controls */
        num_ctls = mixer_get_num_ctls(mixer);
        for (i = 0; i < num_ctls; i++) {
            ctl = mixer_get_ctl(mixer, i);
            if (ctl) {
                name = mixer_ctl_get_name(ctl);
                typeName = mixer_ctl_get_type_string(ctl);
                typeEnum = mixer_ctl_get_type(ctl);
                ctrlNumValues = mixer_ctl_get_num_values(ctl);
                for (j = 0; j < ctrlNumValues; j++) {
                    switch (typeEnum) {
                        /* Focuz only the Mute-UnMute or Enable-Disable control types */
                        case MIXER_CTL_TYPE_BOOL:
                            if (strstr(name, "mute")) {
                                /* We are interested in controllong Mute options. */
                                printf("%u\t%s\t%u\t%-40s\n", i, typeName, ctrlNumValues, name);
                                printf("mixer_ctl_get_value = '%s' && mute = %d\n",
                                        (mixer_ctl_get_value(ctl, j) ? "On" : "Off"), mixer_ctl_get_value(ctl, i), mute);
                                if (!strncmp((mixer_ctl_get_value(ctl, j) ? "On" : "Off"), "Off", strlen("Off")) && mute) {
                                    if (mixer_ctl_set_value(ctl, j, 1)) {
                                        printf("Unable to set value '0' on ctrl index %d/%d\n", i, j);
                                    }
                                } else if (!strncmp((mixer_ctl_get_value(ctl, j) ? "On" : "Off"), "On", strlen("On")) && !mute) {
                                    if (mixer_ctl_set_value(ctl, j, 0)) {
                                        printf("Unable to set value '0' on ctrl index %d/%d\n", i, j);
                                    }
                                }
                            } else {
                                /* Nothing to do here. */
                            }
                            break;
                        default:
                            break;
                    }
                }
            } else {
                printf("ctl is NULL for index %d\n", i);
            }
        }
        mixer_close(mixer);
        status = true;
    }
    return status;
}

char *strnstr(const char *s, const char *find, size_t slen)
{
    char c, sc;
    size_t len;

    if ((c = *find++) != '\0') {
        len = strlen(find);
        do {
            do {
                if (slen-- < 1 || (sc = *s++) == '\0')
                    return (NULL);
            } while (sc != c);
            if (len > slen)
                return (NULL);
        } while (strncmp(s, find, len) != 0);
        s--;
    }
    return ((char *)s);
}

/***
 * @brief      : Enable/Disable HDMI Hotplug Detection GPIO.
 * @param[in]  : bool; true - enable; false - disable.
 */
void enableHDMIHPD(bool enable)
{
    const char *sysEntryOfHDMIResumeFile = "/sys/kernel/debug/aml_reg/paddr";
    int fd = open(sysEntryOfHDMIResumeFile, O_RDWR, S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH);
    if (fd != -1) {
        bool doWrite = false;
        size_t i[3] = {};
        char buffer[32] = {'\0'};
        char cregData[16] = {'\0'};
        char *pData = NULL;
        uint32_t regData = 0;

        i[0] = write(fd, "ff634464", strlen("ff634464"));
        i[1] = read(fd, buffer, sizeof(buffer));
        buffer[i[1]] = '\0';
        if ((pData = strnstr(buffer, "=", sizeof(buffer)))) {
            pData += 2;
            buffer[strcspn(buffer, "\r\n")] = '\0';
            strncpy(cregData, pData, strlen(pData));
            printf("cregData = '%s'\n", cregData);
        }
        regData = (uint32_t)strtoul(cregData, NULL, 16);
        printf("regData = 0x%x\n", regData);
        if (!((regData >> 8) & 1) && enable) {
            regData |= (1 << 8);
            sprintf(buffer, "ff634464 %x", regData);
            doWrite = true;
        } else if (((regData >> 8) & 1) && !enable) {
            regData &= ~(1 << 8);
            sprintf(buffer, "ff634464 %x", regData);
            doWrite = true;
        }
        if (doWrite) {
            printf("Writing '%s'\n", buffer);
            /* Work-around : Trigger HDMI HPD GPIO */
            i[2] = write(fd, buffer, strlen(buffer));
            printf("Write success : %d %d %d\n", i[0], i[1], i[2]);
        }
        close(fd);
    } else {
        printf("open failed : %d\n", fd);
    }
}

/***
 * @brief      : Mute/Un-mute HDMI Video Output
 * @param[in]  : bool; true - mute; false - un-mute.
 */
void muteHDMIVideoOutput(bool mute)
{
    const char *sysEntryOfHDMIVideoFile = "/sys/class/amhdmitx/amhdmitx0/vid_mute";
    int fd = open(sysEntryOfHDMIVideoFile, O_RDWR, S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH);
    if (fd != -1) {
        size_t bytesWritten = 0;
        if (mute) {
            /* Disable */
            bytesWritten = write(fd, "1", strlen("1"));
        } else {
            /* Enable */
            bytesWritten = write(fd, "0", strlen("0"));
        }
        close(fd);
        printf("bytesWritten = %d\n", bytesWritten);
    } else {
        printf("open %s failed\n", sysEntryOfHDMIVideoFile);
    }
}

/***
 * @brief      : Enable/Disable Video outputs(HDMI)
 * @param1[in] : bool; true - enable; false - disable.
 */
void enableVideoOutput(bool enable)
{
    muteHDMIVideoOutput(!enable);
}

/***
 * @brief      : Enable/Disable Audio outputs(All ALSA Audio Controls)
 * @param1[in] : bool; true - enable; false - disable.
 */
void enableAudioOutput(bool enable)
{
    setAllSoundCardsToMute(!enable);
}

