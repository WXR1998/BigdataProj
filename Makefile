gpp = g++
prom = kvdb
deps = $(shell find ./src -name "*.hpp")
src = $(shell find ./src -name "*.cpp" ! -name "generator.cpp")
objdir = ./bin/
setting = -w -std=c++14 -O2
$(shell mkdir $(objdir))
obj = $(src:%.cpp=%.o) 

$(prom): $(obj)
	$(gpp) -o $(objdir)$(prom) $(addprefix $(objdir), $(notdir $(obj))) $(setting)
	@echo ""
	@bin/$(prom)

%.o: %.cpp $(deps)
	$(gpp) -c $< -o $(addprefix $(objdir), $(notdir $@)) $(setting)

clean:
	rm -rf $(objdir)*
