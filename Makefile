# All of the sources participating in the build are defined here
SRCS = $(wildcard misc/*.cc) $(wildcard types/*.cc) $(wildcard tools/*.cc) $(wildcard filters/*.cc) $(wildcard minisat/*.cc) $(wildcard ./*.cc)
HDRS = $(wildcard misc/*.h) $(wildcard types/*.h) $(wildcard tools/*.h) $(wildcard filters/*.h) $(wildcard minisat/*.h) $(wildcard ./*.h)

MAINS = ./cnf2aig.o ./cnf2dot.o ./geninp.o ./metrics.o ./spectrum.o ./preprocess.o
ALLOBJS = $(SRCS:.cc=.o)
OBJS = $(filter-out $(MAINS), $(ALLOBJS))
DEPS = $(SRCS:.cc=.d)

MINI = -DMINISAT_TYPES=1 -Iminisat/solver -I/usr/include/eigen3
MINI2 = -Lminisat/solver/build/release/lib -lminisat

#CFLAGS = $(MINI) -D__GXX_EXPERIMENTAL_CXX0X__ -D__STDC_CONSTANT_MACROS=1 -D__STDC_LIMIT_MACROS=1 -D__STDC_FORMAT_MACROS=1 -O3 -Wall -c -fmessage-length=0 -std=c++11 -fpermissive
CFLAGS = $(MINI) -D__GXX_EXPERIMENTAL_CXX0X__ -D__STDC_CONSTANT_MACROS=1 -D__STDC_LIMIT_MACROS=1 -D__STDC_FORMAT_MACROS=1 -g -Wall -c -fmessage-length=0 -std=c++0x -fpermissive

minisat/%.o: minisat/%.cc
	@echo 'Building file: $<'
	g++ $(CFLAGS) -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '
	
misc/%.o: misc/%.cc
	@echo 'Building file: $<'
	g++ $(CFLAGS) -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '
	
types/%.o: types/%.cc
	@echo 'Building file: $<'
	g++ $(CFLAGS) -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

tools/%.o: tools/%.cc
	@echo 'Building file: $<'
	g++ $(CFLAGS) -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

filters/%.o: filters/%.cc
	@echo 'Building file: $<'
	g++ $(CFLAGS) -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

./%.o: ./%.cc
	@echo 'Building file: $<'
	g++ $(CFLAGS) -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

# All Target
all: cnf2aig cnf2dot geninp metrics preprocess

# Tool invocations
cnf2aig: $(ALLOBJS)
	@echo 'Building target: $@'
	g++ -static -flto -o "$@" $(OBJS) $@.o -lz $(MINI2)
	@echo 'Finished building target: $@'
	@echo ' '

cnf2dot: $(ALLOBJS)
	@echo 'Building target: $@'
	g++ -static -flto -o "cnf2dot" $(OBJS) cnf2dot.o -lz $(MINI2)
	@echo 'Finished building target: $@'
	@echo ' '

geninp: $(ALLOBJS)
	@echo 'Building target: $@'
	g++ -static -flto -o "geninp" $(OBJS) geninp.o -lz $(MINI2)
	@echo 'Finished building target: $@'
	@echo ' '

metrics: $(ALLOBJS)
	@echo 'Building target: $@'
	g++ -static -flto -o "metrics" $(OBJS) metrics.o -lz $(MINI2)
	@echo 'Finished building target: $@'
	@echo ' '

spectrum: $(ALLOBJS)
	@echo 'Building target: $@'
	g++ -static -flto -o "spectrum" $(OBJS) spectrum.o -lz $(MINI2)
	@echo 'Finished building target: $@'
	@echo ' '

preprocess: $(ALLOBJS)
	@echo 'Building target: $@'
	g++ -static -flto -o "preprocess" $(OBJS) preprocess.o -lz $(MINI2)
	@echo 'Finished building target: $@'
	@echo ' '

# Other Targets
clean:
	rm $(ALLOBJS) $(DEPS)

realclean: clean
	rm cnf2aig cnf2dot geninp metrics spectrum preprocess



