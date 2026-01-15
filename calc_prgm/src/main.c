typedef struct global global_t;
#define usb_callback_data_t global_t
#define fat_callback_usr_t msd_t

#include <graphx.h>
#include <keypadc.h>
#include <math.h>
#include <sys/util.h>
#include <ti/screen.h>

#include <usbdrvce.h>
#include <msddrvce.h>
#include <fatdrvce.h>
#include <tice.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define MAX_PARTITIONS 32
#define FAT_BUFFER_SIZE (FAT_BLOCK_SIZE / sizeof(uint16_t) * 38)

enum { USB_RETRY_INIT = USB_USER_ERROR };

struct global
{
    usb_device_t usb;
    msd_t msd;
};

static void putstr(char *str)
{
    os_PutStrFull(str);
    os_NewLine();
}

static usb_error_t handleUsbEvent(usb_event_t event, void *event_data,
                                  usb_callback_data_t *global)
{
    switch (event)
    {
        case USB_DEVICE_DISCONNECTED_EVENT:
            putstr("usb device disconnected");
            if (global->usb)
                msd_Close(&global->msd);
            global->usb = NULL;
            break;
        case USB_DEVICE_CONNECTED_EVENT:
            putstr("usb device connected");
            return usb_ResetDevice(event_data);
        case USB_DEVICE_ENABLED_EVENT:
            putstr("usb device enabled");
            global->usb = event_data;
            break;
        case USB_DEVICE_DISABLED_EVENT:
            putstr("usb device disabled");
            return USB_RETRY_INIT;
        default:
            break;
    }

    return USB_SUCCESS;
}

/*                  Useful Commands                  */
//gfx_RGBTo1555(r,g,b)
//gfx_SetColor(uint8_t col)
//gfx_ZeroScreen()
//gfx_SetPixel(uint24_t x, uint8_t y)
//
//GFX_LCD_WIDTH
//GFX_LCD_HEIGHT


//thanks to https://github.com/TheScienceElf/TI-84-CE-Raytracing/ because I no longer need a palette
volatile uint16_t* VRAM = (uint16_t*)0xD40000;

