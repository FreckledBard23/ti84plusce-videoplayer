# MP4 TI 84+ CE Video Player
A simple way to get any video onto a calculator.

## Prerequisites:
- Jailbroken TI 84+ CE
- [clibs.8xg](https://github.com/CE-Programming/libraries/releases/tag/v14.1) on the calculator
- USB A to USB B OTG Cable
- USB Drive (not micro sd to usb) formatted to FAT32
- A method of getting files to your calculator
- Python

## Setup:
### Calculator Side:
1. Compile ./calc_prgm with the [CE C/C++ Toolchain](https://ce-programming.github.io/toolchain/)
2. Put ./calc_prgm/bin/VIDEO.8xp onto the calculator
### Python Side:
1. Verify that python is installed on your computer
2. Install opencv, pillow, and pytubefix
   ~~~
   pip install opencv-python
   pip install Pillow
   pip install pytubefix
   ~~~

## Usage:
1. Find Youtube video and copy link (optional, any mp4 and srt files can be used)
   1. Run get_youtube_video.py and paste the link
   2. Choose a name for the output files {name}_video.mp4, {name}_subtitles.srt
   3. The program should inform you of what subtitle language was picked
2. Compile video to .bin
   1. Run convert_bin.py
   2. Choose .mp4, .srt (optional), and output .bin (output bin name can be no longer than 12 letters, including file extension)
   3. Wait for the program to finish (Warning: Very inefficient, will use large amounts of RAM on long videos)
   4. Put .bin on the USB Drive
3. Run your .bin (Backup RAM in case of an unexpected crash or RAM reset)
   1. Plug the USB Drive into the calculator with the OTG Cable
   2. Run VIDEO using any jailbreak method
   3. Information should pop up about the drive on the calculator
   4. When the selection screen is reached, use del until your chosen filename appears
   5. Use mode to play video
      - Holding down mode will fast-forward, but it will buffer for a while after fast forward
      - 2nd exits the program (sometimes needs to be pressed twice)
