#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>

#define uint8_t  unsigned char

#define MISC_DEV     "/dev/misc"
//#define MISC_DEV     "./misc.dump"

#define CMD_RUN_RECOVERY   "boot-recovery"

enum {
    SET_BOOTABLE = 1,
    SLOT_SWITCH,
    RUN_RECOVERY,
    RECOVERY_SLOT_A,
    DUMP_MISC
};

typedef struct BrilloSlotInfo {
    uint8_t bootable;
    uint8_t online;
    uint8_t reserved[2];
} BrilloSlotInfo;

typedef struct BrilloBootInfo {
    // Used by fs_mgr. Must be NUL terminated.
    char bootctrl_suffix[4];

    // Magic for identification - must be 'B', 'C', 'c' (short for
    // "boot_control copy" implementation).
    uint8_t magic[3];

    // Version of BrilloBootInfo struct, must be 0 or larger.
    uint8_t version;

    // Currently active slot.
    uint8_t active_slot;

    // Information about each slot.
    BrilloSlotInfo slot_info[2];
    uint8_t attemp_times;

    uint8_t reserved[14];
} BrilloBootInfo;

struct bootloader_message {
    char command[32];
    char status[32];
    char recovery[768];

    // The 'recovery' field used to be 1024 bytes.  It has only ever
    // been used to store the recovery command line, so 768 bytes
    // should be plenty.  We carve off the last 256 bytes to store the
    // stage string (for multistage packages) and possible future
    // expansion.
    char stage[32];
    BrilloBootInfo bootinfo;
    char reserved[192];
};

void Usage(void) {
    printf("./misc [1/2/3/4/5]\n");
    printf("1 -> set current slot bootable.\n");
    printf("2 -> upgrade finished, slot switch, a ->b or b -> a.\n");
    printf("3 -> slot a&b both unbootable, run recovery.\n");
    printf("4 -> recovery upgrade complete, run slot a.\n");
    printf("5 -> dump all misc info data.\n");
}

int ReadBootloaderMessage(int fd, struct bootloader_message *msg) {
    int ret = lseek(fd, 0, SEEK_SET);
    if (ret == -1) {
        printf("lseek failed\n");
        return -1;
    }

    ret = read(fd, msg, sizeof(struct bootloader_message));
    if (ret != sizeof(struct bootloader_message)) {
        printf("read misc failed\n");
        return -1;
    }

    return 0;
}

int WriteBootloaderMessage(int fd, struct bootloader_message *msg) {
    int ret = lseek(fd, 0, SEEK_SET);
    if (ret == -1) {
        printf("lseek failed\n");
        return -1;
    }

    ret = write(fd, msg, sizeof(struct bootloader_message));
    if (ret != sizeof(struct bootloader_message)) {
        printf("write misc failed\n");
        return -1;
    }

    return 0;
}

int SetSlotBootable(int fd) {
    int flag = 0;
    struct bootloader_message msg;
    memset(&msg, 0, sizeof(struct bootloader_message));
    int ret = ReadBootloaderMessage(fd, &msg);
    if (ret < 0) {
        return -1;
    }

    //set slot a bootable
    if (msg.bootinfo.active_slot == 0) {
        if (msg.bootinfo.slot_info[0].bootable == 0) {
            printf("set slot a bootable\n");
            flag = 1;
            msg.bootinfo.slot_info[0].bootable = 1;
        }
    } else {//set slot b bootable
        if (msg.bootinfo.slot_info[1].bootable == 0) {
            printf("set slot b bootable\n");
            flag = 1;
            msg.bootinfo.slot_info[1].bootable = 1;
        }
    }

    //need update misc
    if (flag == 1) {
        msg.bootinfo.attemp_times = 0;
        ret = WriteBootloaderMessage(fd, &msg);
        if (ret < 0) {
            return -1;
        }
    }

    return 0;
}

