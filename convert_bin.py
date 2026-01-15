import cv2
import os
from PIL import Image

vid_in = input(".mp4 Filename: ").strip()
cap_in = input(".srt Filename (hit [ENTER] for no CC): ").strip()
bin_out = input(".bin Export Name: ").strip()

if not vid_in.endswith(".mp4") or not bin_out.endswith(".bin") or (not cap_in.endswith(".srt") and cap_in != ""):
    raise Exception("Make sure you include the file type at the end of the filename! (.mp4, .srt, .bin)")

vid = cv2.VideoCapture(f"./{vid_in}")
print(f"Opening {vid_in}. Collecting frames in ./frames")

timestamp = 0
frame_time = 125 #output frame length in ms
count, con, success = 0, 0, True
cutoff = -1#300000 #set to -1 to ignore. made testing easier
while success:
    success, image = vid.read() # Read frame
    if success and (cutoff > timestamp or cutoff < 0):
        if vid.get(cv2.CAP_PROP_POS_MSEC) > timestamp:
            resized = cv2.resize(image, (160, 120), interpolation=cv2.INTER_LINEAR) #Scale frame
            cv2.imwrite(f"./frames/frame{con}.jpg", resized) # Save frame
            con += 1
            timestamp += frame_time
        count += 1
    else:
        success = False

print("total frames:", count, "| converted frames:", con)

def get_fr(line):
    hr = int(line[0:2])
    mn = int(line[3:5])
    sc = int(line[6:8])
    tn = int(line[9])

    a = hr * 3600 + mn * 60 + sc + tn / 10

    hr = int(line[17:19])
    mn = int(line[20:22])
    sc = int(line[23:25])
    tn = int(line[26])

    b = hr * 3600 + mn * 60 + sc + tn / 10

    return int(a * 8), int(b * 8)

vid.release()

subtitles = []

spot = 0
added_lns = -1
cur = ""
st = 0
en = 0
if cap_in != "":
  with open(cap_in) as file:
    for l in file:
        line = l.strip()
        if line != '':
            try:
                a = int(line)
                j = [st, en, cur]
                subtitles.append(j)
                cur = ""
                spot = a
                added_lns = -1
            except:
                if added_lns == -1:
                    st, en = get_fr(line)
                else:
                    cur += line
                    while len(cur) < (added_lns + 1) * 40:
                        cur += " "
                added_lns += 1
j = [st, en, cur]
subtitles.append(j)

def convert_palette(r, g, b):
    a = (int((r / 256) * 8) << 5) + (int((b / 256) * 4) << 3) + (int((g / 256) * 8))
    if a > 255:
        print(a, r, g, b, int((r / 256) * 8), int((b / 256) * 4), int((g / 256) * 8))
    return a

with open(bin_out, "wb") as file:
    a = []
    for frame in range(con):
        if frame % 500 == 0:
            print(f"{frame}/{con} | {round(frame / con * 100, 3)}%")

        img = Image.open(f"./frames/frame{frame}.jpg")
        pix = img.load()
        for y in range(120):
            for x in range(160):
                a.append(convert_palette(pix[x, y][0], pix[x, y][1], pix[x, y][2]))
        st = ""
        for i in range(len(subtitles)):
            if frame > subtitles[i][0] and frame < subtitles[i][1]:
                st += subtitles[i][2]
       
        add_at_end = 0
        for i in range(256):
            if i < len(st):
                char = ord(st[i])
                if char < 128:
                    a.append(char)
                else:
                    add_at_end+= 1
            else:
                a.append(ord(' '))
        for i in range(add_at_end):
            a.append(ord(' '))
    file.write(bytes(a))

print(f"{con}/{con} | 100%")

print(bin_out, "Generated. Cleaning ./frames")

#clean up all temp images
for img in os.listdir("./frames"):
    pth = os.path.join("./frames", img)

    if img.endswith(".jpg") and os.path.isfile(pth):
        os.remove(pth)

print("Done Cleaning Up")
