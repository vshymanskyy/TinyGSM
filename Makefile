.PHONY: travis-build

travis-build:
ifdef PLATFORMIO_CI_ARGS
	platformio ci --lib="." $(PLATFORMIO_CI_ARGS)
else
	platformio ci --lib="." --board=leonardo
endif

