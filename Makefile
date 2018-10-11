builddir=out
builddir_coverage=$(builddir)/coverage
builddir_field_release=$(builddir)/field-release
builddir_field_debug=$(builddir)/field-debug

ifdef INSTRUMENT_TEST
oom_extra="-DINSTRUMENT_TEST_ENABLED=YES"
endif

ifdef FOOTPRINT
footprint_extra=-DBH_FOOTPRINT
endif

final_release: field-release

all: field-release field-debug

$(builddir):
	mkdir -p $(builddir)

$(builddir_coverage):
	mkdir -p $(builddir_coverage)

$(builddir_field_release):
	mkdir -p $(builddir_field_release)

$(builddir_field_debug):
	mkdir -p $(builddir_field_debug)

coverage: $(builddir_coverage)
	cd $(builddir_coverage);\
	if [ ! -f Makefile ]; then\
	  export CFLAGS="--coverage";\
          export CXXFLAGS="--coverage";\
	  cmake ../../ -DCMAKE_BUILD_TYPE=MinSizeRel -DNO_OPT=YES $(oom_extra);\
        fi;\
        make;\
	cd -;\

field-release: $(builddir_field_release)
		cd $(builddir_field_release);\
		if [ ! -f Makefile ]; then\
		  export CFLAGS="$(footprint_extra)";\
		  export CXXFLAGS="$(footprint_extra)";\
		  cmake ../../ -DCMAKE_BUILD_TYPE=MinSizeRel $(oom_extra);\
		fi;\
		make;\
		cd -;\

field-debug: $(builddir_field_debug)
		cd $(builddir_field_debug);\
		if [ ! -f Makefile ]; then\
		  export CFLAGS="$(CFLAGS) -fstack-protector-all $(footprint_extra) -DBH_DEBUG -DWASM_ENABLE_REPL";\
		  export CXXFLAGS="$(CXXFLAGS) $(footprint_extra)";\
		  cmake ../../ -DCMAKE_BUILD_TYPE=Debug $(oom_extra);\
		fi;\
		make;\
		cd -;\

clean:
	rm -rf $(builddir)

help:
	@echo "    make help           # Print help information"
	@echo "    make                # Make default release version."
	@echo "    make field-release  # Make release version for field."
	@echo "    make field-debug    # Make debug version for field."
	@echo "    make all            # Make all of the field-release and field-debug."
