netcat -l -u -p 8889 | baudline -stdin -stdout -samplerate 48e3 -channels 1 -format le16 | play --buffer 20 -t raw -b 16 -r48000 -esigned - 
#netcat -l -u -p 8889 | baudline -stdin -stdout -samplerate 120e3 -channels 1 -format le16 | play --buffer 20 -t raw -b 16 -r 120e3 -esigned - 