int main(void)
{
    static msd_partition_t partitions[MAX_PARTITIONS];
    static char buffer[212];
    static global_t global;
    static fat_t fat;
    uint8_t num_partitions;
    msd_info_t msdinfo;
    usb_error_t usberr;
    msd_error_t msderr;
    fat_error_t faterr;

    memset(&global, 0, sizeof(global_t));
    os_SetCursorPos(1, 0); 
    
    // usb initialization loop; waits for something to be plugged in
    do
    {
        global.usb = NULL;

        usberr = usb_Init(handleUsbEvent, &global, NULL, USB_DEFAULT_INIT_FLAGS);
        if (usberr != USB_SUCCESS)
        {
            putstr("usb init error.");
            goto usb_error;
        }

        while (usberr == USB_SUCCESS)
        {
            if (global.usb != NULL)
                break;

            // break out if a key is pressed
            if (os_GetCSC())
            {
                putstr("exiting demo, press a key");
                goto usb_error;
            }

            usberr = usb_WaitForInterrupt();
        }
    } while (usberr == USB_RETRY_INIT);
   
    if (usberr != USB_SUCCESS)
    {
        putstr("usb enable error.");
        goto usb_error;
    }

    // initialize the msd device
    msderr = msd_Open(&global.msd, global.usb);
    if (msderr != MSD_SUCCESS)
    {
        putstr("failed opening msd");
        goto usb_error;
    }

    putstr("opened msd");

    // get block count and size
    msderr = msd_Info(&global.msd, &msdinfo);
    if (msderr != MSD_SUCCESS)
    {
        putstr("error getting msd info");
        goto msd_error;
    }

    // print msd sector number and size
    sprintf(buffer, "block size: %u bytes", (uint24_t)msdinfo.bsize);
    putstr(buffer);
    sprintf(buffer, "num blocks: %u", (uint24_t)msdinfo.bnum);
    putstr(buffer);

    // locate the first fat partition available
    num_partitions = msd_FindPartitions(&global.msd, partitions, MAX_PARTITIONS);
    if (num_partitions < 1)
    {
        putstr("no paritions found");
        goto msd_error;
    }

    // attempt to open the first found fat partition
    // it is not required to use a MSD to access a FAT filesystem if the
    // appropriate callbacks are configured.
    for (uint8_t p = 0;;)
    {
        uint32_t base_lba = partitions[p].first_lba;
        fat_callback_usr_t *usr = &global.msd;
        fat_read_callback_t read = &msd_Read;
        fat_write_callback_t write = &msd_Write;

        faterr = fat_Open(&fat, read, write, usr, base_lba);
        if (faterr == FAT_SUCCESS)
        {
            sprintf(buffer, "opened fat partition %u", p - 1);
            putstr(buffer);
            break;
        }

        p++;
        if (p >= num_partitions)
        {
            putstr("no suitable patitions");
            goto msd_error;
        }
    }

    //clear screeen
    os_ClrHome();
	
    fat_dir_t dir;
    fat_dir_entry_t entry;
    if(FAT_SUCCESS != fat_OpenDir(&fat, "/", &dir))
	    goto fat_error;

    putstr("Del > next | Mode > select");

    bool selected = false;
    while(!selected){
	fat_ReadDir(&dir, &entry);
	putstr(entry.name);
	
	if(entry.name[0] == 0)
		goto fat_error;
    
	while(kb_Data[1] == 0){
		kb_Scan();
	}

	if(kb_Data[1] == kb_Mode) {selected = true;}
	kb_Scan();

	while(kb_Data[1] != 0){
		kb_Scan();
	}
    }
    
    sprintf(buffer, "/%s", entry.name);

    static fat_file_t file;
    static uint16_t fat_buffer[FAT_BUFFER_SIZE];

    faterr = fat_OpenFile(&fat, buffer, 0, &file);
    if (faterr != FAT_SUCCESS)
    {
	putstr("could not open file");
        goto fat_error;
    }

    os_ClrHome();
    gfx_Begin();

    uint32_t frame = 0;
    uint32_t sec = 0;
    uint32_t min = 0;
    uint32_t hr = 0;

    gfx_SetTextFGColor(0);
    gfx_SetTextBGColor(255);
    gfx_SetTextTransparentColor(1);
    gfx_SetColor(255);

    /* if 2nd not pressed */
    while (!(kb_Data[1] == kb_2nd)){
	sprintf(buffer, "%lu:%lu:%lu     ", hr, min, sec);
	gfx_PrintStringXY(buffer, 170, 10);

	frame++;

	if(frame % 8 == 7){
		sec++;
		gfx_FillRectangle_NoClip(1, 120, 318, 190);
		if(sec > 59) {sec -= 60; min++;}
		if(min > 59) {min -= 60; hr++;}
	}
	
	if(true/*fat_buffer[256 * 37 + 128] != 0*/){
		for(int i = 0; i < 5; i++){
			//memset(buffer, 32, 60);
			//gfx_PrintStringXY(buffer, 3, 125 + 10 * i);
			memcpy(buffer, &fat_buffer[(256 * 37 + 128) + 20 * i], 40);
			gfx_PrintStringXY(buffer, 3, 125 + 10 * i);
		}
	}


	clock_t start, end;
	float total;

	start = clock();

	fat_ReadFile(&file, 38, fat_buffer);
	for(int i = 0; i < 120; i++){
		memcpy(&VRAM[i * 160], &fat_buffer[i * 80], 160);
	}

	kb_Scan();

	end = clock();
	total = (float)(end - start) / CLOCKS_PER_SEC;

	int to_sleep = (float)(0.125 - total) * 1000;
	if(to_sleep > 0)
		msleep(to_sleep);
	
	if(kb_Data[1] == kb_Mode) {
		while(kb_Data[1] == kb_Mode){
 			sec+=1;
			msleep(10);
			if(sec > 59) {sec -= 60; min++;}
			if(min > 59){min -= 60; hr++;}
			sprintf(buffer, "%lu:%lu:%lu     ", hr, min, sec);
			gfx_PrintStringXY(buffer, 170, 10);
			kb_Scan();
		}
		fat_SetFileBlockOffset(&file, 38 * ((sec * 8) + (min * 8 * 60) + (hr * 3600 * 8)));
		frame = (sec * 8) + (min * 8 * 60) + (hr * 3600 * 8);
	}
    }

    // close the fat file
    faterr = fat_CloseFile(&file);
    if (faterr != FAT_SUCCESS)
    {
	putstr("file close error");
        goto fat_error;
    }

fat_error:
    // release the filesystem
    fat_Close(&fat);

msd_error:
    // close the msd device
    msd_Close(&global.msd);

usb_error:
    // cleanup usb
    usb_Cleanup();

    while (!os_GetCSC());
    
    gfx_End();

    return 0;
}