int SlotSwitch(int fd) {
    struct bootloader_message msg;
    memset(&msg, 0, sizeof(struct bootloader_message));
    int ret = ReadBootloaderMessage(fd, &msg);
    if (ret < 0) {
        return -1;
    }

    if (msg.bootinfo.active_slot == 0) {
        printf("slot a -> b\n");
        msg.bootinfo.active_slot = 1;
        msg.bootinfo.slot_info[1].bootable = 0;
        memcpy(msg.bootinfo.bootctrl_suffix,"_b",4);
    } else {
        printf("slot b -> a\n");
        msg.bootinfo.active_slot = 0;
        msg.bootinfo.slot_info[0].bootable = 0;
        memcpy(msg.bootinfo.bootctrl_suffix,"_a",4);
    }

    msg.bootinfo.attemp_times = 0;
    ret = WriteBootloaderMessage(fd, &msg);
    if (ret < 0) {
        return -1;
    }

    return 0;
}

int RunRecovery(int fd) {
    struct bootloader_message msg;
    memset(&msg, 0, sizeof(struct bootloader_message));
    int ret = ReadBootloaderMessage(fd, &msg);
    if (ret < 0) {
        return -1;
    }

    memcpy(msg.command, CMD_RUN_RECOVERY, sizeof(CMD_RUN_RECOVERY));
    ret = WriteBootloaderMessage(fd, &msg);
    if (ret < 0) {
        return -1;
    }

    return 0;
}

int RecoveryToSlota(int fd) {
    struct bootloader_message msg;
    memset(&msg, 0, sizeof(struct bootloader_message));
    int ret = ReadBootloaderMessage(fd, &msg);
    if (ret < 0) {
        return -1;
    }

    memset(msg.command, 0, sizeof(CMD_RUN_RECOVERY));
    msg.bootinfo.active_slot = 0;
    msg.bootinfo.slot_info[0].bootable = 0;
    msg.bootinfo.attemp_times = 0;

    ret = WriteBootloaderMessage(fd, &msg);
    if (ret < 0) {
        return -1;
    }

    return 0;
}

int DumpMiscInfo(int fd) {
    struct bootloader_message msg;
    int ret = ReadBootloaderMessage(fd, &msg);
    if (ret < 0) {
        return -1;
    }

    printf("command: %s\n", msg.command);
    printf("status: %s\n", msg.status);
    printf("recovery: %s\n", msg.recovery);
    printf("stage: %s\n", msg.stage);

    printf("bootctrl_suffix: %s\n", msg.bootinfo.bootctrl_suffix);
    printf("magic: %c%c%c\n", msg.bootinfo.magic[0], msg.bootinfo.magic[1], msg.bootinfo.magic[2]);
    printf("version: %d\n", msg.bootinfo.version);
    printf("active_slot: %d\n", msg.bootinfo.active_slot);

    printf("slot A:\n");
    printf("bootable: %d\n", msg.bootinfo.slot_info[0].bootable);
    printf("online: %d\n", msg.bootinfo.slot_info[0].online);
    printf("slot B:\n");
    printf("bootable: %d\n", msg.bootinfo.slot_info[1].bootable);
    printf("online: %d\n", msg.bootinfo.slot_info[1].online);

    printf("attemp_times: %d\n", msg.bootinfo.attemp_times);
    return 0;
}

int main(int argc, char **argv) {
#if 0
    if (argc != 2) {
        Usage();
        return 0;
    }
#endif
    int fd = open(MISC_DEV, O_RDWR);
    if (fd == -1) {
        printf("open %s failed!\n", MISC_DEV);
        return -1;
    }

#if 0 //for manually test and debug
    int choice = strtoul(argv[1], NULL, 10);
    printf("choice = %d\n", choice);

    switch (choice) {

        case SET_BOOTABLE:
            SetSlotBootable(fd);
            break;

        case SLOT_SWITCH:
            SlotSwitch(fd);
            break;

        case RUN_RECOVERY:
            RunRecovery(fd);
            break;

        case RECOVERY_SLOT_A:
            RecoveryToSlota(fd);
            break;

        case DUMP_MISC:
            DumpMiscInfo(fd);
            break;

        default:
            Usage();
            break;
    }
#else
    SetSlotBootable(fd);
#endif
    close(fd);
    return 0;
}
