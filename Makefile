all:
	gcc \
	-Wfatal-errors \
	-ggdb \
	-std=c99 \
	./src/*.c \
	-I"C:\Users\ander\Desktop\Development\games\c\dev_libs\SDL2-2.0.10\x86_64-w64-mingw32\include" \
	-I"C:\Users\ander\Desktop\Development\games\c\3drender\include" \
	-L"C:\Users\ander\Desktop\Development\games\c\dev_libs\SDL2-2.0.10\x86_64-w64-mingw32\lib" \
	-lm \
	-lmingw32 \
	-lSDL2main \
	-lSDL2 \
	-o renderer.exe

run:
	renderer.exe

clean:
	del renderer.exe