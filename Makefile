
all: mkdir arena sokol nuklear miniaudio stb json \
	jsonc jsmn uuid

# mate build system, lives in the root directory
mate:
	wget -O mate.h https://github.com/TomasBorquez/mate.h/raw/refs/heads/master/mate.h

mkdir:
	mkdir -p deps

arena:
	wget -O deps/arena.h https://raw.githubusercontent.com/tsoding/arena/refs/heads/master/arena.h

stb:
	wget -O deps/stb_ds.h https://raw.githubusercontent.com/nothings/stb/refs/heads/master/stb_ds.h
	wget -O deps/stb_image.h https://raw.githubusercontent.com/nothings/stb/master/stb_image.h
	wget -O deps/stb_truetype.h https://raw.githubusercontent.com/nothings/stb/refs/heads/master/stb_truetype.h
	wget -O deps/stb_image_write.h https://raw.githubusercontent.com/nothings/stb/refs/heads/master/stb_image_write.h
	wget -O deps/stb_image_resize2.h https://raw.githubusercontent.com/nothings/stb/refs/heads/master/stb_image_resize2.h
	wget -O deps/stb_perline.h https://raw.githubusercontent.com/nothings/stb/refs/heads/master/stb_perlin.h
	wget -O deps/stb_tilemap_editor.h https://raw.githubusercontent.com/nothings/stb/refs/heads/master/stb_tilemap_editor.h

sokol:
	wget -O deps/sokol_app.h https://raw.githubusercontent.com/floooh/sokol/master/sokol_app.h
	wget -O deps/sokol_audio.h https://raw.githubusercontent.com/floooh/sokol/master/sokol_audio.h
	wget -O deps/sokol_gfx.h https://raw.githubusercontent.com/floooh/sokol/master/sokol_gfx.h
	wget -O deps/sokol_glue.h https://raw.githubusercontent.com/floooh/sokol/master/sokol_glue.h
	wget -O deps/sokol_time.h https://raw.githubusercontent.com/floooh/sokol/master/sokol_time.h
	wget -O deps/sokol_log.h https://raw.githubusercontent.com/floooh/sokol/master/sokol_log.h
	wget -O deps/sokol_fetch.h https://raw.githubusercontent.com/floooh/sokol/refs/heads/master/sokol_fetch.h

sokol_gp:
	wget -O deps/sokol_gp.h https://raw.githubusercontent.com/edubart/sokol_gp/master/sokol_gp.h

nuklear:
	wget -O deps/nuklear.h https://github.com/Immediate-Mode-UI/Nuklear/raw/refs/heads/master/nuklear.h
	wget -O deps/sokol_nuklear.h https://github.com/floooh/sokol/raw/refs/heads/master/util/sokol_nuklear.h

miniaudio:
	wget -O deps/miniaudio.h https://raw.githubusercontent.com/mackron/miniaudio/refs/heads/master/miniaudio.h

json:
	wget -O deps/json.h https://raw.githubusercontent.com/sheredom/json.h/refs/heads/master/json.h

jsonc:
	wget -O deps/json.c https://gitlab.com/bztsrc/jsonc/-/raw/master/jsonc.c

jsmn:
	wget -O deps/jsmn.h https://raw.githubusercontent.com/zserge/jsmn/refs/heads/master/jsmn.h

uuid:
	wget -O deps/uuid.h https://raw.githubusercontent.com/wc-duck/uuid_h/refs/heads/master/uuid.h

clay:
	wget -O deps/clay.h https://raw.githubusercontent.com/nicbarker/clay/refs/heads/main/clay.h
	wget -O deps/sokol_clay.h https://github.com/nicbarker/clay/raw/refs/heads/main/renderers/sokol/sokol_clay.h
	wget -O deps/sokol_fontstash.h https://github.com/floooh/sokol/raw/refs/heads/master/util/sokol_fontstash.h

nob:
	wget -O deps/nob.h https://raw.githubusercontent.com/tsoding/nob.h/refs/heads/main/nob.h

flag:
	wget -O deps/flag.h https://raw.githubusercontent.com/tsoding/flag.h/refs/heads/master/flag.h

luigi:
	wget -O deps/luigi3.h https://raw.githubusercontent.com/nakst/luigi/refs/heads/main/experimental/luigi3.h

open-simplex-noise:
	wget -O deps/open-simplex-noise.c https://raw.githubusercontent.com/smcameron/open-simplex-noise-in-c/refs/heads/master/open-simplex-noise.c
	wget -O deps/open-simplex-noise.h https://raw.githubusercontent.com/smcameron/open-simplex-noise-in-c/refs/heads/master/open-simplex-noise.h

minilua:
	wget -O deps/minilua.h https://github.com/edubart/minilua/raw/refs/heads/main/minilua.h

bear-http-client:
	wget -O deps/BearHttpsClientOne.c https://github.com/OUIsolutions/BearHttpsClient/releases/download/0.2.8/BearHttpsClientOne.c

http:
	wget -O deps/http.h https://github.com/mattiasgustavsson/libs/raw/refs/heads/main/http.h
	wget -O deps/cute_tls.h https://github.com/RandyGaul/cute_headers/raw/refs/heads/master/cute_tls.h

mongoose:
	wget -O deps/mongoose.h https://github.com/cesanta/mongoose/raw/refs/heads/master/mongoose.h
	wget -O deps/mongoose.c https://github.com/cesanta/mongoose/raw/refs/heads/master/mongoose.c

compile_flags.txt: FORCE
	@echo "Generating compile_flags.txt for IDE support"
	@echo $(CFLAGS) | tr ' ' '\n' > $@

FORCE:
