ACLOCAL_AMFLAGS = -I m4 ${ACLOCAL_FLAGS}
SUBDIRS = libs/hashes libs/utils libs/config libs/qclassify qclassify tests/unit

test:
	@echo ===================================================================
	@echo LAUNCHING UNIT-TESTS
	@echo ===================================================================
	$(MAKE) -C tests/unit $@
	@echo
	@echo ===================================================================
	@echo LAUNCHING INTEGRAION TESTS
	@echo ===================================================================
	cd tests/integration && ./run.sh
