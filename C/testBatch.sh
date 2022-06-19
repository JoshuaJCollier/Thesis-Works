./gradientAscent -freq 500 -buff 4096 -time 180 # 3 mins 500Hz
./gradientAscent -freq 1000 -buff 2048 -time 180 # 3 mins 1000Hz
./gradientAscent -freq 2000 -buff 1024 -time 180 # 3 mins 2000Hz
./gradientAscent -time 180 -grad 1
./gradientAscent -time 180 -grad 0.5
./gradientAscent -time 180 -grad 0.1
./gradientAscent -time 180 -grad 0.01
./gradientAscent -time 180 -smart
./gradientAscent -time 180 -grad 1 -freq 1000 -buff 2048
./gradientAscent -time 180 -grad 0.5 -freq 1000 -buff 2048
./gradientAscent -time 180 -grad 0.1 -freq 1000 -buff 2048
./gradientAscent -time 180 -grad 0.01 -freq 1000 -buff 2048
./gradientAscent -time 180 -smart -freq 1000 -buff 2048
