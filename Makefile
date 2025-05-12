
all: mkdir arena sokol nuklear miniaudio stb json \
	jsonc jsmn uuid

mkdir:
	mkdir -p include

arena:
	wget -O include/arena.h https://raw.githubusercontent.com/tsoding/arena/refs/heads/master/arena.h

stb:
	wget -O include/stb_ds.h https://raw.githubusercontent.com/nothings/stb/refs/heads/master/stb_ds.h
	wget -O include/stb_image.h https://raw.githubusercontent.com/nothings/stb/master/stb_image.h
	wget -O include/stb_truetype.h https://raw.githubusercontent.com/nothings/stb/refs/heads/master/stb_truetype.h
	wget -O include/stb_image_write.h https://raw.githubusercontent.com/nothings/stb/refs/heads/master/stb_image_write.h
	wget -O include/stb_image_resize2.h https://raw.githubusercontent.com/nothings/stb/refs/heads/master/stb_image_resize2.h
	wget -O include/stb_perline.h https://raw.githubusercontent.com/nothings/stb/refs/heads/master/stb_perlin.h
	wget -O include/stb_tilemap_editor.h https://raw.githubusercontent.com/nothings/stb/refs/heads/master/stb_tilemap_editor.h

sokol:
	wget -O include/sokol_app.h https://raw.githubusercontent.com/floooh/sokol/master/sokol_app.h
	wget -O include/sokol_audio.h https://raw.githubusercontent.com/floooh/sokol/master/sokol_audio.h
	wget -O include/sokol_gfx.h https://raw.githubusercontent.com/floooh/sokol/master/sokol_gfx.h
	wget -O include/sokol_glue.h https://raw.githubusercontent.com/floooh/sokol/master/sokol_glue.h
	wget -O include/sokol_time.h https://raw.githubusercontent.com/floooh/sokol/master/sokol_time.h
	wget -O include/sokol_log.h https://raw.githubusercontent.com/floooh/sokol/master/sokol_log.h
	wget -O include/sokol_fetch.h https://raw.githubusercontent.com/floooh/sokol/refs/heads/master/sokol_fetch.h

sokol_gp:
	wget -O include/sokol_gp.h https://raw.githubusercontent.com/edubart/sokol_gp/master/sokol_gp.h

nuklear:
	wget -O include/nuklear.h https://github.com/Immediate-Mode-UI/Nuklear/raw/refs/heads/master/nuklear.h
	wget -O include/sokol_nuklear.h https://github.com/floooh/sokol/raw/refs/heads/master/util/sokol_nuklear.h

miniaudio:
	wget -O include/miniaudio.h https://raw.githubusercontent.com/mackron/miniaudio/refs/heads/master/miniaudio.h

json:
	wget -O include/json.h https://raw.githubusercontent.com/sheredom/json.h/refs/heads/master/json.h

jsonc:
	wget -O include/json.c https://gitlab.com/bztsrc/jsonc/-/raw/master/jsonc.c

jsmn:
	wget -O include/jsmn.h https://raw.githubusercontent.com/zserge/jsmn/refs/heads/master/jsmn.h

uuid:
	wget -O include/uuid.h https://raw.githubusercontent.com/wc-duck/uuid_h/refs/heads/master/uuid.h

clay:
	wget -O include/clay.h https://raw.githubusercontent.com/nicbarker/clay/refs/heads/main/clay.h

nob:
	wget -O include/nob.h https://raw.githubusercontent.com/tsoding/nob.h/refs/heads/main/nob.h

flag:
	wget -O include/flag.h https://raw.githubusercontent.com/tsoding/flag.h/refs/heads/master/flag.h

luigi:
	wget -O include/luigi3.h https://raw.githubusercontent.com/nakst/luigi/refs/heads/main/experimental/luigi3.h

open-simplex-noise:
	wget -O deps/open-simplex-noise.c https://raw.githubusercontent.com/smcameron/open-simplex-noise-in-c/refs/heads/master/open-simplex-noise.c
	wget -O deps/open-simplex-noise.h https://raw.githubusercontent.com/smcameron/open-simplex-noise-in-c/refs/heads/master/open-simplex-noise.h

