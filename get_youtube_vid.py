from pytubefix import YouTube

url = input("YouTube URL: ")
yt = YouTube(url)

print("Video Title:", yt.title)

filename = input("Name for the file (no file extension): ")

skip_captions = False
try:
    caption = next(iter(yt.captions))
except:
    skip_captions = True

try:
    caption = yt.captions['en']
    print("English Captions")
except:
    try:
        caption = yt.captions['en-GB']
        print("British English Captions")
    except:
        try:
            caption = yt.captions['a.en']
            print("English (Auto Generated) Captions")
        except:
            print(f"No English Captions Available. Language is {yt.captions.keys()}")

if not skip_captions:
    caption.save_captions(f"{filename}_subtitles.srt")

stream = yt.streams.filter(progressive=True, file_extension='mp4').first()
stream.download(filename=f"{filename}_video.mp4")

print("Done Downloading")
