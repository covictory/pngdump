all: pngdump.c
	emcc -O3 -s WASM=1 -s ASSERTIONS=1 -s ALLOW_MEMORY_GROWTH=1 \
			 -s EXTRA_EXPORTED_RUNTIME_METHODS='["cwrap"]' \
			 -s USE_ZLIB \
			 -s MODULARIZE=1 -s EXPORT_NAME='pngdump' \
			 -I../libpng -L../libpng/.libs \
			 pngdump.c -lpng16 -o pngdump.js
	mv pngdump.js pngdump.wasm testsite
