gcc -Wall -g hash.c `pkg-config --cflags --libs glib-2.0` -o test
./test >> hashcout.txt
rm -rf test*