#netcat -l -u -p 8889 | baudline -stdin -samplerate 100e3 -channels 2 -format s8 -quadrature
netcat -l -u -p 8889 | baudline -stdin -samplerate 48e3 -channels 1 -format le16 
#netcat -l -u -p 8889 | ./p25_decode_test | baudline -stdin -samplerate 100e3 -channels 2 -format be16 -quadrature
