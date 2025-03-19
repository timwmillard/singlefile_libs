
compile_flags.txt:
	@echo "Generating compile_flags.txt for IDE support"
	@echo $(CFLAGS) | tr ' ' '\n' > $@

mkdirs:
	mkdir -p include

update-deps: mkdirs
	wget -O include/sokol_app.h https://raw.githubusercontent.com/floooh/sokol/master/sokol_app.h

# Compile C files
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Compile C++ files
%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

